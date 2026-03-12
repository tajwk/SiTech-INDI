# SiTech INDI Driver

This is an INDI driver for SiTech telescope mounts, supporting both serial and TCP/IP communication protocols.

## Features

- **Dual Communication Support**: Both serial (RS-232) and TCP/IP connections
- **Full Mount Control**: GoTo, Sync, Park/Unpark, Abort, Manual slewing
- **Tracking Modes**: Sidereal, Solar, Lunar tracking support
- **Guiding**: Pulse guiding support for autoguiding
- **ASCII Checksum Mode**: Support for SiTech's ASCII checksum protocol
- **Mount Information**: Retrieves scope and site information from SiTechExe
- **Status Monitoring**: Real-time mount status and position feedback

## Supported Protocols

### TCP/IP Protocol
- Connects to SiTechExe software (typically Windows, but can run on Linux via Wine)
- Default port: 8079 (configurable in SiTechExe)
- Full command set including coordinate transformations
- Real-time status updates
- Can run on same machine as INDI driver or separate networked computer

### Serial Protocol  
- Direct connection to SiTech servo controller
- ASCII command set with optional checksum mode
- Binary command support for advanced operations
- Compatible with SiTech Servo II and newer controllers

## Installation

### Prerequisites
- INDI library (version 1.8.0 or later)
- libnova development libraries
- CMake 3.16 or later
- C++17 compatible compiler

### Building from Source

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Package Installation

#### Debian/Ubuntu
```bash
sudo dpkg -i indi-sitech_1.0.0_amd64.deb
```

#### Red Hat/CentOS/Fedora
```bash
sudo rpm -i indi-sitech-1.0.0.x86_64.rpm
```

## Configuration

### TCP/IP Connection
1. Start SiTechExe (Windows native or Linux via Wine)
2. Configure the TCP port in SiTechExe: Config → Change Config → Misc → "Indi Port Number"
3. In INDI client, select "TCP/IP" connection method
4. Set host to the IP address of the computer running SiTechExe (or "localhost" if same machine)
5. Set port to match the configured port in SiTechExe (default: 8079)

### Serial Connection
1. Connect SiTech controller to your computer via serial cable
2. In INDI client, select "Serial" connection method  
3. Configure serial port (e.g., /dev/ttyUSB0) and baud rate
4. Enable ASCII checksum mode if using noisy serial connection

## Usage

### Basic Operations

#### Connection
1. Start your INDI client (KStars, PHD2, etc.)
2. Select "SiTech Mount" driver
3. Choose connection method (TCP or Serial)
4. Configure connection parameters
5. Connect to the mount

#### Telescope Control
- **GoTo**: Send telescope to specific RA/Dec coordinates
- **Sync**: Synchronize mount position with known star position
- **Park**: Move telescope to park position and stop tracking
- **Unpark**: Resume operations from park position
- **Abort**: Stop all telescope movement immediately

#### Tracking
- **Sidereal**: Standard star tracking (default)
- **Solar**: Track the Sun (use with appropriate solar filters!)
- **Lunar**: Track the Moon
- **Custom**: Set custom tracking rates

#### Manual Slewing
- Use direction buttons in INDI client
- Adjustable slew rates from 0.01°/s to 10°/s
- Separate guide rate setting for precision movements

### Advanced Features

#### Pulse Guiding
The driver supports autoguiding through pulse guide commands:
- North/South guiding via declination axis
- East/West guiding via right ascension axis
- Configurable guide rates
- Compatible with PHD2 and other guiding software

#### Mount Information
The driver automatically retrieves:
- Telescope specifications (aperture, focal length, etc.)
- Site location (latitude, longitude, elevation)
- Mount status and tracking state
- Pier side information for German equatorial mounts

## Troubleshooting

### Connection Issues

#### TCP Connection
- Verify SiTechExe is running and configured correctly
- Check Windows firewall settings
- Ensure no other software is connected to SiTechExe
- Try telnet to test basic TCP connectivity

#### Serial Connection
- Check cable connections and pinouts
- Verify baud rate matches controller setting
- Test with terminal program first
- Check Linux device permissions (add user to dialout group)

### Mount Control Issues

#### GoTo Problems
- Ensure mount is initialized and tracking
- Check that coordinates are within mount limits
- Verify mount is not in "Blinky" (manual) mode
- Check for communication faults in status

#### Tracking Issues
- Verify mount model is loaded in SiTechExe (TCP mode)
- Check tracking rates are set correctly
- Ensure mount is properly polar aligned
- Monitor for limit switch activations

### Status Monitoring
The driver reports various status conditions:
- **Communication Fault**: Check cable connections
- **Blinky Mode**: Controller in manual mode, use MotorsToAuto command
- **Limit Switches**: Check for mechanical obstructions
- **Below Horizon**: Target below horizon limit setting

## Configuration Files

### SiTechExe Configuration
Important SiTechExe settings for INDI operation:
- Indi Port Number: TCP port for connections
- Horizon Limit: Minimum elevation for GoTo operations
- Park Positions: Define park locations
- Mount Model: Load pointing model for accuracy

### INDI Configuration
Driver settings saved in INDI configuration:
- Connection parameters (host, port, device)
- Slew and guide rates
- ASCII checksum mode preference
- Park position coordinates

## Protocol Reference

### TCP Commands
Key TCP commands supported:
- `""`: Get status (standard return string)
- `"GoTo RA Dec\n"`: Slew to coordinates
- `"Sync RA Dec\n"`: Sync to coordinates  
- `"Park\n"`: Park telescope
- `"Abort\n"`: Stop all motion
- `"SetTrackMode enabled useCustomRates raRate decRate\n"`: Set tracking

### Serial Commands
Key serial commands supported:
- `\r`: Get controller status
- `XS###\r`: Set X-axis speed
- `YS###\r`: Set Y-axis speed
- `Q`: Emergency stop
- ASCII checksum mode for reliable communication

## Development

### Code Structure
- `sitech_mount.h`: Main driver class definition
- `sitech_mount.cpp`: Driver implementation
- `CMakeLists.txt`: Build configuration
- `indi_sitech.xml`: Driver registration

### Contributing
1. Fork the repository
2. Create feature branch
3. Implement changes with proper error handling
4. Add documentation and tests
5. Submit pull request

### Testing
Recommended testing procedures:
1. Test both TCP and serial connections
2. Verify all mount operations (GoTo, Sync, Park, etc.)
3. Test pulse guiding functionality
4. Verify status reporting accuracy
5. Test error handling and recovery

## Support

### Documentation
- SiTech Operations Manual
- INDI Developer Documentation
- Protocol specifications (included with driver)

### Community
- INDI Forum: https://indilib.org/forum/
- SiTech User Groups
- Astronomy software communities

### Bug Reports
Please report issues with:
- Driver version
- SiTech controller model  
- Operating system
- Connection method used
- Detailed error messages
- Log files when possible

## License

This driver is released under the LGPL v2.1 license, compatible with the INDI library licensing.

## Acknowledgments

- Dan Gray and Sidereal Technology for SiTech hardware and protocols
- INDI development team for the excellent framework
- SiTech community for testing and feedback

## Version History

### v1.0.0
- Initial release
- TCP/IP and serial communication support
- Full telescope control functionality
- Pulse guiding support
- ASCII checksum mode
- Status monitoring and error reporting
