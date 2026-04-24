//
// Created by Hyacinthe Chemasle on 24/04/2026.
//
#include "XPLMDefs.h"
#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include <stdio.h>
#include "XPLMDisplay.h"
#include <string.h>
#include "XPLMUtilities.h"


/*
 * State machine
 *
 */
typedef enum {
    STATE_AIRBORNE, //Flying and waiting for touchdown
    STATE_TOUCHDOWN, //just landed, shows result
    STATE_ON_GROUND //taxiing, waiting for takeoff
} PluginState;

static PluginState g_state = STATE_AIRBORNE; // default state is airborne

/*
 * Dataref handles
 *
 */

/* Reference actual VS from simulator*/
static XPLMDataRef vertical_speed_ref = NULL;
/* Checks if landing gear is on the ground*/
static XPLMDataRef on_ground_ref = NULL;
/* Reference actual airspeed from simulator*/

static XPLMDataRef airspeed_ref = NULL;
static XPLMDataRef gforce_ref = NULL;


/* Vertical speed at touchdown*/
static float touchdown_vertical_speed = 0.0f;
/* G force captured at exact moment of touchdown */
static float touchdown_g_force = 0.0f;
/* Counts down from 10 to 0 to keep displayed on screen*/

static float display_countdown = 0.0f;

/* How many seconds to show result after landing*/
#define RESULT_DISPLAY_DURATION 10.0f

/* Window to display landing result */

static XPLMWindowID result_window = NULL;


/* Decides conditionally what landing rate is based on detected fpm */
static const char *get_landing_result(float fpm) {
    if (fpm > -60){ return "Excellent Landing"; }
    if (fpm > -180){ return "Great Landing"; }
    if (fpm > -300){ return "Good Landing";}
    if (fpm > -500){ return "Hard Landing";}
    if (fpm > -600){ return "Unacceptable Landing";}
}

static void get_result_colour(float fpm, float *red, float *green, float *blue) {
   /*Purple*/if (fpm > -60){ *red = 0.8f; *green=0.4f; *blue=1.0f; return;}
   /*Green*/ if (fpm > -180){*red=0.0f; *green=1.0f; *blue=0.4f; return;   }
   /*Yellow*/if (fpm > -300){*red=1.0f; *green=1.0f; *blue=0.0f; return; }
    /*Orange*/if (fpm > -500){*red=1.0f; *green=0.5f; *blue=0.0f; return;  }
   /* Red */ if (fpm > -600){*red=1.0f; *green=0.1f; *blue=0.1f; return; }
}

/* forward declarations (needed because flight_loop and draw_window are */
static float flight_loop_callback(float, float, int, void *);
static void draw_result_window(XPLMWindowID, void *);

/*
 * XPLUGINSTART
 * Called once by X-Plane when plugin loads
 * This is where we find datarefs and create window
 * Source: SDK/CHeaders/XPLM/XPLMPLugin.h
 */

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    // Identify plugin
    strcpy(outName, "Landing Rate Monitor");
    strcpy(outSig, "com.hyacinthechemasle.landingrate");
    strcpy(outDesc, "Shows your landing rate at touchdown");


    /*
     * Crucial part:
     * find all datarefs by their string name
     * XPLMFindDataRef returns a handle, not a value
     * we call it once here and store handle for later use
     * Source: SDK/CHeaders/XPLM/XPLMDataAccess.h - XPLMFindDataRef()
     */

    vertical_speed_ref = XPLMFindDataRef("sim/flightmodel/position/vh_ind_fpm");
    on_ground_ref = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
    airspeed_ref = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed");
    gforce_ref = XPLMFindDataRef("sim/flightmodel/forces/g_nrml");

    if (!vertical_speed_ref || !on_ground_ref || !airspeed_ref || !gforce_ref) {
        /*XPLM Debug string write to the X-Plane 12 log.txt*/
        XPLMDebugString("Landing Rate Monitor Failed to Load");
        return 0; //on failure
    }

    /* Otherwise create the window for success, however we want to keep this hidden at first since we only want window
     * to pop up when a landing is detected
     *
     */
    XPLMCreateWindow_t window_params;
    memset(&window_params, 0, sizeof(window_params));
    window_params.structSize = sizeof(window_params);
    window_params.left = 100; // pixels from left
    window_params.top = 600; // pixels from top
    window_params.right = 400; // pixels from right
    window_params.bottom = 450; // pixels from bottom
    window_params.visible = 0; // hidden at startup
    window_params.drawWindowFunc = draw_result_window;
    window_params.handleMouseClickFunc     = NULL;
    window_params.handleKeyFunc            = NULL;
    window_params.handleCursorFunc         = NULL;
    window_params.handleMouseWheelFunc     = NULL;
    window_params.refcon                   = NULL;
    window_params.layer = xplm_WindowLayerFloatingWindows;
    window_params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

    result_window = XPLMCreateWindowEx(&window_params);
    XPLMSetWindowPositioningMode(result_window, xplm_WindowPositionFree, -1);
    XPLMSetWindowTitle(result_window, "Landing Rate");

    XPLMDebugString("LandingRate: loaded successfully\n");
    return 1; //indicate success
}

/*
 * XPLUGINENABLE
 * call when plugin is enabled
 * this is where we can register our flight loop
 * Source: SDK/CHeaders/XPLM/XPLMPlugin.h
 */

PLUGIN_API int XPluginEnable(void) {
    /* Register flight loop to run every 0.1 seconds (100ms) */
    XPLMRegisterFlightLoopCallback(flight_loop_callback, 0.1f, NULL);
    return 1;
}

/* Called when plugin is disabled or X-Plane quits, important to clean up things you created */
PLUGIN_API void XPluginDisable(void) {
    XPLMUnregisterFlightLoopCallback(flight_loop_callback, NULL);
}

PLUGIN_API void XPluginStop() {
    // Destroy window to free memory
    if (result_window) {
        XPLMDestroyWindow(result_window);
        result_window = NULL;
    }
}

/* Required by SDK but don't for plugin*/

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from_plugin, int message, void *param){}

