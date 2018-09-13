# Compiling Thymio firmware

## Source code

To compile the firmware from git, you need the following source code:

- [Aseba](http://github.com/mobsya/aseba), branch master
- [aseba-target-thymio2](https://github.com/aseba-community/aseba-target-thymio2) , branch master and its submodules

## Tools 

You need the MPLAB X environment and C compiler:
- IDE: [MPLAB X](https://www.microchip.com/pagehandler/en-us/family/mplabx/)
- Compiler: [MPLAB XC16](https://www.microchip.com/pagehandler/en_us/devtools/mplabxc/)
- Legacy Peripheral Libraries:[PIC24/dsPIC® DSCs](https://www.microchip.com/pagehandler/en_us/devtools/mplabxc/)

## Compilation

You can compile using two methods, one using CMake (only compiling) or used the Microchip MPLAB X tool

### CMake

Follow [compilation instructions](cmake.md)

### Use MPLAB X

First, you need to start MPLAB X and create a new project: 

- Microcontroller: PIC24FJ128GB106
- Compiler: XC16
- Hardware tool: None (simulator, or whatever). 

Use the linker script provided in the aseba-target-thymio2 git tree.

Use the following sources:
- from aseba-target-thymio2 git tree, almost everything except *skel-usb-user.c/h*:
   
    *abo.c, analog.c, behavior.c, button.c, crc.c, entry.s, ground_ir.c, ir_prox.c, leds.c, leds_low.s, log.c, main.c, mma7660.c, mode.c, motor.c, ntc.c, pid_motor.c, playback.c, pmp.c, pwm_motor.c, rc5.c, regulator.c, rf.c, sd/ff.c, sd/mmc.c, sd.c, sensors.c, skel-usb.c, sound.c, test_mode.c, thymio-buffer.c, thymio_natives.c, tone.c, usb_uart.c, wav.c*
- from **usb_pic24** folder:

    *usb_descriptors.c, usb_device.c, usb_function_cdc.c*
- from aseba git tree:

    *vm/natives.c, vm/vm.c, transport/buffer/vm-buffer.c*
- from molole git tree:

    *clock/clock.c, error/error.c, flash/flash.c, i2c/i2c.c, i2c/master.c, i2c/master_protocol.c, timer/timer.c*

Note that if you want to edit the firmware, you should add the following two files from the aseba-target-thymio2 git tree as "Important files": *skel-usb-user.c, skel-usb-user.h*. You should not compile them directly, as they are included from *skel-usb.c/h*. The reason is that *skel-usb.c/h* is target-independent code, while *skel-usb-user.c/h* is target-dependent code.

Project setup:
- Define common macros: *ASEBA_LIMITED_MESSAGE_SIZE;ASEBA_ASSERT*
- Compiler options: * -Os -mlarge-code -fomit-frame-pointer -fno-strict-aliasing*
- On, linux add: *-fno-peephole2*, because of a compiler bug (official workaround from microchip).
- Add molole, usb_pic24, aseba git tree, and aseba-target-thymio2 as include search path.

Here are screenshots of the detailed options you should set in the project properties:

![](/Capture-XC16.PNG)
![](/Capture-XC16-as.PNG)
![](/Capture-XC16-gcc.PNG)
![](/Capture-XC16-ld.PNG)


Then you should be able to compile the firmware. To upload it to the robot, use the [Thymio firmware upgrader](https://www.thymio.org/en:thymioupdate) and not a program such as icd3 or pickit3.
