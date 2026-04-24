# landing-monitor-plugin
Plugin for X-Plane 12 Flight simulator that computes aircraft landing performance with visual feedback to user


## X-Plane 12 SDK

This program utilises the SDK to utilise Datarefs to extract real-time data of indicated vertical speed, air-speed, g-force. This ensures that we can use these variables to calculate fpm and compute a landing performance metric

### Dataref requirements from X-Plane SDK
1. onground_any ( detects if wheels are on ground )

2. vh_ind_fpm ( detects feet per minute )

3. indicated_airspeed ( make sure you've actually landed )

4. tire_vertical_force_n_mtr ( gear compression more precise touchdown detection

all from sim/flightmodel
