#ifndef _RC5_H
#define _RC5_H


typedef void (*rc5_cb)(unsigned int address, unsigned int command);

enum rc5_errors
{
	RC5_ERROR_BASE = 0x1300,
	RC5_ERROR_IMPOSSIBLE_STATE,			/**< Two RC5 cycles have elapsed while the previous bit was a 0. */
};

void rc5_init(unsigned int timer, rc5_cb ucb, int priority);
void rc5_shutdown(void);

void rc5_disable(void);
void rc5_enable(void);

extern unsigned char rc5_valid_flag;

#endif // _RC5_H

