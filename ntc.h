#ifndef _NTC_H_
#define _NTC_H_


typedef void (*ntc_cb)(int temp);

void ntc_init(ntc_cb mes_done, int prio);


void ntc_mesure(void);

void ntc_shutdown(void);


#endif
