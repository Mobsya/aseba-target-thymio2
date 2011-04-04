#include <i2c/i2c.h>
#include <error/error.h>
#include <clock/clock.h>

#include "mma7660.h"
#include "regulator.h"

#define XOUT		0x00
#define YOUT		0x01
#define ZOUT		0x02
#define TILT		0x03
#define SR_STATUS	0x04
#define SLEEP_CNT	0x05
#define INT_SETUP	0x06
#define MODE		0x07
#define SR		0x08
#define TAP_DETECT	0x09
#define TAP_DEBOUNCE	0x0a

// XOUT, YOUT, ZOUT bits
#define ALERT		6
#define ALERT_MASK	(1 << ALERT)
#define DATA_MASK	0b111111		// with the sign bit

// INT_SETUP bits
#define GINT		4
#define PDINT 		2	

// MODE bits
#define ON		0
#define IPP		6	// Interrupt output open-drain (0) / push-pull (1)
#define IAH		7	// Interrupt output active low (0) /  high (1)

#define MODE_CONFIG_OFF	((1 << IPP) | (1 << IAH))
#define MODE_CONFIG_ON	(MODE_CONFIG_OFF | (1 << ON))

static int i2c_bus;
static int i2c_address;
static mma7660_cb cb;
static char data[4];
static unsigned char reg;

static void mma7660_int_cb(void);

static void mma7660_i2c_cb(int i2c_id, bool status) {
	static int relaunched;
	int tap;
	// Check is the ALERT bit is set -> invalid input
	if ( (data[0] & ALERT_MASK) ||
	     (data[1] & ALERT_MASK) ||
	     (data[2] & ALERT_MASK) ||
		 (data[3] & ALERT_MASK)) {
		// Don't fire the callback
		if(!relaunched) {
			mma7660_int_cb();
			relaunched = 1;
		}
		return;
	}
	
	relaunched = 0;
	
	tap = data[3] & 0x20;

	// Fix the sign bit
	data[0] = ((signed char) (((unsigned char) data[0]) << 2)) >> 2;
	data[1] = ((signed char) (((unsigned char) data[1]) << 2)) >> 2;
	data[2] = ((signed char) (((unsigned char) data[2]) << 2)) >> 2;

	if(cb)
		cb(data[0],data[1],data[2],tap);
}

static void mma7660_int_cb(void) {
	// Safety: If i2c is busy, ignore this interrupt
	if(i2c_master_is_busy(i2c_bus))
		return;
	
	// Read XOUT, YOUT, ZOUT, status and fire the callback
	reg = XOUT;
	i2c_master_transfert_async(i2c_bus, i2c_address, &reg, 1, (unsigned char *) data, 4, mma7660_i2c_cb);
}

static void write(unsigned char reg, unsigned char data){
	unsigned char array[2];
	array[0] = reg;
	array[1] = data;
	i2c_master_transfert_block(i2c_bus, i2c_address,
		array, 2, NULL, 0);
}

void mma7660_init(int i2c, unsigned char address, mma7660_cb ucb, int prio) {
	i2c_bus = i2c;
	i2c_address = address;
	cb = ucb;
	
	va_get(); // Enable Va LDO
	
	clock_delay_us(1300);
	
	/* Configure device */
	write(MODE, MODE_CONFIG_OFF);		// Reset
	write(INT_SETUP, 1 << GINT);		// Enable auto interrupt on update (GINT)

	/* Configure PIC */
	TRISDbits.TRISD7 = 1;			// Set RD7 pin as input
//	CNPU2bits.CN16PUE = 1;			// Enable internal pull-up resistor

	/* Configure interrupts */
	IPC4bits.CNIP = prio;			// CN interrupt priority
	IFS1bits.CNIF = 0;			// Clear flag
	CNEN2bits.CN16IE = 1;			// Enable CN16 interrupt
	IEC1bits.CNIE = 1;			// Enable CN interrupt
}

void mma7660_set_mode(int hz, int tap_en) {
	ERROR_CHECK_RANGE(hz, MMA7660_120HZ, MMA7660_0HZ, MMA7660_ERROR_INVALID_PARAM);

	IEC1bits.CNIE = 0;

	while(i2c_master_is_busy(i2c_bus));

	// Set the device into standby mode
	write(MODE, MODE_CONFIG_OFF);

	if(tap_en && hz != MMA7660_120HZ)
		ERROR(MMA7660_ERROR_INVALID_PARAM, &hz);

	// Change
	if (hz == MMA7660_0HZ)
		// Stop the device
		return;
	else
		// Set the frequency
		write(SR, hz);
		
	// Write the Tap detection register
	// All axis, treshold: 12 counts
	if(tap_en)
		write(TAP_DETECT, 11);
	else
		write(TAP_DETECT,  0xE0);
	
	//write the tap debounce register
	// 12 counts debouce
	if(tap_en) 
		write(TAP_DEBOUNCE, 11);
	else
		write(TAP_DEBOUNCE, 0);
/*		
	if(tap_en)
		write(INT_SETUP, (1 << GINT));
	else
		write(INT_SETUP, (1 << GINT));
	*/

	// Enable the device
	write(MODE, MODE_CONFIG_ON);
	
	IEC1bits.CNIE = 1;
}


void mma7660_suspend(void) {
	IEC1bits.CNIE = 0;
	
	// Disable the device
	write(MODE, MODE_CONFIG_OFF);
	
	va_put(); // disable LDO
}

void _ISR _CNInterrupt(void)
{
	// OK
	IFS1bits.CNIF = 0;
	
	
	// One interrupt for all CN pins!

	// Check the state of the pin
	if (PORTDbits.RD7 != 1)
		return;

	// Initiate the data transfer
	mma7660_int_cb();

}

