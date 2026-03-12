# SiTech TCP Protocol Command Reference

This document shows the exact commands sent by the INDI driver and their expected responses.

## Status and Information Commands

### ReadScopeStatus() - Poll every second
**Command:** `"\n"` (empty command, just newline)
**Response Format:**
```
boolParms;RA;Dec;Alt;Az;SecondaryAngle;PrimaryAngle;SiderealTime;JulianDay;ScopeTime;AirMass;_message
```
**Example:**
```
3;12.5;45.0;60.0;180.0;45.0;180.0;6.5;2459635.5;12.5;1.2;_
```
**Fields:**
- boolParms: Integer with bit flags (see below)
- RA: Right Ascension in hours (JNow)
- Dec: Declination in degrees (JNow)
- Alt: Altitude in degrees
- Az: Azimuth in degrees
- SecondaryAngle: Secondary axis angle in degrees
- PrimaryAngle: Primary axis angle in degrees
- SiderealTime: Sidereal time in hours
- JulianDay: Julian day number
- ScopeTime: Scope time in hours
- AirMass: Air mass value
- _message: Status message starting with underscore

**Boolean Flags (boolParms):**
- Bit 0 (AND 1): Scope Is Initialized
- Bit 1 (AND 2): Scope Is Tracking
- Bit 2 (AND 4): Scope is Slewing
- Bit 3 (AND 8): Scope is Parking
- Bit 4 (AND 16): Scope is Parked
- Bit 5 (AND 32): Scope is Looking East (GEM)
- Bit 6 (AND 64): Servo Controller in Blinky (Manual) mode
- Bit 7 (AND 128): Communication fault with ServoController

### ScopeInfo - Get telescope specifications
**Command:** `"ScopeInfo\n"`
**Response Format:**
```
ApertureDiameter;ApertureArea;FocalLength;NameOfScope;_ScopeInfo
```
**Example:**
```
200;31416;2000;My Telescope;_ScopeInfo
```
**Fields:**
- ApertureDiameter: Aperture diameter in mm
- ApertureArea: Aperture area in square mm
- FocalLength: Focal length in mm
- NameOfScope: Name of the telescope

### SiteLocations - Get observatory location
**Command:** `"SiteLocations\n"`
**Response Format:**
```
Latitude;Longitude;Elevation;_SiteLocations
```
**Example:**
```
42.360081;-71.088571;43.0;_SiteLocations
```
**Fields:**
- Latitude: Site latitude in degrees
- Longitude: Site longitude in degrees
- Elevation: Site elevation in meters

## Movement Commands

### GoTo - Slew to coordinates
**Command:** `"GoTo RA Dec J2K\n"`
**Parameters:**
- RA: Right Ascension in hours (J2000)
- Dec: Declination in degrees (J2000)
- J2K: Optional flag indicating J2000 coordinates

**Example:**
```
GoTo 12.5 45.0 J2K\n
```

**Response:** Standard return string

**Notes:**
- Without J2K flag, coordinates are assumed to be JNow
- With J2K flag, SiTechExe applies precession, nutation, and aberration corrections
- INDI provides J2000 coordinates, so we always use J2K flag

### Sync - Synchronize mount position
**Command:** `"Sync RA Dec mode J2K\n"`
**Parameters:**
- RA: Right Ascension in hours (J2000)
- Dec: Declination in degrees (J2000)
- mode: Sync type (0=standard, 1=instant offset, 2=instant calibration)
- J2K: Optional flag indicating J2000 coordinates

**Example:**
```
Sync 12.5 45.0 2 J2K\n
```

**Response:** Standard return string

**Notes:**
- Mode 0: Uses SiTechExe standard Init Window (popup)
- Mode 1: Instant Offset init (no popup)
- Mode 2: Instant "load calibration" init (no popup) - **Used by driver**
- With J2K flag, SiTechExe applies precession, nutation, and aberration corrections

### Abort - Stop all motion
**Command:** `"Abort\n"`
**Response:** Standard return string
**Effect:** Aborts any slews and stops tracking

## Parking Commands

### Park - Park the mount
**Command:** `"Park\n"`
**Response:** Standard return string
**Effect:** Moves to park position and sets parked status

### UnPark - Unpark the mount
**Command:** `"UnPark\n"`
**Response:** Standard return string
**Effect:** Clears parked status, ready for GoTo commands

### SetPark - Set current position as park
**Command:** `"SetPark\n"`
**Response:** 
```
;_SetPark Command Successful
```
or
```
You can't set park if you're tracking.
```

## Tracking Commands

### SetTrackMode - Enable/disable tracking
**Command:** `"SetTrackMode enabled useRates raRate decRate\n"`
**Parameters:**
- enabled: 1 = track, 0 = stop tracking
- useRates: 1 = use custom rates, 0 = sidereal rate
- raRate: RA rate in arcseconds per second (0.0 = sidereal)
- decRate: Dec rate in arcseconds per second (0.0 = tracking)

**Examples:**
```
SetTrackMode 1 0 0.0 0.0\n   # Start sidereal tracking
SetTrackMode 0 0 0.0 0.0\n   # Stop tracking
SetTrackMode 1 1 15.5 0.0\n  # Track at custom rate
```

**Response:** Standard return string

## Manual Motion Commands

### MoveAxisPri - Move primary axis
**Command:** `"MoveAxisPri dps\n"`
**Parameters:**
- dps: Degrees per second (negative = reverse direction, 0 = stop)

**Example:**
```
MoveAxisPri 1.5\n   # Move at 1.5 deg/sec
MoveAxisPri -1.5\n  # Move reverse at 1.5 deg/sec
MoveAxisPri 0\n     # Stop
```

**Response:** Standard return string

**Note:** If no additional MoveAxisPri command is received within ~10 seconds, motion will stop automatically

### MoveAxisSec - Move secondary axis
**Command:** `"MoveAxisSec dps\n"`
**Parameters:**
- dps: Degrees per second (negative = reverse direction, 0 = stop)

**Example:**
```
MoveAxisSec 1.5\n
```

**Response:** Standard return string

## Guiding Commands

### PulseGuide - Guide pulse
**Command:** `"PulseGuide Direction, Milliseconds\n"`
**Parameters:**
- Direction: 0=North, 1=South, 2=East, 3=West
- Milliseconds: Duration of guide pulse

**Examples:**
```
PulseGuide 0, 1000\n   # Guide north for 1 second
PulseGuide 1, 500\n    # Guide south for 0.5 seconds
PulseGuide 2, 1000\n   # Guide east for 1 second
PulseGuide 3, 750\n    # Guide west for 0.75 seconds
```

**Response:** Standard return string

**Note:** 
- The comma separator is REQUIRED
- Mount moves based on guide rate stored in controller configuration
- Movement distance = guide_rate * (milliseconds / 1000)
- Changes telescope setpoint, doesn't directly control motors

## Error Handling

### Communication Errors
If a command fails:
1. Check PortFD is valid (>= 0)
2. Verify SiTechExe is running
3. Confirm TCP port 8079 is correct
4. Check network connectivity

### Response Parsing Errors
If response parsing fails:
1. Log shows "Invalid TCP response - expected at least N fields"
2. Check SiTechExe version (protocol requires 0.92e or later)
3. Verify response format matches specification

### Mount Status Errors
If boolParms bit flags indicate problems:
- Bit 6 set: Controller in Blinky (manual) mode - commands will fail
- Bit 7 set: Communication fault with ServoController - check hardware

## Protocol Version
Based on SiTech TCP Protocol version 0.96R (2025-07-08)

## Implementation Notes

### Coordinate Systems
- INDI uses J2000 coordinates internally
- Driver sends J2K flag to SiTechExe for automatic corrections
- SiTechExe handles precession, nutation, and aberration
- Mount operates in JNow coordinates internally

### Response Timing
- Commands return immediately (non-blocking in SiTechExe)
- Status polling occurs every 1 second via timer
- Long operations (slews) complete asynchronously
- Check boolParms flags to monitor operation status

### TCP Connection
- Persistent connection on port 8079
- Connection established during Handshake()
- Single shared PortFD for all commands
- No connection pooling or multiplexing needed
