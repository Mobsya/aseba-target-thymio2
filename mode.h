#ifndef _MODE_H_
#define _MODE_H_

void mode_init(void);

// Mode is in fact a behavior, but a special one 
// Which manage other behavior. So it has it's own tick 
void mode_tick(void);


#endif
