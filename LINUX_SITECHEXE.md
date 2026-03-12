# Running SiTechExe on Linux

## Overview
While SiTechExe is natively a Windows application, it can be run on Linux systems using Wine (Windows compatibility layer). This allows you to run both SiTechExe and the INDI driver on the same Linux computer.

## Installation Methods

### Method 1: Wine (Recommended)

#### Install Wine
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install wine winetricks

# CentOS/RHEL/Fedora
sudo dnf install wine winetricks

# Arch Linux
sudo pacman -S wine winetricks
```

#### Configure Wine
```bash
# Run Wine configuration
winecfg

# In winecfg:
# - Set Windows version to "Windows 10" 
# - Configure graphics and audio as needed
# - Apply and OK
```

#### Install SiTechExe in Wine
```bash
# Create a Wine prefix for SiTech (optional but recommended)
export WINEPREFIX=~/.wine-sitech
winecfg

# Install Windows dependencies if needed
winetricks vcrun2019 dotnet48

# Install SiTechExe
# Download SiTechExe installer from Sidereal Technology
wine SiTechExeInstaller.exe

# Or if you have existing SiTech installation:
# Copy SiTech folder to Wine drive_c
```

#### Run SiTechExe
```bash
# Navigate to SiTech installation directory
cd ~/.wine/drive_c/Program\ Files/SiTech/

# Run SiTechExe
wine SiTechExe.exe
```

### Method 2: Virtual Machine
Run Windows in a VM (VirtualBox, VMware, etc.) and install SiTechExe there. Connect via TCP/IP from Linux host.

### Method 3: Separate Windows Computer
Run SiTechExe on a dedicated Windows computer and connect via network.

## Configuration for INDI Driver

### Same Machine Setup (Wine)
If running SiTechExe via Wine on the same Linux computer:

1. **Start SiTechExe in Wine**
   ```bash
   wine ~/.wine/drive_c/Program\ Files/SiTech/SiTechExe.exe
   ```

2. **Configure TCP Port in SiTechExe**
   - Go to Config → Change Config → Misc
   - Set "Indi Port Number" (default: 8079)
   - Save configuration

3. **Configure INDI Driver**
   - Connection method: TCP/IP
   - Host: `localhost` or `127.0.0.1`
   - Port: `8079` (or whatever you configured)

### Network Setup
If running SiTechExe on a separate computer:

1. **On SiTechExe Computer**
   - Configure TCP port as above
   - Note the computer's IP address
   - Ensure Windows firewall allows the port

2. **On INDI Computer**
   - Connection method: TCP/IP
   - Host: IP address of SiTechExe computer
   - Port: 8079 (or configured port)

## Advantages of Each Method

### Wine on Same Machine
✅ Single computer setup
✅ No network latency
✅ Simplified configuration
❌ Wine compatibility issues possible
❌ Resource usage on observatory computer

### Separate Windows Computer
✅ Native Windows performance
✅ Isolated from Linux issues
✅ Can run other Windows astronomy software
❌ Additional hardware required
❌ Network dependency

### Virtual Machine
✅ Isolated Windows environment
✅ Can snapshot/backup VM state
❌ Higher resource usage
❌ VM overhead

## Troubleshooting Wine Setup

### Common Issues

#### SiTechExe Won't Start
```bash
# Check Wine version
wine --version

# Install Visual C++ runtime
winetricks vcrun2019

# Check for missing DLLs
wine SiTechExe.exe  # Look for missing DLL errors
```

#### TCP Port Issues
```bash
# Check if Wine is listening on the port
netstat -tulpn | grep :8079

# Test TCP connection
telnet localhost 8079
```

#### Serial Port Access in Wine
```bash
# Wine can access Linux serial ports
# Map serial ports in winecfg under Ports tab
# /dev/ttyUSB0 → COM1
# /dev/ttyS0 → COM2
```

### Performance Optimization
```bash
# Reduce Wine debug output
export WINEDEBUG=-all

# Use a dedicated Wine prefix
export WINEPREFIX=~/.wine-sitech

# Allocate more memory to Wine
winecfg  # Graphics tab → increase video memory
```

## Alternative: Direct Serial Connection

If Wine proves problematic, you can bypass SiTechExe entirely:

### Use Serial Mode
1. Connect SiTech controller directly to Linux computer via serial
2. Configure INDI driver for serial connection
3. Use direct serial protocol (more limited functionality)

### Advantages
✅ No Wine dependency
✅ Direct hardware control
✅ Lower latency
❌ Limited features compared to SiTechExe
❌ No mount model/calibration from SiTechExe

## Recommended Setup

For most users, I recommend:

1. **Development/Testing**: Wine on same Linux computer
2. **Production Observatory**: Dedicated Windows computer for SiTechExe
3. **Portable Setup**: Wine on observatory laptop
4. **Minimal Setup**: Direct serial connection

## Testing Your Setup

### Verify SiTechExe is Running
```bash
# Check if TCP port is open
nmap -p 8079 localhost

# Test basic TCP communication (note: requires \n terminator)
echo -e "\n" | nc localhost 8079
```

### Test INDI Driver Connection
```bash
# Start INDI server with SiTech driver
indiserver indi_sitech_mount

# Connect with test client
indi_getprop "SiTech Mount.CONNECTION.CONNECT"
```

## Support

If you encounter issues with Wine:
- Check Wine AppDB for SiTechExe compatibility
- Wine forums and documentation
- Consider using PlayOnLinux/Lutris for easier Wine management

The TCP/IP protocol is platform-agnostic, so once SiTechExe is running (whether natively on Windows or via Wine on Linux), the INDI driver will work the same way.
