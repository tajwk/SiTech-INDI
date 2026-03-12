# SiTech TCP Protocol Implementation Fix Summary

## Overview
Fixed the INDI driver to use the actual SiTech TCP protocol as specified in SiTechTCPProtocol.txt, replacing all placeholder commands with real protocol commands.

## Changes Made

### 1. ReadScopeStatus() - Line ~212
**Before:** Sent `"ScopeInfo\n"` (invalid command for status polling)
**After:** Sends `"\n"` (empty command returns standard status string)
**Reason:** According to protocol, sending just a newline returns the standard return string with boolParms, RA, Dec, Alt, Az, etc.

### 2. Handshake() - Line ~187-210
**Before:** Only tested connection with `"ScopeInfo\n"`
**After:** 
- Tests connection with `"\n"` (basic status)
- Fetches scope information with `"ScopeInfo\n"`
- Fetches site location with `"SiteLocations\n"`

**Added Features:**
- Parses and displays scope info (aperture, focal length, name)
- Parses and displays site location (latitude, longitude, elevation)
- Updates INDI location properties automatically

### 3. PulseGuide Commands - Line ~365-402
**Before:** `"PulseGuide 0 1000\n"` (missing comma separator)
**After:** `"PulseGuide 0, 1000\n"` (correct format with comma)
**Reason:** Protocol specifies: `"PulseGuide Direction, Milliseconds\n"` - note the comma

### 4. parseTCPResponse() - Line ~509-606
**Enhanced:**
- Added detailed logging of response
- Added descriptive comments for each field
- Added validation of response field count
- Added warning logs for error conditions (Blinky mode, communication faults)
- Improved error messages

**Correctly parses standard return string:**
```
boolParms;RA;Dec;Alt;Az;SecondaryAngle;PrimaryAngle;SiderealTime;JulianDay;ScopeTime;AirMass;_message
```

**Boolean flags extracted (from boolParms):**
- Bit 0: Scope Is Initialized
- Bit 1: Scope Is Tracking
- Bit 2: Scope is Slewing
- Bit 3: Scope is Parking
- Bit 4: Scope is Parked
- Bit 5: Scope is "Looking East" (GEM)
- Bit 6: Servo Controller is in "Blinky" (Manual) mode
- Bit 7: Communication fault with ServoController

### 5. New Helper Methods Added

#### parseScopeInfo()
- Parses `ScopeInfo\n` response
- Format: `ApertureDiameter;ApertureArea;FocalLength;NameOfScope;_ScopeInfo`
- Updates ScopeInfo properties in INDI
- Logs scope details for debugging

#### parseSiteLocation()
- Parses `SiteLocations\n` response
- Format: `Latitude;Longitude;Elevation;_SiteLocations`
- Updates both SiteLocation and Location properties
- Logs site coordinates for debugging

### 6. Header File Changes
Added function declarations:
- `bool parseScopeInfo(const std::string &response);`
- `bool parseSiteLocation(const std::string &response);`

### 7. GoTo and Sync Commands Enhanced - Line ~238, 250
**Before:** `"GoTo RA Dec\n"` and `"Sync RA Dec\n"` (sent JNow coordinates)
**After:** 
- GoTo: `"GoTo RA Dec J2K\n"` 
- Sync: `"Sync RA Dec 2 J2K\n"`

**Reason:** 
- INDI provides J2000 coordinates by default
- The J2K flag tells SiTechExe to handle precession, nutation, and aberration corrections
- Sync mode 2 = instant "load calibration" init (no popup window)

## Commands Already Correct

The following commands were already using the correct protocol format:
- ✅ **Abort**: `"Abort\n"`
- ✅ **Park**: `"Park\n"`
- ✅ **UnPark**: `"UnPark\n"`
- ✅ **SetPark**: `"SetPark\n"`
- ✅ **SetTrackMode**: `"SetTrackMode 1 0 0.0 0.0\n"`
- ✅ **MoveAxisPri/Sec**: `"MoveAxisPri dps\n"`, `"MoveAxisSec dps\n"`

## Commands Enhanced

The following commands were correct but have been improved to use advanced features:
- 🔧 **GoTo**: Now uses `"GoTo RA Dec J2K\n"` to send J2000 coordinates
- 🔧 **Sync**: Now uses `"Sync RA Dec 2 J2K\n"` for instant calibration with J2000

## Testing Recommendations

1. **Connection Test:** Verify driver connects and retrieves status
2. **Status Polling:** Confirm status updates every second showing correct RA/Dec
3. **Scope Info:** Check that aperture, focal length, and name display correctly
4. **Site Location:** Verify latitude, longitude, and elevation are fetched
5. **GoTo Test:** Perform slew and verify mount moves
6. **Tracking:** Enable tracking and verify status bit changes
7. **PulseGuide:** Test guiding in all four directions
8. **Park/Unpark:** Test parking functionality

## Protocol Reference
All changes are based on the official SiTech TCP Protocol document (SiTechTCPProtocol.txt) version 0.96R.

## Compilation Status
✅ All changes compile without errors or warnings

## Expected Behavior
- Driver should now successfully communicate with SiTechExe over TCP
- Status updates should work correctly
- All mount control functions should operate properly
- Error conditions should be logged appropriately
