#include <timer/timer.h>

#include "behavior.h"
#include <skel-usb.h>


// Macro to do a when-like as in aseba
// We put all the state into a section so we can
// Zero it on reinit.

#define when(cond) if({(static unsigned char attribute((section("when"))) p; \
						unsigned int c = !!(cond); \
						unsigned int r = 0; \
						if(c && !p) \
							r = p = 1; \
						r; )})


static int timer;
static unsigned int behavior;

#define ENABLED(b) ({behavior & b;})
#define ENABLE(b) do {behavior |= b;} while(0)
#define DISABLE(b) do {behavior &= ~b;} while(0)

static void timer_cb(int timer_id) {

}

void behavior_init(int t, int prio) {
	timer = t;
	timer_init(timer, 100, 3);
	timer_enable_interrupt(timer,timer_cb, prio);
}

void behavior_start(int b) {
	ENABLE(b);

	if(behavior)
		timer_enable(timer);
}

void behavior_stop(int b) {
	
	DISABLE(b);
	
	if(!behavior) {
		// TODO: Zero the when section
		timer_disable(timer);
	}
}
