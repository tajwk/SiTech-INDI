# SiTech INDI Driver

## Installation

### Prerequisites
- INDI library development files (version 1.8.0 or later), to compile the driver
- INDI server (`indiserver`), to run the driver
- libnova development libraries
- CMake 3.16 or later
- C++17 compatible compiler

### Quick Install (Debian, Ubuntu, Raspberry Pi OS)

```bash
git clone https://github.com/tajwk/SiTech-INDI.git
cd SiTech-INDI
chmod +x install.sh build.sh
./install.sh
```

This installs the dependencies, builds the driver and installs it.

### Building from Source

On Debian, Ubuntu and Raspberry Pi OS, install the dependencies first:

```bash
sudo apt-get install -y libindi-dev indi-bin libnova-dev cmake build-essential pkg-config
```

`libindi-dev` is needed to compile the driver. `indi-bin` provides `indiserver`, which is needed to run it. Install both from the same source so their versions match.

Then build and install:

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

The driver installs under `/usr` by default: the program goes to `/usr/bin/indi_sitech_mount` and the driver list entry to `/usr/share/indi/indi_sitech.xml`, which is where KStars/Ekos and INDI Web Manager look for it. To install somewhere else, pass `-DCMAKE_INSTALL_PREFIX=<path>` to `cmake`.

If you upgrade INDI later (for example from a different repository), rebuild and reinstall the driver so it matches the new `indiserver`.

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

### Ports: Two Separate Connections

With a TCP setup there are two network connections, each on its own port:

```
KStars/Ekos ──(port 7624)──► indiserver + indi_sitech_mount ──(port 8079)──► SiTechExe
```

| Port | What listens on it | Where it is set |
|------|--------------------|-----------------|
| 7624 | `indiserver` (the standard INDI port) | Ekos Profile Editor → Port (default 7624) |
| 8079 | SiTechExe | SiTechExe: Config → Change Config → Misc → "Indi Port Number", **and** the driver's Connection tab |

SiTechExe calls its setting "Indi Port Number", but it is **not** the INDI server port. It is the port the driver uses to reach SiTechExe. If you enter 8079 as the INDI server port in Ekos, Ekos tries to talk to SiTechExe instead of `indiserver` and the connection fails.

### TCP/IP Connection
1. Start SiTechExe (Windows native or Linux via Mono)
2. Configure the TCP port in SiTechExe: Config → Change Config → Misc → "Indi Port Number" (default: 8079)
3. Start `indiserver -v indi_sitech_mount`, or start the driver from Ekos or INDI Web Manager
4. Connect your INDI client (KStars/Ekos) to `indiserver` on port **7624**
5. In the INDI Control Panel → SiTech Mount → Connection tab, select "TCP/IP"
6. Set host to the IP address of the computer running SiTechExe (or "localhost" if same machine)
7. Set port to match the "Indi Port Number" configured in SiTechExe (default: **8079**)






