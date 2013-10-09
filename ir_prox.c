/*
        Thymio-II Firmware

        Copyright (C) 2013 Philippe Retornaz <philippe dot retornaz at epfl dot ch>,
		Copyright (C) 2013 Josep Soldevila Vilarrasa
        Mobots group (http://mobots.epfl.ch), Robotics system laboratory (http://lsro.epfl.ch)
        EPFL Ecole polytechnique federale de Lausanne (http://www.epfl.ch)

        See authors.txt for more details about other contributors.

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published
        by the Free Software Foundation, version 3 of the License.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <types/types.h>

#include "ir_prox.h"
#include "regulator.h"
#include "sound.h"
#include "sensors.h"

#include <skel-usb.h>

#define COMM_GAIN		28  // TODO: Optimize
#define COMM_OFFSET		(7000) // TODO: Optimize

#define COMM_MAX		((0xFFFF - COMM_OFFSET - 960 - 4) / COMM_GAIN)

#define N_SENSORS 7
#define CALIB_HIST 30
#define DEFAULT_CALIB 0x7FFF

	
static unsigned char update_calib;
static unsigned char prox_calib_counter[N_SENSORS];
static unsigned char __attribute((near)) pulse_count; // used in interrupt processing. needs to be "near"
static unsigned char __attribute((near)) last_tx;   // used in interrupt processing. needs to be "near"
static unsigned char enable_network;				  // 1 mean will be enabled, 2 mean enabled. 0 == disabled (default)
static unsigned int loopback_delay[N_SENSORS];

// I don't want gcc to optimise too much, force noinline
static int __attribute((noinline)) ic_bufne(int ic) {
	IC1CON1BITS * ic_ptr =(IC1CON1BITS *) &IC1CON1bits + 4*ic; // Memory map: con1, con2, buf, tmr
	return ic_ptr->ICBNE;
}

static unsigned int __attribute((noinline)) ic_buf(int ic) {
	return *((&IC1BUF) + ic*4);
}

static int _calib(int value, int i) {
	int ret;

	if(value - CALIB_HIST < settings.prox_min[i]) {
		if(++prox_calib_counter[i] > 3) {
			if(value < settings.prox_min[i]) {
				settings.prox_min[i] = value;
				update_calib = 1;
			} else
				prox_calib_counter[i] = 0;
		}
	} else
		prox_calib_counter[i] = 0;
	// Cast to unsigned so the compiler optimise by a shift
	// We checked that the value was positive, so it's safe.

	if (settings.prox_min[i] == DEFAULT_CALIB)
		ret = value;
	else
		ret = value - (3*((unsigned int) settings.prox_min[i])) / 4 + 800;

	if(ret < 0)
		ret = 0;

	return ret;
}

static int perform_calib(unsigned int raw, int i) {
	int value;
	if(raw > 32767)
		return 0; // Sanity check
	
	value = raw;

	if(settings.prox_min[i] > 0) {
			// On the fly recalibration
			return _calib(value,i);
	} else {
			// Calibration disabled
			return value;
	}
}

void __attribute((interrupt, no_auto_psv)) _OC8Interrupt(void) {
// The compiler is too dumb to optimize thoses registers access....
	asm volatile (
	"bclr 0x89, #4					\n"		// _OC8IF = 0;
	"dec.b %[cnt]					\n"		// if (--cnt == 0) {
	"bra nz, 1f						\n"
	"bclr 0x105, #7					\n"		//		T1CONbits.TON = 0;
	"mov #0xFFF8, w0				\n"
	"and 0x1CC						\n"		//		OC7CON1bits.OCM = 0;
	"and 0x1D6						\n"		//		OC8CON1bits.OCM = 0;
	"inc.b %[sec]					\n"		//		sec++;  // set to on, as it should be init. at 0
	"1:								\n"		// }
	"rcall _sound_ignore_next_sample\n"		// sound_ignore_next_sample(); // This function only modify w0 and a memory location
	: [cnt] "+U" (pulse_count), [sec] "+U" (last_tx) : : "cc","memory","w0");
}

static void ir_tx(int value) {
	// Timer 1 manage the time between the two pulses
	TMR1 = 0;
	if(value >= 0) {
		unsigned int _v = value;
		if(_v > COMM_MAX)
			_v = COMM_MAX;
		// minus 3, because we have a constant offset between the OC enable and the Timer enable.
		PR1 = COMM_OFFSET - 3 + COMM_GAIN * _v;
		pulse_count = 2;
	} else {
		PR1 = 0xFFFF; // Large enough to never trigger before the OC interrupt
		pulse_count = 1;
	}

	last_tx = 0;

	// Reset the IC TMR and hold in reset until corresponding OC pulse
	IC1CON2bits.TRIGSTAT = 0;
	IC2CON2bits.TRIGSTAT = 0;
	IC3CON2bits.TRIGSTAT = 0;
	IC4CON2bits.TRIGSTAT = 0;
	IC5CON2bits.TRIGSTAT = 0;
	IC6CON2bits.TRIGSTAT = 0;
	IC7CON2bits.TRIGSTAT = 0;
	
	// The order (OC7 then OC8) is important as I prefer to have OC7
	// a little bit (1 cycle) before OC8 for the first pulse
	// instead of the opposite (leading to 1 cycle of front & back switched on)
	// For the second pulse it's not important as TIMER1 is synchronizing both

	// The next assembly statement is critical
	// We are at IPL6 (highest currently in use) thus no need to protect against
	// others interrupts.
	// We should start the OC as close as possible to the timer.

	asm volatile(
	"mov #0xFFF8, w0	\n"
	"mov 0x1CC, w1		\n"
	"and w1, w0, w0		\n"
	"ior #0x5, w0		\n"
	"mov w0, 0x1CC		\n"		// OC7CON1bits.OCM = 5;
	"mov w0, 0x1D6		\n"		// OC8CON1bits.OCM = 5; (overwrite OC8CON1 to match OC7)
	"bset 0x105, #7		\n"		// T1CONbits.TON = 1;
	: : : "cc", "memory", "w0", "w1");

	

}

static unsigned int edge_ic_time[N_SENSORS * 4];
static unsigned int edge_uc_time[N_SENSORS * 4];
static unsigned char edge[N_SENSORS];

static void ir_prox_rx_reset(void) {
	int i;
	for(i = 0; i < N_SENSORS; i++) {
		while(ic_bufne(i))
			ic_buf(i);
		edge[i] = 0;
	}
}


static unsigned int do_rx(int i, unsigned int time) {
	int ret = 0;
	if(ic_bufne(i)) {
		edge_ic_time[i + N_SENSORS * edge[i]] = ic_buf(i);
		edge_uc_time[i + N_SENSORS * edge[i]] = time;
		edge[i]++;
		// Do we have 2 complete pulses in the buffer ?
		if(edge[i] == 4) {
			// Do we have more than 36*125us between the first edge and the last one ?
			if(edge_uc_time[i + N_SENSORS * 3] - edge_uc_time[i] > 36) {
				// remove the first edge, and shift the others ones
				unsigned int k;
				for(k = 0; k < 3; k++) {
					edge_ic_time[i + N_SENSORS * k] = edge_ic_time[i + N_SENSORS * (k+1)];
					edge_uc_time[i + N_SENSORS * k] = edge_uc_time[i + N_SENSORS * (k+1)];
				}
				edge[i] = 3;
			} else {
				unsigned int symbol_lead;
				unsigned int symbol_trail;
				unsigned int intensity =  edge_ic_time[i + N_SENSORS] - edge_ic_time[i];
				symbol_lead = edge_ic_time[i + N_SENSORS*2] - edge_ic_time[i];
				symbol_trail = edge_ic_time[i + N_SENSORS*3] - edge_ic_time[i+  N_SENSORS];
				 // This force the same light intensity on both pulse
				if(abs(symbol_trail - symbol_lead) < 20) {
					// This force to have a good enough SNR. Weak signal have much much more noise (rising edge is slow)
					// And remove too big pulses such as two robot sending at the same times
					if(intensity >= 1400 && intensity < COMM_OFFSET) {
						// Check that the decoded input is really close to a symbol
						unsigned int temp =  ((int) ((symbol_lead - COMM_OFFSET + COMM_GAIN/2) / COMM_GAIN));
						unsigned int temp2 = temp * COMM_GAIN + COMM_OFFSET;
						// This is the most strict filter when the robot is moving
						if(abs(symbol_lead - temp2) < 3) {
							// Ok, valid.
							vmVariables.intensity[i] = intensity;
							vmVariables.sensor_data[i] = temp;
							ret = 1;
						}
					}
				}
				edge[i] = 0;
			}
		}
	}
	return ret;
}

static int ir_prox_rx(unsigned int time) {
	static unsigned int rx_pending;
	static unsigned char hold_off;

	int i;
	int ret = 0;
	unsigned int active = 0;

	for(i = 0; i < N_SENSORS; i++)
		active |= do_rx(i, time) << i;

	if(active) {
		hold_off = 2; // we might have 500 us between the end of pulse on one sensor and on another one...
		if(time > 720)
			ret = -2; // we should slow down a bit ...
		if(time < 80)
			ret = 2;  // we should hurry up a bit ...
	} else if(hold_off)
		hold_off--;
	
	rx_pending |= active;

	if(hold_off == 0 && rx_pending != 0) {
		// We got something previously, and all the sensors are now idle.
		int max;
#define FF1R(word, pos) asm("ff1r %[w], %[b]" : [b] "=x" (pos) : [w] "r" (word) : "cc")
		
		FF1R(rx_pending, max);
		max--; // rx_pending is != 0, thus we MUST have a 1 in it.

		// Look for the strongest signal
		for(i = max; i < N_SENSORS; i++)
			if((vmVariables.intensity[max] < vmVariables.intensity[i]) && (rx_pending & (1 << i)))
				max = i;

		vmVariables.rx_data = vmVariables.sensor_data[max];

		SET_EVENT(EVENT_DATA);

		rx_pending = 0;
	} 
	
	return 0;
}

static int ir_prox_rx_oa(void) {
	int temp[2];
	int i;
	int ret = 0;
	for(i = 0; i < N_SENSORS; i++) {
		vmVariables.prox[i] = 0;
		loopback_delay[i] = 0;
		if(ic_bufne(i)) {
			temp[0] = ic_buf(i);
			if(ic_bufne(i)) {
				temp[1] = ic_buf(i);
				loopback_delay[i] = temp[0];
				// Validity check
				if(temp[0] < 2000) {
					if(temp[0] > 100)
						vmVariables.prox[i] = perform_calib(temp[1] - temp[0],i);
					else if(!sensors_prox_drift()) {
						ret = ((temp[0] & 0x3) - 2) * 5; // Should use random here ....
					}
				} else if(!sensors_prox_drift()) {
					ret = ((temp[0] & 0x3) - 1) * 5; // Should use random here ....
				}
			} 
		}
		// Reset the calibration counter if the sensor did not see something:
		// => perform_calib was not trigged, thus, prox_calib_counter was not updated
		// So, do it here.
		if(!vmVariables.prox[i])
			prox_calib_counter[i] = 0;
	}
	return ret;
}

static int ir_prox_check_2nd_pulse(void) {
	unsigned int temp;
	int i;
	int ret = 0;
	unsigned int period = PR1;
	for(i = 0; i < N_SENSORS; i++) {
		if(ic_bufne(i)) {
			temp = ic_buf(i);
			
			if((loopback_delay[i] + period - COMM_GAIN < temp) && !sensors_prox_drift()) {
				ret =  ((temp & 0x3) - 2) * 5; // Should use random here ....
			}
			if((loopback_delay[i] + period + COMM_GAIN > temp) && !sensors_prox_drift()) {
				ret =  ((temp & 0x3) - 1) * 5; // Should use random here ....
			}
		}
	}
	return ret;
}

int ir_prox_tick(unsigned int time) {
	int ret = 0;
	switch(time) {
		case 0:
			ir_prox_rx_reset();

			if(enable_network) {
				enable_network = 2;
				ir_tx(vmVariables.ir_tx_data);
			} else
				ir_tx(-1);
			break;
		case 5: // First pulse should be emitted by now. (max pulse: 8000, emition time: 960, back is delayed by 960: 9920 => 5.
			ret = ir_prox_rx_oa();
			break;
		case 6 ... 0xFFFF: 
			if(enable_network == 2 && last_tx) {
				if(last_tx == 6) {
					ret = ir_prox_check_2nd_pulse(); // If we just sent the 2nd pulse, check if there is contention.
					ir_prox_rx_reset();
					last_tx++;
				} else if(last_tx > 6) {
					// We sent the two pulses, now start to listen
					ret = ir_prox_rx(time);
				} else
					// Wait the local echo
					last_tx++;
			}
			break;
	}
	if(enable_network != 2)
		ret = 0;
	return ret;
}

void prox_init(int priority) {
    int i;
	va_get();

	// brand-new robots will have a settings to 0
	// Thus put the maximum minimum value in order to start the calibration
	// from a valid point.
	for(i = 0; i < N_SENSORS; i++)
		if(settings.prox_min[i] == 0) {
			settings.prox_min[i] = DEFAULT_CALIB;
		}

	
	// IC configuration
	IC1CON2bits.ICTRIG = 1; // start counting when OC pulse
	IC1CON2bits.SYNCSEL = 7; // OC7 trigger the IC
	IC1CON1bits.ICSIDL = 0;

	IC1CON1bits.ICTSEL = 7; // Fcy as clock source

	IC1CON1bits.ICM = 0x1; // capture rising and falling ...
	
	IC2CON2 = IC1CON2; // IR2
	IC2CON1 = IC1CON1; 
	
	IC3CON2 = IC1CON2; // IR3
	IC3CON1 = IC1CON1;
	
	IC4CON2 = IC1CON2; // IR4
	IC4CON1 = IC1CON1;
	
	IC5CON2 = IC1CON2; // IR5
	IC5CON1 = IC1CON1;

	IC6CON2 = IC1CON2; // Back IR 6
	IC6CON2bits.SYNCSEL = 8; // OC8 trigger the IC

	IC6CON1 = IC1CON1;

	IC7CON2 = IC6CON2; // Back IR 7
	IC7CON1 = IC6CON1;
	
	
	
	// OC7 is used to generate the front pulse
	OC7CON1 = 0;
	OC7CON2 = 0;
	OC7CON1bits.OCTSEL = 0x7; // CPU clock
	OC7CON2bits.SYNCSEL = 0xB; // Sync with timer 1
	OC7R = 4;
	OC7RS = OC7R + 960; // 60us

	// OC8 the back pulse
	OC8CON1 = OC7CON1;
	OC8CON2 = OC7CON2;
	OC8R = OC7RS; // Do not pulse back & front at the same times.
	OC8RS = OC8R + 960;

	_OC8IP = priority; // OC8 interrupt is used to stop generating TX pulses
	_OC8IF = 0;
	_OC8IE = 1;

}

void prox_poweroff(void) {

    
	OC7CON1 = 0;
	OC8CON1 = 0;
	IC1CON1 = 0;
	IC2CON1 = 0;
	IC3CON1 = 0;
	IC4CON1 = 0;
	IC5CON1 = 0;
	IC6CON1 = 0;
	IC7CON1 = 0;

	_OC8IE = 0;

	T1CONbits.TON = 0; 
	
	va_put();
        
	// if calibration is new, and Vbat > 3.3V, then flash
	if(update_calib && vmVariables.vbat[0] > 655) {
		AsebaNative__system_settings_flash(NULL);
	}
}

void prox_enable_network(void) {
	if(!enable_network)
		enable_network = 1; // will be fully enabled by the interrupt
}

void prox_disable_network(void) {
	enable_network = 0;
}

