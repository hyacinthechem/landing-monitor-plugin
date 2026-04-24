//
// Created by Hyacinthe Chemasle on 24/04/2026.
//
#include "XPLMDefs.h"
#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include <stdio.h>
#include "XPLMDisplay.h"


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

/* Create flight loop call back which x plane uses*/
static float flight_loop_callback(float seconds_since_last_call, float seconds_since_last_loop, int counter, void *refcon){}