#ifndef _LOG_H_
#define _LOG_H_

// To be called at 1Khz when in "on" mode
void log_poweron_tick(void);

// To be called by the poweroff code
void log_poweroff_tick(void);

void log_prepare_reset(void);

void log_dump(void * _f);

void log_init(void);

#define LOG_FLAG_INTERNAL		(0) // always not set
#define LOG_FLAG_BATTERY 		(1)
#define LOG_FLAG_ASEBABUG 		(2)
#define LOG_FLAG_VMCODESD		(3)
#define LOG_FLAG_PLAYBACKSD		(4)
#define LOG_FLAG_FLASHVMCODE	(5)
#define LOG_FLAG_RECORDSD		(6)
#define LOG_FLAG_MOTORUSED		(7)
#define LOG_FLAG_IRUSED			(8)
#define LOG_FLAG_NTCUSED		(9)
#define LOG_FLAG_SOUND			(10)
#define LOG_FLAG_LEDIR			(11)
#define LOG_FLAG_LEDRGB			(12)
#define LOG_FLAG_LEDCIRCLE		(13)
#define LOG_FLAG_ACCUSED		(14)
#define LOG_FLAG_BUTTONUSED		(15)
#define LOG_FLAG_SOUNDTRESH		(16)
#define LOG_FLAG_RC5USED		(17)
#define LOG_FLAG_LEDBUTTON		(18)
#define LOG_FLAG_LEDOTHER		(19)

void log_set_flag(unsigned char flag);

#endif
