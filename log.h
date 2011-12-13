#ifndef _LOG_H_
#define _LOG_H_

// To be called at 1Khz when in "on" mode
void log_poweron_tick(void);

// To be called by the poweroff code
void log_poweroff_tick(void);

void log_prepare_reset(void);

void log_init(void);

#endif
