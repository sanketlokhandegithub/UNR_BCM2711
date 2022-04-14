/* Author: Sanket L. (slokhande@unr.edu)
  
    Copyright (C) 2022  Sanket Lokhande

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
* 
*   
*  File Description: This driver file is intended to interface the BCM2711 GPIO with POSIX / *Unix sytems.  
* 
       (raspi-gpio comments) Since kernel 4.1.7 (2015) character device /dev/gpiomem maps the GPIO register page.
        /dev/gpiomem allows GPIO access without root to members of the gpio group.
        This does not provide access the PWM or clock hardware registers.
        The alternative is to use /dev/mem which requires root and determining the address of the GPIO peripheral which varies depending on SoC.
        This function will use /dev/mem if invoked with root privileges and try /dev/gpiomem if not.

* Unauthorized Distrubution is strictly prohibited.
* Rev 1: GPIO support added  // TODO: Make class
*/

//Basic Includes
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
//Driver header
#include "UNR_GPIO_BCM2711.h"


//Defines based off of BCM2711 Datasheet. (https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf)
#define UNR_BCM2711_GPIO_BASE_ADDR  0x7E200000

// GPIO Register Assignments
#define UNR_FSEL_OFFSET                 0   // 0x0000
#define UNR_SET_OFFSET                  7   // 0x001c / 4   // 4 bytes each 
#define UNR_CLR_OFFSET                  10  // 0x0028 / 4
#define UNR_PINLEVEL_OFFSET             13  // 0x0034 / 4
#define UNR_PINEVT_OFFSET               16  // 0x0040 / 4
#define UNR_PINRISING_OFFSET            19  // 0x004C / 4
// TODO : Add rest of the rising - falling edge, Async, Low detect high detect registers // Not important for Rev 1 implementation
// TODO : Setup Clock registers
// TODO : Setup PWM registers

#define UNR_PULLUPDN_OFFSET_2711_0      57  // 0x00e4 / 4
#define UNR_PULLUPDN_OFFSET_2711_1      58
#define UNR_PULLUPDN_OFFSET_2711_2      59
#define UNR_PULLUPDN_OFFSET_2711_3      60

#define ARM_PAGE_SIZE  (4*1024)                     // Must check BCM2835 driver class for accuracy
#define ARM_BLOCK_SIZE (4*1024)

volatile uint32_t* gpio_map;
int piGPIOSetup = 0;
int piMemSetup = 0;

/*
* Function map_gpio_mem 
*  Boiler Plate code for memory mapping in POSIX systems
*   Input: FileDescriptor, GPIO-BaseAddress
*   Returns: 0 for successful operation, 2 (SETUP_MALLOC_FAIL) or 3 (SETUP_MMAP_FAIL) for failed operation
*/
int map_gpio_mem(int mem_fd, uint32_t gpio_base) {
    uint8_t* gpio_mem; //	The base address of the GPIO memory mapped hardware IO

    // allocate the gpio memory size based on size of the gpio block and each page size
    if ((gpio_mem = (uint8_t*)malloc(ARM_BLOCK_SIZE + (ARM_PAGE_SIZE - 1))) == NULL)
        return SETUP_MALLOC_FAIL;

    // if gpio mem allocation is not a multiple of page size, then and then only reformat
    if ((uint32_t)gpio_mem % (uint32_t)ARM_PAGE_SIZE)
        gpio_mem += ARM_PAGE_SIZE - (uint32_t)gpio_mem % ARM_PAGE_SIZE;

    //finally map the allocated memory using the mmap function
    if ((gpio_map = (uint32_t*)mmap(
        (void*)gpio_mem, ARM_BLOCK_SIZE, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_FIXED, mem_fd, gpio_base)) == MAP_FAILED)
        return SETUP_MMAP_FAIL;
    piGPIOSetup = 1;
    return 0;
}


/* Function Setup
* Input to function: none
* Output: 0 if successful else it gives a number as returned by map_gpio_mem function
*/

int setup(void) {
    int mem_fd;
    int map_result;
    uint32_t gpio_base;

    if (piGPIOSetup)
        return SETUP_OK; // already initialized

    // open the gpiomem file in {Read , Synchronized and close on execute} 
    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
        if ((mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC | O_CLOEXEC)) > 0)
            return map_gpio_mem(mem_fd, 0);
    }

    gpio_base = UNR_BCM2711_GPIO_BASE_ADDR;

    map_result = map_gpio_mem(mem_fd, gpio_base);   // this is the line that sets up the memory map
    if (map_result)
        return map_result;

    piMemSetup = 1;
    return SETUP_OK;
}

/* Function to check if the GPIO memory map is already setted up or not
* input : none
* output : none (except the print statement of the standard error)
*/
static void setupCheck(void) {
    if (piGPIOSetup)
        return;
    fprintf(stderr, "Pi Setup failure\n");
    exit(5); // setup failed TODO: create a define instead of writing 5
}

/* Function to set the pull up or pull down method on a given pin
* input : GPIO pin , pull up or pull down
* output : none
*/
static void set_pullupdn(int gpio, int pud)
{
    setupCheck(); //sanity check

    // Pi 4 Pull-up/down method
    int pullreg = UNR_PULLUPDN_OFFSET_2711_0 + (gpio >> 4);
    int pullshift = (gpio & 0xf) << 1;
    unsigned int pullbits;
    unsigned int pull = 0;
    switch (pud) {     // values from page 101 of https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
    case PUD_OFF:    pull = 0; break;
    case PUD_DOWN:   pull = 1; break;
    case PUD_UP:     pull = 2; break;
    default:         pull = 0; // switch PUD to OFF for other values
    }
    pullbits = *(gpio_map + pullreg);
    pullbits &= ~(3 << pullshift);
    pullbits |= (pull << pullshift);
    *(gpio_map + pullreg) = pullbits;
    
}

int get_pullupdn(int gpio) {
    setupCheck(); // sanity check
    // Pi 4 Pull-up/down check method
    int pullreg = UNR_PULLUPDN_OFFSET_2711_0 + (gpio >> 4);
    int pullshift = (gpio & 0xf) << 1;
    unsigned int pullbits;
    pullbits = *(gpio_map + pullreg);
    pullbits &= (3 << pullshift);
    pullbits >>= pullshift;
    return pullbits;

}

/* Function to setup the GPIO pin to either read a physical pin or output a given value.
* input: GPIO pin, INPUT / OUTPUT, pull up or pull down method
* output : none
*/
void setup_gpio(int gpio, int direction, int pud) {
    setupCheck();
    int offset = UNR_FSEL_OFFSET + (gpio / 10);
    int shift = (gpio % 10) * 3;
    set_pullupdn(gpio, pud);
    if (direction == OUTPUT)
        *(gpio_map + offset) =
        (*(gpio_map + offset) & ~(7 << shift)) | (1 << shift);
    else // direction == INPUT
        *(gpio_map + offset) = (*(gpio_map + offset) & ~(7 << shift));
}


/*  Function to output a 0 or 1 on given GPIO
* input : GPIO pin, 0 or a 1
* output : none
*/
void output_gpio(int gpio, int value) {
    int offset, shift;

    setupCheck(); // sanity check
    if (value) // value == HIGH
        offset = UNR_SET_OFFSET + (gpio / 32);
    else // value == LOW
        offset = UNR_CLR_OFFSET + (gpio / 32);

    shift = (gpio % 32);

    *(gpio_map + offset) = 1 << shift;
}

/* Function to make the GPIO read the digital input
*  Input : GPIO 
*  Output : 0 or 1 as read by the pin
*/
int input_gpio(int gpio) {
    unsigned int offset, value, mask;

    setupCheck();
    offset = UNR_PINLEVEL_OFFSET + (gpio / 32);
    mask = (1 << gpio % 32);
    value = *(gpio_map + offset) & mask;
    return value ? 1 : 0;
}

// deallocate the memory when done. Always run this function at the end of your implementation
void cleanup(void) 
{
    if (piGPIOSetup)
        //munmap((void*)gpio_map, ARM_BLOCK_SIZE);
        munmap((void*)gpio_map, (ARM_BLOCK_SIZE + (ARM_PAGE_SIZE - 1)));
}

// -----------------------------
