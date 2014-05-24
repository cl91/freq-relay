freq-relay
==========

A low-cost frequency relay implemented in a soft-core processor with FreeRTOS. Written in C.

What it does?
=============

The low-cost frequency relay aims to dynamical and intelligently regulate household electricity usage, thus saving energy and minimizing power network instability. 

Directory Structure
===================

The entire workspace is in the `./workspace` directory, which on my system is in the root of `H:` drive.

The sopinfo file is in `./workspace/hardware_configuration`.

The projects (both the bsp and the actual project) are in `./workspace/hardware_configuration/software`.


Main Logic
==========

The main logic is in file `state_machine.c`, which implements the state machine task. `freq_irq.c` implements the frequency interrrupt handler. `load_control.c` implements the load states utility functions and the `red_led_control` task. `threshold_control.c` implements the PS/2 interrupt handler and the threshold utility functions. timer.c implements the 0.5sec timer used in the state machine. `vga.c` implements the vga task. `panic.h` is the `panic()` function that prints an error message and hangs the system. `main.c` is the entry point of our system and initialises all subsystems and calls scheduler.

Documentation
=============

For a complete project report see `./doc/report.pdf`.

Running the Project
===================

Import the project folder into eclipse. Import the bsp (`freq_relay_proj_bsp`) first, then `freq_relay_proj`.

It may be a good idea to put the whole workspace directory in the `H:` drive, as some of the eclipse-generated code may have reference to absolute path.
