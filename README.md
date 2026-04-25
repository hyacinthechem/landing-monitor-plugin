# Landing Monitor Plugin
Plugin for X-Plane 12 Flight simulator that computes aircraft landing performance. This is effective for any type of aircraft such as General Aviation and Commercial airliners. This plugin helps keep a log of all past Landing reports within a log file to help gauge pilot landing performance and potential improvements for approach method and technical skills such as flaring

![landingUnited.jpg](assets/landingUnited.jpg)
## X-Plane 12 SDK

![x-plane-developer-logo-web.png](assets/x-plane-developer-logo-web.png)

This program utilises the SDK to implement Datarefs to extract real-time data such as indicated vertical speed, air-speed, g-force. This ensures that we can use these variables to calculate fpm and compute a landing performance metric

### Dataref requirements from X-Plane SDK
1. onground_any ( detects if wheels are on ground )

2. vh_ind_fpm ( detects feet per minute )

3. indicated_airspeed ( gives us aircraft airspeed )

4. tire_vertical_force_n_mtr ( gear compression more precise touchdown detection

All found from sim/flightmodel


### Setup

This project requires the X-Plane SDK headers

1. Download the SDK from developer.x-plane.com/sdk
2. Extract it into the root of this project as '\SDK'
3. Build with CMake
