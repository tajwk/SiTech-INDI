# SiTech INDI Driver - Installation and Usage Guide

## Overview
This is a complete INDI driver package for SiTech telescope mounts. Since you're developing on Windows but targeting Linux, here's how to use this driver package.

## Package Contents

### Core Driver Files
- `sitech_mount.h` - Header file with class definitions
- `sitech_mount.cpp` - Main driver implementation
- `CMakeLists.txt` - Build configuration for Linux
- `indi_sitech.xml` - Driver registration file

### Documentation
- `README.md` - Comprehensive user documentation
- `PROTOCOL_NOTES.md` - Technical implementation details
- `SiTechSerialProtocol.txt` - Original serial protocol specification
- `SiTechTCPProtocol.txt` - Original TCP protocol specification

### Build Scripts
- `build.sh` - Linux build script
- `build.bat` - Windows build script (for development only)
- `99-sitech.rules` - Linux udev rules for device permissions

## Installation on Linux (Target Platform)

### Method 1: Build from Source
1. Copy all files to a Linux system with INDI development environment
2. Install dependencies:
   ```bash
   sudo apt-get update
   sudo apt-get install libindi-dev libnova-dev cmake build-essential
   ```
3. Build and install:
   ```bash
   chmod +x build.sh
   ./build.sh
   cd build
   sudo make install
   ```

### Method 2: Transfer and Build
1. Package the files on Windows:
   ```powershell
   Compress-Archive -Path * -DestinationPath sitech-indi-driver.zip
   ```
2. Transfer to Linux system
3. Extract and build as above

## Driver Features

### Communication Protocols
- **TCP/IP**: Connects to SiTechExe software on Windows
- **Serial**: Direct connection to SiTech servo controllers

### Telescope Control
- GoTo (slew to coordinates)
- Sync (plate solving alignment)
- Park/Unpark operations
- Abort (emergency stop)
- Manual slewing (N/S/E/W)
- Tracking control (sidereal, solar, lunar)

### Guiding Support
- Pulse guiding for autoguiding software
- Configurable guide rates
- Compatible with PHD2, LinGuider, etc.

### Status Monitoring
- Real-time position feedback
- Mount status (tracking, slewing, parked, etc.)
- Communication fault detection
- Limit switch monitoring

## Configuration

### TCP Connection (Recommended)
1. Run SiTechExe on computer (Windows typically, or Linux via Wine)
2. Configure TCP port in SiTechExe (default: 8079)
3. In INDI client:
   - Select "SiTech Mount" driver
   - Choose "TCP/IP" connection
   - Set host IP and port (use "localhost" or "127.0.0.1" if same machine)
   - Connect

**Note**: SiTechExe can run on the same Linux computer as the INDI driver, or on a separate computer accessible via network.

### Serial Connection
1. Connect SiTech controller via serial cable
2. In INDI client:
   - Select "SiTech Mount" driver  
   - Choose "Serial" connection
   - Configure port and baud rate
   - Enable ASCII checksum if needed
   - Connect

## Supported INDI Clients

### KStars/Ekos
- Full integration with mount control
- GoTo and sync operations
- Guiding support via PHD2
- Observatory automation

### PHD2 Guiding
- Direct pulse guiding support
- Automatic guide rate detection
- Mount settling detection

### Other Clients
- INDI Web Manager
- CCDciel
- FireCapture
- Any INDI-compatible software

## Troubleshooting

### Connection Issues
1. **TCP Connection Fails**
   - Verify SiTechExe is running
   - Check Windows firewall settings
   - Ensure correct IP address and port
   - Test with telnet: `telnet <ip> <port>`

2. **Serial Connection Fails**
   - Check cable and connections
   - Verify baud rate settings
   - Test permissions: `sudo usermod -a -G dialout $USER`
   - Try different USB ports

### Mount Control Issues
1. **GoTo Doesn't Work**
   - Ensure mount is initialized
   - Check coordinate limits
   - Verify mount not in "blinky" mode
   - Check for communication faults

2. **Tracking Problems**
   - Verify polar alignment
   - Check tracking rates
   - Monitor for limit switches
   - Ensure mount model loaded (TCP mode)

### Status Messages
- **Communication Fault**: Hardware connection problem
- **Blinky Mode**: Controller in manual mode
- **Below Horizon**: Target below elevation limit
- **Limit Switch**: Mechanical obstruction detected

## Development Notes

### Windows Development
Since you're developing on Windows:
1. Use Visual Studio Code with C++ extensions
2. Install Windows Subsystem for Linux (WSL) for testing
3. Use cross-compilation tools if needed
4. Test protocol implementation with SiTechExe directly

### Code Structure
```
sitech_mount.h          - Class definitions and interface
sitech_mount.cpp        - Implementation
├── Connection handling
├── Protocol parsing  
├── INDI property management
├── Telescope operations
└── Error handling
```

### Protocol Implementation
- TCP commands are text-based with '\n' terminators
- Serial commands use '\r' terminators with optional checksums
- Status parsing handles both ASCII and binary responses
- Error recovery and reconnection logic included

## Testing Recommendations

### Unit Testing
1. Test TCP and serial communication separately
2. Verify command parsing and response handling
3. Test coordinate transformations
4. Validate error handling

### Integration Testing
1. Test with actual SiTech hardware
2. Verify all mount operations
3. Test with multiple INDI clients
4. Stress test communication reliability

### Field Testing
1. Test under actual observing conditions
2. Verify tracking accuracy
3. Test autoguiding integration
4. Monitor for connection stability

## Support and Contributions

### Getting Help
- INDI Forum: https://indilib.org/forum/
- SiTech user groups
- GitHub issues (when published)

### Contributing
1. Fork the repository
2. Create feature branches
3. Follow INDI coding standards
4. Add documentation and tests
5. Submit pull requests

### Reporting Bugs
Include:
- Driver version
- SiTech controller model
- Operating system details
- Connection method used
- Detailed error messages
- Log files

## License
Released under LGPL v2.1, compatible with INDI library licensing.

## Future Enhancements
- Binary protocol support for advanced features
- Satellite tracking capabilities
- Enhanced mount model integration
- Configuration management tools
- Performance optimizations
