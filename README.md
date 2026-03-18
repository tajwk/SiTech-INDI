# SiTech INDI Driver

This is an INDI driver for SiTech telescope mounts, supporting both serial and TCP/IP communication protocols.

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
1. Start SiTechExe (Windows native or Linux via Mono)
2. Configure the TCP port in SiTechExe: Config → Change Config → Misc → "Indi Port Number"
3. In INDI client, select "TCP/IP" connection method
4. Set host to the IP address of the computer running SiTechExe (or "localhost" if same machine)
5. Set port to match the configured port in SiTechExe (default: 8079)






