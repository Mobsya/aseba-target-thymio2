/*
        Thymio-II Firmware

        Copyright (C) 2013 Philippe Retornaz <philippe dot retornaz at epfl dot ch>,
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

#ifndef _SKEL_H_
#define _SKEL_H_

// Aseba Includes
#include <vm/natives.h>
#include <vm/vm.h>

#include <common/types.h>

#include <skel-usb-user.h>


extern struct _vmVariables vmVariables;
extern unsigned int events_flags[2];
extern AsebaVMState vmState;

/*** In your code, put "SET_EVENT(EVENT_NUMBER)" when you want to trigger an
	 event. This macro is interrupt-safe, you can call it anywhere you want.
***/
#define _SET_EVENT_LOW(event) atomic_or(&events_flags[0], 1 << (event))
#define _SET_EVENT_HIGH(event) atomic_or(&events_flags[1], 1 << (event-16))

#define SET_EVENT(event) do { unsigned int _ = event; if (_ < 16) _SET_EVENT_LOW(_); else _SET_EVENT_HIGH(_); } while(0)

#define _CLEAR_EVENT_LOW(event) atomic_and(&events_flags[0], ~(1 << (event)))
#define _CLEAR_EVENT_HIGH(event) atomic_and(&events_flags[1], ~(1 << (event-16)))
#define CLEAR_EVENT(event) do { unsigned int _ = event; if (_ < 16) _CLEAR_EVENT_LOW(_); else _CLEAR_EVENT_HIGH(_); } while(0)

#define _IS_EVENT_HIGH(event) (events_flags[1] & (1 << (event - 16)))
#define _IS_EVENT_LOW(event) (events_flags[0] & (1 << (event)))

#define IS_EVENT(event) (event < 16 ? _IS_EVENT_LOW(event) : _IS_EVENT_HIGH(event))

// Might be usefull if the firmware want to automatically update the settings
// USE WITH CAUTION !
void AsebaNative__system_settings_flash(AsebaVMState *vm);

// Call this when everything is initialised and you are ready to give full control to the VM
void __attribute((noreturn)) run_aseba_main_loop(void);

// Call this to init aseba. Beware, this will:
// 1. init the USB CDC module
// 2. init the RF module
// 2. clear vmVariables.
// 3. Load any bytecode in flash if present
// return 0 if not loaded, 1 if code was loaded from flash
int init_aseba_and_fifo(void);

// This function must update the variable to match the microcontroller state
// It is called _BEFORE_ running the VM, so it's a {Microcontroller state} -> {Aseba Variable}
// synchronisation
// Implement it yourself
void update_aseba_variables_read(void);

// This function must update the microcontrolleur state to match the variables
// It is called _AFTER_ running the VM, so it's a {Aseba Variables} -> {Microcontroller state}
// synchronisation
//Implement it yourself
void update_aseba_variables_write(void);

// This function load the settings structure from flash. Call it _AFTER_ init_aseba_and_can()
// return 0 if the settings were loaded
// return non-zero if the settings were NOT found (settings is non-initilised)
int load_settings_from_flash(void);

// Loads some persistent informations specific to thymio 2
// This are never used by the device itself but are used by applications using the device.
// It includes a unique uuid ( initialized on a computer ) as well as a device name that user can modify
int load_thymio_device_info_from_flash(void);

// This function should switch off everything, absolutly NO interrupt should be generated
// After the call to this function. The perif. should be in a kind of power-down mode.
void switch_off(void);

// This function prepare the microcontroller to enter in deep sleep.
// Return true if successfull (the microcontroller will enter sleep mode)
// Return false otherwise (The microcontroller will abort the sleep process)
// Implement it yourself
int prepare_sleep(void);

// This function is called immediatly after the microcontroller is woken up.
// After the execution of this function, the microcontroller must function exactly
// As before the prepare_sleep() function has been called.
// Implemnt it yourself
void resume_sleep(void);

extern struct private_settings settings;
extern struct thymio_device_info thymio_info;


//This function save internal setting if there where update.  It is called at poweroff. 
void save_settings(void);

//This function is called to validate the need to save changes in settings.
void set_save_settings(void);

#endif
