# Running SiTech INDI Driver on Raspberry Pi

## Overview

The SiTech INDI driver runs excellently on Raspberry Pi, making it ideal for:
- **Remote Observatory Control**: Headless operation via network
- **Dedicated INDI Server**: Lightweight mount controller
- **Integration Hub**: Connect mount, cameras, and accessories
- **Low Power Solution**: Run 24/7 observatory automation

## Supported Models

All Raspberry Pi models with Raspberry Pi OS (32-bit or 64-bit):
- ✅ **Raspberry Pi 5** (Recommended - best performance)
- ✅ **Raspberry Pi 4** (Excellent - 2GB+ RAM recommended)
- ✅ **Raspberry Pi 3 B+** (Good - sufficient for most setups)
- ✅ **Raspberry Pi Zero 2 W** (Basic - driver only, limited clients)
- ⚠️ **Raspberry Pi 3/2/Zero** (Possible but may be slow with multiple clients)

## Quick Installation

### One-Line Install
```bash
curl -fsSL https://raw.githubusercontent.com/tajwk/SiTech-INDI/main/install.sh | bash
```

### Manual Installation (Recommended)

1. **Update System**
   ```bash
   sudo apt-get update
   sudo apt-get upgrade -y
   ```

2. **Install Dependencies**
   ```bash
   sudo apt-get install -y libindi-dev libnova-dev cmake build-essential git
   ```

3. **Clone from GitHub**
   ```bash
   git clone https://github.com/tajwk/SiTech-INDI.git
   cd SiTech-INDI
   
   # OR transfer your files from Windows:
   # scp -r /path/to/driver pi@raspberrypi.local:~/sitech-indi-driver
   ```

4. **Build and Install**
   ```bash
   chmod +x build.sh install.sh
   ./install.sh
   ```

5. **Verify Installation**
   ```bash
   which indi_sitech_mount
   ls /usr/share/indi/indi_sitech.xml
   ```

## Network Configuration

### Typical Setup Architecture

```
┌─────────────────────┐
│   Windows PC        │
│   (SiTechExe)       │
│   192.168.1.100     │
└──────────┬──────────┘
           │ TCP/IP (Port 8079)
           │
      ┌────┴────┐ Local Network
      │         │
┌─────┴─────────────┐         ┌──────────────────┐
│  Raspberry Pi     │◄────────│  Your Laptop     │
│  INDI Server      │  WiFi   │  (KStars/Ekos)   │
│  192.168.1.50     │         │  192.168.1.200   │
└────────┬──────────┘         └──────────────────┘
         │ USB (future serial support)
         │
    ┌────┴──────┐
    │  SiTech   │
    │  Mount    │
    └───────────┘
```

### Static IP Configuration (Recommended)

Set a static IP for your Raspberry Pi to ensure consistent connections:

1. **Edit Network Configuration**
   ```bash
   sudo nano /etc/dhcpcd.conf
   ```

2. **Add Static IP Settings** (at the end of file)
   ```
   interface eth0
   static ip_address=192.168.1.50/24
   static routers=192.168.1.1
   static domain_name_servers=192.168.1.1 8.8.8.8
   
   # For WiFi, use wlan0 instead:
   interface wlan0
   static ip_address=192.168.1.50/24
   static routers=192.168.1.1
   static domain_name_servers=192.168.1.1 8.8.8.8
   ```

3. **Reboot to Apply**
   ```bash
   sudo reboot
   ```

## Running the Driver

### Method 1: Direct Command Line

Start the INDI server with the SiTech driver:
```bash
indiserver -v indi_sitech_mount
```

For verbose logging:
```bash
indiserver -v -v indi_sitech_mount
```

### Method 2: Using INDI Web Manager (Recommended for Remote Use)

1. **Install INDI Web Manager**
   ```bash
   sudo pip3 install indiweb
   ```

2. **Enable as System Service**
   ```bash
   sudo systemctl enable indi-web
   sudo systemctl start indi-web
   ```

3. **Access Web Interface**
   - Open browser to: `http://raspberrypi.local:8624`
   - Or use IP: `http://192.168.1.50:8624`

4. **Start Driver**
   - In web interface, select "SiTech Mount"
   - Click "Start" - driver runs on port 7624 (default)

### Method 3: Autostart on Boot

Create a systemd service:

1. **Create Service File**
   ```bash
   sudo nano /etc/systemd/system/indi-sitech.service
   ```

2. **Add Service Configuration**
   ```ini
   [Unit]
   Description=INDI SiTech Mount Driver
   After=network.target
   
   [Service]
   Type=simple
   User=pi
   ExecStart=/usr/bin/indiserver -v indi_sitech_mount
   Restart=always
   RestartSec=10
   
   [Install]
   WantedBy=multi-user.target
   ```

3. **Enable and Start Service**
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable indi-sitech.service
   sudo systemctl start indi-sitech.service
   ```

4. **Check Status**
   ```bash
   sudo systemctl status indi-sitech.service
   ```

## Connecting to SiTechExe

### Scenario 1: SiTechExe on Separate Windows PC

**Windows PC Setup (SiTechExe):**
1. Start SiTechExe
2. Configure TCP Port: Config → Change Config → Misc → "Indi Port Number" (default: 8079)
3. Note the Windows PC's IP address (e.g., `192.168.1.100`)

**Raspberry Pi Configuration:**
1. Start INDI driver (any method above)
2. From KStars/Ekos or INDI client:
   - Connection Type: TCP/IP
   - Host: `192.168.1.100` (Windows PC IP)
   - Port: `8079`
   - Connect

### Scenario 2: SiTechExe via Wine on Raspberry Pi (Not Recommended)

Running SiTechExe on the Pi itself is possible but resource-intensive:

```bash
# Install Wine (for Pi 4/5 only)
sudo apt-get install wine wine32
wine SiTechExe.exe
```

Then connect to `localhost:8079` from the driver.

**Note**: This is not recommended for older Pi models. Better to run SiTechExe on a separate PC.

## Client Connection

### From KStars/Ekos on Another Computer

1. **Configure Equipment Profile**
   - Profile Editor → Add Profile
   - Mode: Remote
   - Host: `raspberrypi.local` or `192.168.1.50`
   - Port: `7624` (default INDI port)

2. **Add Mount**
   - Ekos Setup → Equipment
   - Mount: Select "SiTech Mount"
   - Start INDI Services

3. **Configure Mount Connection**
   - INDI Control Panel → SiTech Mount → Connection
   - Type: TCP
   - Host: (IP of Windows PC running SiTechExe)
   - Port: 8079
   - Connect

### Using INDI Control Panel

```bash
# On any computer on the network:
indi_control_panel "raspberrypi.local:7624"
```

Or from Python:
```python
import PyIndi

# Connect to Pi
client = PyIndi.BaseClient()
client.setServer("192.168.1.50", 7624)
client.connectServer()
```

## Performance Optimization

### 1. Disable Unnecessary Services

Free up resources for astronomy:
```bash
# Disable Bluetooth (if not needed)
sudo systemctl disable bluetooth

# Disable WiFi power management
sudo iwconfig wlan0 power off

# Reduce GPU memory (edit /boot/config.txt)
sudo nano /boot/config.txt
# Add: gpu_mem=16
```

### 2. Enable Ethernet Over WiFi

Wired connection is more stable for observatory use:
- Connect Pi via Ethernet cable
- Provides lower latency and more reliable connection

### 3. Monitor Resources

```bash
# Check CPU and memory usage
htop

# Monitor temperature
vcgencmd measure_temp

# Watch network traffic
iftop
```

### 4. Cooling for Pi 4/5

For continuous operation, add:
- Heatsinks
- Fan (especially in enclosed observatory)
- Monitor temperature: ideally below 60°C

## Troubleshooting

### Driver Won't Start

```bash
# Check if driver is installed
which indi_sitech_mount

# Test driver directly
indi_sitech_mount --help

# Check for library issues
ldd $(which indi_sitech_mount)
```

### Connection Issues

```bash
# Test network connectivity to Windows PC
ping 192.168.1.100

# Test if SiTechExe port is accessible
telnet 192.168.1.100 8079
# or
nc -zv 192.168.1.100 8079

# Check firewall on Windows PC
# Ensure TCP port 8079 is allowed

# Check INDI server is running
ps aux | grep indiserver

# Check what ports are listening
sudo netstat -tulpn | grep indi
```

### Slow Performance

```bash
# Check CPU usage
top

# Check for throttling (Pi 4/5)
vcgencmd get_throttled
# 0x0 = OK, anything else = throttling

# Check network latency
ping raspberrypi.local

# Reduce INDI polling rate in driver settings
# Connection → Polling Period → 1000ms (1 second)
```

### Remote Access Issues

```bash
# Ensure SSH is enabled
sudo raspi-config
# → Interface Options → SSH → Enable

# Test SSH connection
ssh pi@raspberrypi.local

# For INDI Web Manager, check service
sudo systemctl status indi-web
```

## Remote Observatory Setup

### Complete Remote System

1. **Headless Operation**
   ```bash
   # No monitor needed - access via SSH
   ssh pi@raspberrypi.local
   
   # Or use INDI Web Manager
   # http://raspberrypi.local:8624
   ```

2. **VNC for Desktop Access** (Optional)
   ```bash
   # Enable VNC
   sudo raspi-config
   # → Interface Options → VNC → Enable
   
   # Connect with VNC Viewer
   # vnc://raspberrypi.local
   ```

3. **Power Management**
   - Use UPS for clean shutdown during power loss
   - Install nut (Network UPS Tools):
     ```bash
     sudo apt-get install nut
     ```

4. **Observatory Automation**
   - Use KStars/Ekos scheduler
   - Configure INDI watchdog
   - Set up automated parking

### Security Considerations

```bash
# Change default password
passwd

# Update regularly
sudo apt-get update && sudo apt-get upgrade

# Configure firewall (optional)
sudo apt-get install ufw
sudo ufw allow 22/tcp    # SSH
sudo ufw allow 7624/tcp  # INDI
sudo ufw allow 8624/tcp  # INDI Web Manager
sudo ufw enable

# Disable unused services
sudo systemctl list-unit-files --state=enabled
```

## Backup and Recovery

### Backup Configuration

```bash
# Backup INDI settings
cp -r ~/.indi ~/indi_backup_$(date +%Y%m%d)

# Backup driver
sudo cp /usr/bin/indi_sitech_mount ~/driver_backup

# Create full SD card image (from another computer)
# sudo dd if=/dev/sdX of=~/pi_backup.img bs=4M status=progress
```

### Quick Recovery

Keep a spare SD card with:
1. Fresh Raspberry Pi OS installation
2. Driver installation script saved
3. Network configuration documented

## Support and Resources

### Useful Commands

```bash
# View driver logs
journalctl -u indi-sitech -f

# Restart driver service
sudo systemctl restart indi-sitech

# Check all INDI drivers
ls /usr/bin/indi_*

# Test INDI communication
indi_getprop "SiTech Mount.*"
```

### Common Pi-Specific Issues

1. **SD Card Corruption**: Use high-quality SD card, enable read-only filesystem for long-term deployment
2. **Power Issues**: Use official 5V 3A power supply
3. **WiFi Dropouts**: Use Ethernet or configure power management off
4. **Overheating**: Add cooling, monitor temperature
5. **Slow Boot**: Disable unused services

## Next Steps

After successful installation:
1. ✅ Verify driver starts and connects to SiTechExe
2. ✅ Test connection from KStars/Ekos on your main computer
3. ✅ Configure autostart if using for observatory
4. ✅ Set up monitoring and alerts
5. ✅ Document your network configuration

For additional help, see:
- [README.md](README.md) - Full driver features and usage
- [INSTALL_GUIDE.md](INSTALL_GUIDE.md) - Detailed installation instructions
- INDI Forum: https://indilib.org/forum/

---

**Happy observing with your Raspberry Pi-powered observatory! 🔭🍓**
