#include "sitech_mount.h"

#include <libnova/sidereal_time.h>
#include <libnova/transform.h>
#include <cstring>
#include <cmath>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <io.h>
    #define read _read
    #define write _write
#else
    #include <unistd.h>
#endif

// Unique identifier for this driver
static std::unique_ptr<SiTechMount> sitech_mount(new SiTechMount());

SiTechMount::SiTechMount()
{
    setVersion(1, 0);
    SetTelescopeCapability(TELESCOPE_CAN_PARK | TELESCOPE_CAN_SYNC | TELESCOPE_CAN_GOTO |
                          TELESCOPE_CAN_ABORT | TELESCOPE_HAS_TIME | TELESCOPE_HAS_LOCATION |
                          TELESCOPE_HAS_PIER_SIDE | TELESCOPE_CAN_CONTROL_TRACK |
                          TELESCOPE_HAS_TRACK_MODE | TELESCOPE_HAS_TRACK_RATE, 9);
    
    // Initialize state
    scope_initialized = false;
    scope_tracking = false;
    scope_slewing = false;
    scope_parking = false;
    scope_parked = false;
    scope_looking_east = false;
    controller_blinky = false;
    communication_fault = false;
    use_ascii_checksum = false;
    
    current_ra = 0;
    current_dec = 0;
    current_alt = 0;
    current_az = 0;
    scope_sidereal_time = 0;
    scope_julian_day = 0;
    scope_time = 0;
    air_mass = 0;
}

const char *SiTechMount::getDefaultName()
{
    return "SiTech Mount";
}

bool SiTechMount::initProperties()
{
    INDI::Telescope::initProperties();

    // ASCII Checksum Mode
    IUFillSwitch(&ASCIIChecksumS[0], "ENABLE", "Enable", ISS_OFF);
    IUFillSwitch(&ASCIIChecksumS[1], "DISABLE", "Disable", ISS_ON);
    IUFillSwitchVector(&ASCIIChecksumSP, ASCIIChecksumS, 2, getDeviceName(),
                       "ASCII_CHECKSUM", "ASCII Checksum", CONNECTION_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);

    // Slew Rate
    IUFillNumber(&SlewRateN[0], "SLEW_RATE", "Rate (deg/s)", "%.2f", 0.01, 10.0, 0.1, 1.0);
    IUFillNumber(&SlewRateN[1], "GUIDE_RATE", "Guide Rate (x)", "%.2f", 0.1, 1.0, 0.1, 0.5);
    IUFillNumberVector(&SlewRateNP, SlewRateN, 2, getDeviceName(), "SLEW_RATE", "Slew Rate",
                       MOTION_TAB, IP_RW, 60, IPS_IDLE);

    // Scope Information
    IUFillText(&ScopeInfoT[0], "APERTURE_DIAMETER", "Aperture (mm)", "");
    IUFillText(&ScopeInfoT[1], "APERTURE_AREA", "Area (sq mm)", "");
    IUFillText(&ScopeInfoT[2], "FOCAL_LENGTH", "Focal Length (mm)", "");
    IUFillText(&ScopeInfoT[3], "SCOPE_NAME", "Name", "");
    IUFillTextVector(&ScopeInfoTP, ScopeInfoT, 4, getDeviceName(), "SCOPE_INFO",
                     "Scope Info", INFO_TAB, IP_RO, 60, IPS_IDLE);

    // Site Location
    IUFillNumber(&SiteLocationN[0], "LATITUDE", "Latitude (deg)", "%.6f", -90, 90, 0, 0);
    IUFillNumber(&SiteLocationN[1], "LONGITUDE", "Longitude (deg)", "%.6f", -180, 180, 0, 0);
    IUFillNumber(&SiteLocationN[2], "ELEVATION", "Elevation (m)", "%.1f", -200, 9000, 0, 0);
    IUFillNumberVector(&SiteLocationNP, SiteLocationN, 3, getDeviceName(), "SITE_LOCATION",
                       "Site", SITE_TAB, IP_RO, 60, IPS_IDLE);

    // Set park data
    SetParkDataType(PARK_AZ_ALT);

    // TCP Connection plugin only
    tcpConnection = new Connection::TCP(this);
    tcpConnection->registerHandshake([&]() { return Handshake(); });
    tcpConnection->setDefaultHost("localhost");
    tcpConnection->setDefaultPort(8079);

    registerConnection(tcpConnection);
    setActiveConnection(tcpConnection);
    
    return true;
}

bool SiTechMount::updateProperties()
{
    INDI::Telescope::updateProperties();

    if (isConnected())
    {
        defineProperty(&ASCIIChecksumSP);
        defineProperty(&SlewRateNP);
        defineProperty(&ScopeInfoTP);
        defineProperty(&SiteLocationNP);
    }
    else
    {
        deleteProperty(ASCIIChecksumSP.name);
        deleteProperty(SlewRateNP.name);
        deleteProperty(ScopeInfoTP.name);
        deleteProperty(SiteLocationNP.name);
    }

    return true;
}

bool SiTechMount::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Handle any text properties here if needed
    }

    return INDI::Telescope::ISNewText(dev, name, texts, names, n);
}

bool SiTechMount::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(name, SlewRateNP.name) == 0)
        {
            IUUpdateNumber(&SlewRateNP, values, names, n);
            SlewRateNP.s = IPS_OK;
            IDSetNumber(&SlewRateNP, nullptr);
            return true;
        }
    }

    return INDI::Telescope::ISNewNumber(dev, name, values, names, n);
}

bool SiTechMount::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(name, ASCIIChecksumSP.name) == 0)
        {
            IUUpdateSwitch(&ASCIIChecksumSP, states, names, n);
            use_ascii_checksum = (ASCIIChecksumS[0].s == ISS_ON);
            
            // Send command to enable/disable ASCII checksum mode
            std::string response;
            if (use_ascii_checksum)
            {
                sendCommand("YXY1", response);
            }
            else
            {
                sendCommand("YXY0", response);
            }
            
            ASCIIChecksumSP.s = IPS_OK;
            IDSetSwitch(&ASCIIChecksumSP, nullptr);
            return true;
        }
    }

    return INDI::Telescope::ISNewSwitch(dev, name, states, names, n);
}

bool SiTechMount::Handshake()
{
    // Only use TCP connection
    PortFD = tcpConnection->getPortFD();

    if (PortFD < 0)
    {
        LOG_ERROR("Failed to connect to SiTech mount");
        return false;
    }

    // Test TCP connection with ReadScopeStatus command
    std::string response;
    if (!sendTCPCommand("ReadScopeStatus\n", response))
    {
        LOG_ERROR("Failed to communicate with SiTech mount via TCP");
        return false;
    }

    LOG_INFO("SiTech mount connected successfully via TCP");
    
    // Get scope information
    if (sendTCPCommand("ScopeInfo\n", response))
    {
        parseScopeInfo(response);
    }
    
    // Get site location
    if (sendTCPCommand("SiteLocations\n", response))
    {
        parseSiteLocation(response);
    }
    
    // Start periodic polling of mount status
    SetTimer(getCurrentPollingPeriod());
    
    return true;
}

bool SiTechMount::ReadScopeStatus()
{
    std::string response;
    
    // Send ReadScopeStatus command to get standard return string with mount status
    if (!sendTCPCommand("ReadScopeStatus\n", response))
    {
        return false;
    }
    
    return parseTCPResponse(response);
}

bool SiTechMount::Goto(double RA, double Dec)
{
    std::string command, response;
    
    // INDI provides J2000 coordinates, so we add J2K flag to let SiTechExe
    // handle precession, nutation, and aberration corrections
    command = "GoTo " + std::to_string(RA) + " " + std::to_string(Dec) + " J2K\n";
    if (!sendTCPCommand(command, response))
    {
        return false;
    }

    TrackState = SCOPE_SLEWING;
    return true;
}

bool SiTechMount::Sync(double RA, double Dec)
{
    std::string command, response;
    
    // INDI provides J2000 coordinates, so we add J2K flag
    // Use mode 2 (instant "load calibration" init) for quick sync
    command = "Sync " + std::to_string(RA) + " " + std::to_string(Dec) + " 2 J2K\n";
    if (!sendTCPCommand(command, response))
    {
        return false;
    }

    return true;
}

bool SiTechMount::Abort()
{
    std::string response;
    
    if (!sendTCPCommand("Abort\n", response))
    {
        return false;
    }

    TrackState = SCOPE_IDLE;
    return true;
}

bool SiTechMount::Park()
{
    std::string response;
    
    if (!sendTCPCommand("Park\n", response))
    {
        return false;
    }

    TrackState = SCOPE_PARKING;
    return true;
}

bool SiTechMount::UnPark()
{
    std::string response;
    
    if (!sendTCPCommand("UnPark\n", response))
    {
        return false;
    }

    return true;
}

bool SiTechMount::SetCurrentPark()
{
    std::string response;
    
    if (!sendTCPCommand("SetPark\n", response))
    {
        return false;
    }

    return true;
}

bool SiTechMount::SetDefaultPark()
{
    // Set default park position (pointing to celestial pole)
    SetAxis1Park(0);  // HA = 0
    SetAxis2Park(LocationN[LOCATION_LATITUDE].value);  // Dec = Latitude
    return true;
}

bool SiTechMount::MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command)
{
    std::string cmd, response;
    
    if (command == MOTION_START)
    {
        double rate = SlewRateN[0].value;
        if (dir == DIRECTION_NORTH)
            cmd = "MoveAxisPri " + std::to_string(rate) + "\n";
        else
            cmd = "MoveAxisPri " + std::to_string(-rate) + "\n";
    }
    else
    {
        cmd = "MoveAxisPri 0\n";
    }
    
    return sendTCPCommand(cmd, response);
}

bool SiTechMount::MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command)
{
    std::string cmd, response;
    
    if (command == MOTION_START)
    {
        double rate = SlewRateN[0].value;
        if (dir == DIRECTION_WEST)
            cmd = "MoveAxisSec " + std::to_string(rate) + "\n";
        else
            cmd = "MoveAxisSec " + std::to_string(-rate) + "\n";
    }
    else
    {
        cmd = "MoveAxisSec 0\n";
    }
    
    return sendTCPCommand(cmd, response);
}

IPState SiTechMount::GuideNorth(uint32_t ms)
{
    std::string cmd, response;
    
    // PulseGuide format: "PulseGuide Direction, Milliseconds\n" (note the comma)
    cmd = "PulseGuide 0, " + std::to_string(ms) + "\n";
    if (sendTCPCommand(cmd, response))
        return IPS_OK;
    
    return IPS_ALERT;
}

IPState SiTechMount::GuideSouth(uint32_t ms)
{
    std::string cmd, response;
    
    cmd = "PulseGuide 1, " + std::to_string(ms) + "\n";
    if (sendTCPCommand(cmd, response))
        return IPS_OK;
    
    return IPS_ALERT;
}

IPState SiTechMount::GuideEast(uint32_t ms)
{
    std::string cmd, response;
    
    cmd = "PulseGuide 2, " + std::to_string(ms) + "\n";
    if (sendTCPCommand(cmd, response))
        return IPS_OK;
    
    return IPS_ALERT;
}

IPState SiTechMount::GuideWest(uint32_t ms)
{
    std::string cmd, response;
    
    cmd = "PulseGuide 3, " + std::to_string(ms) + "\n";
    if (sendTCPCommand(cmd, response))
        return IPS_OK;
    
    return IPS_ALERT;
}

bool SiTechMount::SetTrackMode(uint8_t mode)
{
    std::string cmd, response;
    
    if (mode == TRACK_SIDEREAL)
    {
        cmd = "SetTrackMode 1 0 0.0 0.0\n";
    }
    else if (mode == TRACK_SOLAR)
    {
        cmd = "SetTrackMode 1 1 0.0 0.0\n"; // Would need solar tracking rates
    }
    else if (mode == TRACK_LUNAR)
    {
        cmd = "SetTrackMode 1 1 0.0 0.0\n"; // Would need lunar tracking rates
    }
    else
    {
        cmd = "SetTrackMode 0 0 0.0 0.0\n";
    }
    
    return sendTCPCommand(cmd, response);
}

bool SiTechMount::SetTrackEnabled(bool enabled)
{
    std::string cmd, response;
    
    if (enabled)
        cmd = "SetTrackMode 1 0 0.0 0.0\n";
    else
        cmd = "SetTrackMode 0 0 0.0 0.0\n";
    
    return sendTCPCommand(cmd, response);
}

bool SiTechMount::sendCommand(const std::string &command, std::string &response)
{
    return sendTCPCommand(command, response);
}

bool SiTechMount::sendTCPCommand(const std::string &command, std::string &response)
{
    if (PortFD < 0)
    {
        LOG_ERROR("No active connection to SiTechExe");
        return false;
    }

    // Send command via persistent connection
    int bytes_written = write(PortFD, command.c_str(), command.length());
    if (bytes_written != static_cast<int>(command.length()))
    {
        LOGF_ERROR("Failed to send command: %s", command.c_str());
        return false;
    }

    // Read response
    char buffer[SITECH_BUFFER_SIZE];
    int bytes_read = read(PortFD, buffer, SITECH_BUFFER_SIZE - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        response = std::string(buffer);
        return true;
    }

    LOG_ERROR("Failed to read response from SiTechExe");
    return false;
}

bool SiTechMount::sendSerialCommand(const std::string &command, std::string &response)
{
    if (PortFD < 0)
        return false;

    std::string cmd = command;
    
    // Add checksum if in ASCII Checksum mode
    if (use_ascii_checksum && command != "\r")
    {
        cmd += calculateChecksum(command);
    }

    // Send command
    int bytes_written = write(PortFD, cmd.c_str(), cmd.length());
    if (bytes_written != static_cast<int>(cmd.length()))
    {
        LOGF_ERROR("Failed to send command: %s", cmd.c_str());
        return false;
    }

    // Read response
    char buffer[SITECH_BUFFER_SIZE];
    int bytes_read = read(PortFD, buffer, SITECH_BUFFER_SIZE - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        response = std::string(buffer);
        return true;
    }

    return false;
}

bool SiTechMount::parseStatusResponse(const std::string &response)
{
    // Parse serial status response format:
    // X# Y# XZ# YZ# XC# YC# V# T# X[AM] Y[AM] K#
    
    // This is a simplified parser - would need more robust parsing
    LOGF_DEBUG("Status response: %s", response.c_str());
    
    // Update telescope status based on response
    TrackState = scope_slewing ? SCOPE_SLEWING : (scope_tracking ? SCOPE_TRACKING : SCOPE_IDLE);
    
    return true;
}

bool SiTechMount::parseTCPResponse(const std::string &response)
{
    // Parse TCP response format (standard return string):
    // boolParms;RA;Dec;Alt;Az;SecondaryAngle;PrimaryAngle;SiderealTime;JulianDay;ScopeTime;AirMass;_message
    
    LOGF_DEBUG("TCP Response: %s", response.c_str());
    
    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    
    while (std::getline(ss, token, ';'))
    {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 11)
    {
        try
        {
            int boolParms = std::stoi(tokens[0]);
            current_ra = std::stod(tokens[1]);  // Hours, JNow
            current_dec = std::stod(tokens[2]); // Degrees, JNow
            current_alt = std::stod(tokens[3]); // Degrees
            current_az = std::stod(tokens[4]);  // Degrees
            // tokens[5] is Secondary Axis Angle (degrees)
            // tokens[6] is Primary Axis Angle (degrees)
            scope_sidereal_time = std::stod(tokens[7]); // Hours
            scope_julian_day = std::stod(tokens[8]);    // Julian Day
            scope_time = std::stod(tokens[9]);          // Hours
            air_mass = std::stod(tokens[10]);           // AirMass
            // tokens[11] starts with "_" and contains message
            
            // Parse boolean parameters (bit flags)
            scope_initialized = (boolParms & 1) != 0;      // Bit 0: Scope Is Initialized
            scope_tracking = (boolParms & 2) != 0;         // Bit 1: Scope Is Tracking
            scope_slewing = (boolParms & 4) != 0;          // Bit 2: Scope is Slewing
            scope_parking = (boolParms & 8) != 0;          // Bit 3: Scope is Parking
            scope_parked = (boolParms & 16) != 0;          // Bit 4: Scope is Parked
            scope_looking_east = (boolParms & 32) != 0;    // Bit 5: Looking East (GEM)
            controller_blinky = (boolParms & 64) != 0;     // Bit 6: Blinky (Manual) mode
            communication_fault = (boolParms & 128) != 0;  // Bit 7: Comm fault with controller
            
            // Update INDI properties
            NewRaDec(current_ra, current_dec);
            
            // Update tracking state
            if (scope_parked)
                TrackState = SCOPE_PARKED;
            else if (scope_slewing)
                TrackState = SCOPE_SLEWING;
            else if (scope_tracking)
                TrackState = SCOPE_TRACKING;
            else
                TrackState = SCOPE_IDLE;
            
            // Log any error conditions
            if (controller_blinky)
                LOG_WARN("Servo controller is in Blinky (Manual) mode");
            if (communication_fault)
                LOG_ERROR("Communication fault between SiTechExe and ServoController");
            
            return true;
        }
        catch (const std::exception &e)
        {
            LOGF_ERROR("Error parsing TCP response: %s", e.what());
            return false;
        }
    }
    else
    {
        LOGF_ERROR("Invalid TCP response - expected at least 11 fields, got %d", tokens.size());
    }
    
    return false;
}

bool SiTechMount::parseScopeInfo(const std::string &response)
{
    // Parse ScopeInfo response format:
    // ApertureDiameter;ApertureArea;FocalLength;NameOfScope;_ScopeInfo
    
    LOGF_DEBUG("ScopeInfo Response: %s", response.c_str());
    
    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    
    while (std::getline(ss, token, ';'))
    {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 4)
    {
        try
        {
            IUSaveText(&ScopeInfoT[0], tokens[0].c_str()); // Aperture Diameter
            IUSaveText(&ScopeInfoT[1], tokens[1].c_str()); // Aperture Area
            IUSaveText(&ScopeInfoT[2], tokens[2].c_str()); // Focal Length
            IUSaveText(&ScopeInfoT[3], tokens[3].c_str()); // Scope Name
            
            ScopeInfoTP.s = IPS_OK;
            IDSetText(&ScopeInfoTP, nullptr);
            
            LOGF_INFO("Scope: %s, Aperture: %s mm, Focal Length: %s mm", 
                     tokens[3].c_str(), tokens[0].c_str(), tokens[2].c_str());
            
            return true;
        }
        catch (const std::exception &e)
        {
            LOGF_ERROR("Error parsing ScopeInfo response: %s", e.what());
            return false;
        }
    }
    else
    {
        LOGF_ERROR("Invalid ScopeInfo response - expected at least 4 fields, got %d", tokens.size());
    }
    
    return false;
}

bool SiTechMount::parseSiteLocation(const std::string &response)
{
    // Parse SiteLocations response format:
    // siteLatitude (deg's);siteLongitude (deg's);siteElevation (meters);_SiteLocations
    
    LOGF_DEBUG("SiteLocations Response: %s", response.c_str());
    
    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    
    while (std::getline(ss, token, ';'))
    {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 3)
    {
        try
        {
            double latitude = std::stod(tokens[0]);
            double longitude = std::stod(tokens[1]);
            double elevation = std::stod(tokens[2]);
            
            SiteLocationN[0].value = latitude;
            SiteLocationN[1].value = longitude;
            SiteLocationN[2].value = elevation;
            
            SiteLocationNP.s = IPS_OK;
            IDSetNumber(&SiteLocationNP, nullptr);
            
            // Also update the telescope location
            LocationN[LOCATION_LATITUDE].value = latitude;
            LocationN[LOCATION_LONGITUDE].value = longitude;
            LocationN[LOCATION_ELEVATION].value = elevation;
            LocationNP.s = IPS_OK;
            IDSetNumber(&LocationNP, nullptr);
            
            LOGF_INFO("Site Location: Lat %.6f°, Lon %.6f°, Elev %.1f m", 
                     latitude, longitude, elevation);
            
            return true;
        }
        catch (const std::exception &e)
        {
            LOGF_ERROR("Error parsing SiteLocations response: %s", e.what());
            return false;
        }
    }
    else
    {
        LOGF_ERROR("Invalid SiteLocations response - expected at least 3 fields, got %d", tokens.size());
    }
    
    return false;
}

std::string SiTechMount::calculateChecksum(const std::string &command)
{
    uint8_t sum = 0;
    for (char c : command)
    {
        sum += static_cast<uint8_t>(c);
    }
    sum = ~sum; // Invert
    
    return std::string(1, static_cast<char>(sum));
}

// Required for INDI driver registration
extern "C"
{
    INDI::BaseDevice *CreateDevice()
    {
        return sitech_mount.get();
    }
}
