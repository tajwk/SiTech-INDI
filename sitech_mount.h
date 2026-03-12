#pragma once

#include <inditelescope.h>
#include <indicom.h>
#include <connectionplugins/connectiontcp.h>
#include <indiguiderinterface.h>

#include <string>
#include <memory>

class SiTechMount : public INDI::Telescope, public INDI::GuiderInterface
{
public:
    SiTechMount();
    virtual ~SiTechMount() = default;

    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;

protected:
    virtual bool Handshake() override;
    virtual bool ReadScopeStatus() override;
    virtual bool Goto(double RA, double Dec) override;
    virtual bool Sync(double RA, double Dec) override;
    virtual bool Abort() override;
    virtual bool Park() override;
    virtual bool UnPark() override;
    virtual bool SetCurrentPark() override;
    virtual bool SetDefaultPark() override;
    virtual bool MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command) override;
    virtual bool MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command) override;
    virtual bool SetTrackMode(uint8_t mode) override;
    virtual bool SetTrackEnabled(bool enabled) override;

    // GuiderInterface methods
    virtual IPState GuideNorth(uint32_t ms) override;
    virtual IPState GuideSouth(uint32_t ms) override;
    virtual IPState GuideEast(uint32_t ms) override;
    virtual IPState GuideWest(uint32_t ms) override;

private:
    // Communication methods
    bool sendCommand(const std::string &command, std::string &response);
    bool sendTCPCommand(const std::string &command, std::string &response);
    bool sendSerialCommand(const std::string &command, std::string &response);
    
    // Protocol parsing
    bool parseStatusResponse(const std::string &response);
    bool parseTCPResponse(const std::string &response);
    
    // Utility methods
    std::string calculateChecksum(const std::string &command);
    bool isASCIIChecksumMode() { return use_ascii_checksum; }
    
    // Properties
    INumberVectorProperty SlewRateNP;
    INumber SlewRateN[2];
    
    ISwitchVectorProperty ASCIIChecksumSP;
    ISwitch ASCIIChecksumS[2];
    
    ITextVectorProperty ScopeInfoTP;
    IText ScopeInfoT[4];
    
    INumberVectorProperty SiteLocationNP;
    INumber SiteLocationN[3];
    
    // Status tracking
    bool scope_initialized;
    bool scope_tracking;
    bool scope_slewing;
    bool scope_parking;
    bool scope_parked;
    bool scope_looking_east;
    bool controller_blinky;
    bool communication_fault;
    bool use_ascii_checksum;
    
    // Position tracking
    double current_ra;
    double current_dec;
    double current_alt;
    double current_az;
    double scope_sidereal_time;
    double scope_julian_day;
    double scope_time;
    double air_mass;
    
    // Connection
    Connection::TCP *tcpConnection { nullptr };
    int PortFD { -1 };
    
    // Constants
    static constexpr double SIDEREAL_RATE = 15.041067; // arcsec/sec
    static constexpr int SITECH_TIMEOUT = 3; // seconds
    static constexpr int SITECH_BUFFER_SIZE = 1024;
};
