#pragma once

#define SETUP_OK           0
#define SETUP_MALLOC_FAIL  1
#define SETUP_MMAP_FAIL    2

#define INPUT  0 
#define OUTPUT 1 

#define HIGH 1
#define LOW  0

#define PUD_OFF  0
#define PUD_DOWN 2
#define PUD_UP   1

int setup(void);
void cleanup(void);
void setup_gpio(int gpio, int direction, int pud);
void output_gpio(int gpio, int value);
int input_gpio(int gpio);
int get_pullupdn(int gpio);

