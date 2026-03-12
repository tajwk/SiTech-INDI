# Testing Guide for SiTech TCP Protocol Fixes

## Prerequisites
- Raspberry Pi with INDI installed
- SiTechExe running on Windows/Linux (accessible via network)
- SiTechExe configured with TCP port 8079
- Mount hardware connected to SiTechExe

## Initial Connection Test

### 1. Start SiTechExe
```bash
# Make sure SiTechExe is running and configured for TCP on port 8079
# In SiTechExe: Config -> ChangeConfig -> Misc -> "Indi Port Number" = 8079
```

### 2. Start INDI Driver
```bash
cd ~/indi_driver/build
./indi_sitech_mount
```

### 3. Connect with INDI Client
```bash
# Using indi_getprop to test
indi_getprop

# Or use KStars/Ekos/INDI Web Manager
```

## Manual Testing with Direct TCP

You can test the protocol directly using netcat or telnet:

```bash
# Connect to SiTechExe TCP port
nc localhost 8079

# Or if SiTechExe is on a different machine:
nc 192.168.1.10 8079

# Send basic status command (just hit Enter)
<press Enter>

# Expected response (example):
# 3;12.5;45.0;60.0;180.0;45.0;180.0;6.5;2459635.5;12.5;1.2;_

# Test ScopeInfo command
ScopeInfo
<press Enter>

# Expected response (example):
# 200;31416;2000;My Telescope;_ScopeInfo

# Test SiteLocations command
SiteLocations
<press Enter>

# Expected response (example):
# 42.360081;-71.088571;43.0;_SiteLocations
```

## Automated Test Sequence

### Test 1: Connection and Status Polling
```bash
# Watch INDI logs
tail -f /tmp/indi_sitech_mount.log

# Look for:
# - "SiTech mount connected successfully via TCP"
# - "Scope: <name>, Aperture: <mm> mm, Focal Length: <mm> mm"
# - "Site Location: Lat XX.XXX°, Lon YY.YYY°, Elev ZZ.Z m"
# - Regular "TCP Response:" messages every second
```

**Expected Behavior:**
- Driver connects successfully
- Scope info and site location retrieved
- Status updates every second
- No error messages about invalid commands

### Test 2: Status Parsing
```bash
# Use indi_getprop to check properties
indi_getprop "SiTech Mount.EQUATORIAL_EOD_COORD.*"

# Should show current RA/Dec
# RA should be in hours (0-24)
# Dec should be in degrees (-90 to +90)
```

**Expected Behavior:**
- RA and Dec values update regularly
- Values match mount position
- No parsing errors in logs

### Test 3: Tracking State Detection
```bash
# Check tracking state
indi_getprop "SiTech Mount.TELESCOPE_TRACK_STATE.*"

# Start tracking in SiTechExe GUI
# Check state changes in INDI

# Stop tracking in SiTechExe GUI
# Check state changes in INDI
```

**Expected Behavior:**
- TRACK_ON should reflect actual tracking state
- State changes within 1 second (polling interval)
- boolParms bit 1 (value 2) should toggle

### Test 4: GoTo Command
```bash
# Set target coordinates (example: Polaris)
indi_setprop "SiTech Mount.EQUATORIAL_EOD_COORD.RA=2.5;DEC=89.2"

# Watch logs for GoTo command
# Expected log entry:
# "GoTo 2.5 89.2 J2K\n"
```

**Expected Behavior:**
- Mount starts slewing
- boolParms bit 2 (value 4) sets (slewing)
- Mount arrives at target
- boolParms bit 2 clears
- boolParms bit 1 sets (tracking)

### Test 5: Sync Command
```bash
# With mount pointed at a known star, sync to coordinates
indi_setprop "SiTech Mount.EQUATORIAL_EOD_COORD.RA=10.0;DEC=30.0"
indi_setprop "SiTech Mount.TELESCOPE_MOTION.SYNC=On"

# Watch logs for Sync command
# Expected log entry:
# "Sync 10.0 30.0 2 J2K\n"
```

**Expected Behavior:**
- SiTechExe accepts sync
- No "Init Window" popup (mode 2 = instant calibration)
- Mount model updated
- Future GoTo commands more accurate

### Test 6: Abort Command
```bash
# Start a long slew
indi_setprop "SiTech Mount.EQUATORIAL_EOD_COORD.RA=0.0;DEC=45.0"

# Wait 1 second then abort
sleep 1
indi_setprop "SiTech Mount.TELESCOPE_ABORT.ABORT=On"

# Watch logs for Abort command
# Expected log entry:
# "Abort\n"
```

**Expected Behavior:**
- Mount stops immediately
- boolParms bit 2 clears (no longer slewing)
- boolParms bit 1 clears (no longer tracking)
- Mount status = SCOPE_IDLE

### Test 7: Park/Unpark
```bash
# Park the mount
indi_setprop "SiTech Mount.TELESCOPE_PARK.PARK=On"

# Wait for park to complete
# Check status
indi_getprop "SiTech Mount.TELESCOPE_PARK.*"

# Unpark
indi_setprop "SiTech Mount.TELESCOPE_PARK.UNPARK=On"
```

**Expected Behavior:**
- Park: Mount slews to park position, boolParms bit 4 sets (value 16)
- Unpark: boolParms bit 4 clears, mount ready for GoTo

### Test 8: Guiding (PulseGuide)
```bash
# Test guide north for 1 second
indi_setprop "SiTech Mount.TELESCOPE_TIMED_GUIDE_NS.TIMED_GUIDE_N=1000"

# Watch logs for PulseGuide command
# Expected log entry:
# "PulseGuide 0, 1000\n"  (note the comma!)

# Test all four directions
indi_setprop "SiTech Mount.TELESCOPE_TIMED_GUIDE_NS.TIMED_GUIDE_S=500"
indi_setprop "SiTech Mount.TELESCOPE_TIMED_GUIDE_WE.TIMED_GUIDE_E=750"
indi_setprop "SiTech Mount.TELESCOPE_TIMED_GUIDE_WE.TIMED_GUIDE_W=1000"
```

**Expected Behavior:**
- Mount nudges in specified direction
- Movement duration matches milliseconds parameter
- Movement distance based on guide rate in controller config

### Test 9: Manual Motion (Slew Buttons)
```bash
# Start north motion
indi_setprop "SiTech Mount.TELESCOPE_MOTION_NS.MOTION_NORTH=On"

# Stop motion
indi_setprop "SiTech Mount.TELESCOPE_MOTION_NS.MOTION_NORTH=Off"

# Watch logs for MoveAxisPri commands
```

**Expected Behavior:**
- Mount moves while button is "pressed"
- Mount stops when button is "released"
- Speed controlled by SLEW_RATE property

### Test 10: Tracking Control
```bash
# Enable tracking
indi_setprop "SiTech Mount.TELESCOPE_TRACK_STATE.TRACK_ON=On"

# Watch logs for SetTrackMode command
# Expected log entry:
# "SetTrackMode 1 0 0.0 0.0\n"

# Disable tracking
indi_setprop "SiTech Mount.TELESCOPE_TRACK_STATE.TRACK_ON=Off"

# Expected log entry:
# "SetTrackMode 0 0 0.0 0.0\n"
```

**Expected Behavior:**
- Mount starts/stops sidereal tracking
- boolParms bit 1 toggles
- Current RA/Dec updates reflect tracking

## Error Condition Tests

### Test 11: Blinky Mode Detection
1. Put controller in manual (Blinky) mode via SiTechExe
2. Check INDI logs
3. Expected: "Servo controller is in Blinky (Manual) mode"
4. boolParms bit 6 should be set (value 64)

### Test 12: Communication Fault Detection
1. Simulate fault (disconnect servo controller cable, if possible)
2. Check INDI logs
3. Expected: "Communication fault between SiTechExe and ServoController"
4. boolParms bit 7 should be set (value 128)

## Verification Checklist

- [ ] Driver connects to SiTechExe successfully
- [ ] Status updates every second without errors
- [ ] Scope info displays correctly (aperture, focal length, name)
- [ ] Site location displays correctly (lat, lon, elevation)
- [ ] RA/Dec values are reasonable and update
- [ ] Tracking state reflects actual mount state
- [ ] GoTo slews to correct coordinates
- [ ] Sync updates mount model without popup
- [ ] Abort stops motion immediately
- [ ] Park moves to park position and sets parked flag
- [ ] Unpark clears parked flag
- [ ] Guiding works in all four directions
- [ ] Manual motion responds to slew buttons
- [ ] Tracking can be enabled/disabled
- [ ] Error conditions logged appropriately
- [ ] No "invalid command" errors in SiTechExe or INDI logs

## Debug Tips

### If connection fails:
1. Check SiTechExe is running
2. Verify TCP port is 8079 in SiTechExe config
3. Check firewall allows port 8079
4. Verify IP address/hostname is correct
5. Test with netcat: `nc localhost 8079`

### If status parsing fails:
1. Check log for "Invalid TCP response" messages
2. Verify SiTechExe version is 0.92e or later
3. Test manually with netcat to see raw responses
4. Check for extra semicolons or missing fields

### If commands don't work:
1. Check logs for exact command sent
2. Verify command format matches protocol
3. Test command manually with netcat
4. Check SiTechExe is not in Blinky mode
5. Verify mount is initialized in SiTechExe

### If coordinates are wrong:
1. Verify site location is set correctly
2. Check J2K flag is being sent
3. Confirm telescope is initialized in SiTechExe
4. Check mount model has calibration points

## Success Criteria

The protocol implementation is correct if:
1. ✅ All commands sent match SiTechTCPProtocol.txt exactly
2. ✅ All responses parsed correctly without errors
3. ✅ Mount responds to all commands appropriately
4. ✅ Status updates reflect actual mount state in real-time
5. ✅ GoTo/Sync use J2000 coordinates correctly
6. ✅ No communication timeouts or failures
7. ✅ Error conditions detected and logged

## Performance Metrics

Expected performance:
- Connection time: < 2 seconds
- Status update frequency: 1 Hz (every second)
- Command response time: < 100ms
- GoTo accuracy: Within sync error + pointing model error
- Tracking accuracy: Limited by mount, not protocol
- Guide pulse timing: ±50ms of requested duration

## Log File Locations

```bash
# INDI driver logs
/tmp/indi_sitech_mount.log

# INDI server logs
/tmp/indiserverXXXX.log

# System logs
journalctl -u indiserver
```

## Next Steps After Testing

If all tests pass:
1. Integrate with KStars/Ekos
2. Test autoguiding with PHD2
3. Perform plate solving and sync
4. Test long-duration tracking
5. Verify polar alignment assistant works
6. Test park/unpark sequences
7. Document any platform-specific issues
