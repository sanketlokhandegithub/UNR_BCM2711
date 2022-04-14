# UNR_BCM2711

Revision 1 contains GPIO support.

To use the driver files, include the cpp and header file in your implementation. (Tested on UNIX system with linux kernel 5.15) 

Usage:
1. Call "setup()" function in the beginning to get the GPIO memory map into kernel space
2. Set the input / output direction on a particular pin with "setup_gpio(pin, input/output, pull_up/pull_down)"
3. use the function "output_gpio(pin, high / low)" to output on a particular pin
4. use the function "input_gpio(pin)" to read high / low on a pin
5. lastly always use "cleanup()" to make sure the allocated memory is freed before you exit.
