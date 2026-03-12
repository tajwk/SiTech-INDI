# SiTech INDI Driver Protocol Implementation Notes

## Overview
This document details the implementation of SiTech protocols in the INDI driver, based on the official SiTech protocol documentation.

## Protocol Support Matrix

| Feature | TCP/IP | Serial | Status |
|---------|--------|--------|--------|
| Basic Status | ✅ | ✅ | Implemented |
| GoTo | ✅ | ⚠️ | TCP: Full, Serial: Needs mount model |
| Sync | ✅ | ⚠️ | TCP: Full, Serial: Needs mount model |
| Park/Unpark | ✅ | ⚠️ | TCP: Full, Serial: Partial |
| Tracking Control | ✅ | ⚠️ | TCP: Full, Serial: Basic |
| Pulse Guiding | ✅ | ❌ | TCP only |
| Manual Slewing | ✅ | ❌ | TCP only |
| ASCII Checksum | N/A | ✅ | Serial only |

## TCP/IP Protocol Implementation

### Connection
- Default port: 4030
- Text-based protocol with semicolon-separated responses
- Commands terminated with `\n`
- Immediate response to all commands

### Standard Response Format
```
boolParms;RA;Dec;Alt;Az;SecondaryAngle;PrimaryAngle;SiderealTime;JulianDay;ScopeTime;AirMass;_message
```

### Boolean Parameters (boolParms)
```cpp
bool scope_initialized = (boolParms & 1) != 0;      // Bit 0
bool scope_tracking = (boolParms & 2) != 0;         // Bit 1  
bool scope_slewing = (boolParms & 4) != 0;          // Bit 2
bool scope_parking = (boolParms & 8) != 0;          // Bit 3
bool scope_parked = (boolParms & 16) != 0;          // Bit 4
bool scope_looking_east = (boolParms & 32) != 0;    // Bit 5
bool controller_blinky = (boolParms & 64) != 0;     // Bit 6
bool communication_fault = (boolParms & 128) != 0;  // Bit 7
// Additional bits for limit switches, tracking modes, etc.
```

### Key Commands Implemented

#### Status Query
```cpp
// Command: "" (empty string)
// Response: Standard format with current telescope state
```

#### GoTo
```cpp
// Command: "GoTo RA Dec\n" or "GoTo RA Dec J2K\n"  
// RA in hours, Dec in degrees
// Optional J2K for J2000 epoch coordinates
```

#### Sync
```cpp
// Command: "Sync RA Dec [n] [J2K]\n"
// n: sync type (0=window, 1=offset, 2=calibration point)
```

#### Park/Unpark
```cpp
// Park: "Park\n"
// Unpark: "UnPark\n" 
// Set Park: "SetPark\n"
```

#### Tracking
```cpp
// Command: "SetTrackMode enabled useCustomRates raRate decRate\n"
// enabled: 1=track, 0=stop
// useCustomRates: 1=use custom rates, 0=sidereal
// raRate, decRate: arcsec/sec
```

#### Pulse Guiding
```cpp
// Command: "PulseGuide direction milliseconds\n"
// direction: 0=North, 1=South, 2=East, 3=West
```

## Serial Protocol Implementation

### Connection
- RS-232 serial communication
- ASCII command set
- Commands terminated with `\r` (carriage return)
- Optional ASCII Checksum Mode for reliability

### ASCII Checksum Mode
When enabled, commands require a checksum byte:
```cpp
std::string calculateChecksum(const std::string &command) {
    uint8_t sum = 0;
    for (char c : command) {
        sum += static_cast<uint8_t>(c);
    }
    sum = ~sum; // Invert all bits
    return std::string(1, static_cast<char>(sum));
}
```

### Axis Naming Convention
- X axis = Declination (equatorial) or Altitude (alt-az)
- Y axis = Right Ascension (equatorial) or Azimuth (alt-az)

### Status Response Format
```
X# Y# XZ# YZ# XC# YC# V# T# X[AM] Y[AM] K#
```
Where:
- X, Y: Motor encoder positions
- XZ, YZ: Scope axis encoder positions  
- XC, YC: Motor currents (×100)
- V: Supply voltage (×10)
- T: CPU temperature (°F)
- XA/XM, YA/YM: Motor modes (Auto/Manual)
- K: Handpaddle status bits

### Speed Calculations
The controller loops at 1953 Hz. Speed values are:
```cpp
// Convert counts/sec to SiTech speed value
int CountsPerSecToSpeedValue(double cps) {
    return round(cps * 33.55657962109575);
}

// Convert SiTech speed value to counts/sec  
double SpeedValueToCountsPerSec(int speed) {
    return round(speed * 0.0298004150390625);
}
```

### Key Commands

#### Status Query
```cpp
// Command: "\r"
// Returns full controller status
```

#### Emergency Stop
```cpp
// Command: "Q" (no CR)
// Immediately stops all motion
```

#### Position Query
```cpp
// Command: "X\r" or "Y\r"
// Returns current motor position for axis
```

## Mount Model Integration

### TCP Mode
- SiTechExe handles all coordinate transformations
- Driver sends celestial coordinates directly
- Mount model corrections applied automatically

### Serial Mode
- Driver must handle coordinate transformations
- Requires knowledge of mount parameters:
  - Gear ratios
  - Encoder resolution
  - Mount type (GEM, fork, alt-az)
  - Pointing model corrections

## Error Handling

### Communication Errors
```cpp
// TCP: Connection loss, socket errors
// Serial: Timeout, checksum mismatch, framing errors
```

### Mount Errors
```cpp
// Below horizon limit
// Limit switch activation
// Controller in blinky (manual) mode
// Communication fault with servo controller
```

### Recovery Procedures
1. Detect communication fault
2. Attempt reconnection
3. Re-establish mount status
4. Resume operations or alert user

## Future Enhancements

### Planned Features
1. **Full Serial Protocol Support**
   - Complete mount model implementation
   - Binary command support for advanced features
   - Focuser/rotator control

2. **Advanced Tracking**
   - Satellite tracking support
   - Custom tracking rates
   - Periodic error correction

3. **Configuration Management**
   - Mount parameter storage
   - Pointing model management
   - Automated initialization sequences

### Performance Optimizations
1. **Asynchronous Communication**
   - Non-blocking I/O operations
   - Command queuing system
   - Status update threading

2. **Smart Polling**
   - Adaptive polling rates
   - Event-driven updates
   - Minimal bandwidth usage

## Testing and Validation

### Test Scenarios
1. **Connection Testing**
   - TCP and serial connections
   - Reconnection after network/cable loss
   - Multi-client TCP access

2. **Mount Operations**
   - GoTo accuracy across sky
   - Sync precision
   - Park/unpark reliability
   - Emergency stop response

3. **Tracking Performance**
   - Sidereal tracking accuracy
   - Guide pulse response
   - Rate changes

### Validation Criteria
- GoTo accuracy: < 1 arcminute RMS
- Guide pulse precision: ±10ms timing
- Status update rate: 1-10 Hz
- Connection recovery: < 5 seconds

## Troubleshooting Guide

### Common Issues
1. **"Communication Fault" Status**
   - Check serial cable connections
   - Verify baud rate settings
   - Test with SiTechExe directly

2. **GoTo Failures**
   - Ensure mount is initialized
   - Check coordinate limits
   - Verify mount model loaded

3. **Tracking Problems**
   - Check polar alignment
   - Verify sidereal rate setting
   - Monitor for limit switch hits

### Debug Logging
Enable detailed logging for troubleshooting:
```cpp
LOGF_DEBUG("TCP Response: %s", response.c_str());
LOGF_ERROR("GoTo failed: %s", error_message.c_str());
```

## References
- SiTech Operations Manual
- SiTech Serial Protocol Documentation  
- SiTech TCP Protocol Documentation
- INDI Telescope Driver Documentation
