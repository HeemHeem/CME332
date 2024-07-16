// Jason Xie, xix277, 11255431, CME332 LAB 2

#include "address_map_nios2.h"
#include "globals.h" // defines global values
#include "nios2_ctrl_reg_macros.h"

/* the global variables are written by interrupt service routines; we have to
 * declare
 * these as volatile to avoid the compiler caching their values in registers */
// volatile int pattern      = 0x0000000F; // pattern for shifting
// volatile int shift_dir    = LEFT;       // direction to shift the pattern
// volatile int shift_enable = ENABLE; // enable/disable shifting of the pattern


//pointers
// volatile int *red_LED_ptr = (int *) LEDR_BASE;
// volatile int *sw_switch_ptr = (int *)SW_BASE;
// volatile int *hex3_0_ptr = (int *) HEX3_HEX0_BASE;
// volatile int *green_LED_ptr = (int * ) LEDG_BASE;

// acceleration/speed variables
//int new_speed = 0;
volatile int speed = 0;
volatile int acceleration = 0;
int speed_in_km = 0;



// other global variables
int sw_value;
int key_value;
int* hex_array; // hold 3 numbers

//function declarations
int hex_display_cnvtr(int value);
void digit_separator(int *array, int numbers);




/*******************************************************************************
 * This program demonstrates use of interrupts. It
 * first starts the interval timer with 50 msec timeouts, and then enables
 * Nios II interrupts from the interval timer and pushbutton KEYs
 *
 * The interrupt service routine for the interval timer displays a pattern on
 * the LED lights, and shifts this pattern either left or right. The shifting
 * direction is reversed when KEY[1] is pressed
********************************************************************************/
int main(void) {
    /* Declare volatile pointers to I/O registers (volatile means that IO load
     * and store instructions will be used to access these pointer locations,
     * instead of regular memory loads and stores)
     */
    //pointers
    //volatile int *red_LED_ptr = (int *) LEDR_BASE;
    //volatile int *sw_switch_ptr = (int *)SW_BASE;
    volatile int *hex3_0_ptr = (int *) HEX3_HEX0_BASE;
    volatile int *green_LED_ptr = (int * ) LEDG_BASE;
    volatile int * interval_timer_ptr = (int *)TIMER_BASE; // interal timer base address
    volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address

    /* set the interval timer period for scrolling the LED lights */
    int counter                 = 50000000; // 1/(50 MHz) x (50000000) = 1 sec
    *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
    *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;

    /* start interval timer, enable its interrupts */
    *(interval_timer_ptr + 1) = 0x7; // STOP = 0, START = 1, CONT = 1, ITO = 1
    /* set interrupt mask bits for levels 0 (interval timer) and level 1
     * (pushbuttons) */
    NIOS2_WRITE_IENABLE(0x1);

    NIOS2_WRITE_STATUS(1); // enable Nios II interrupts

    //display initial speed
    digit_separator(hex_array, speed);
    *(hex3_0_ptr) = (hex_display_cnvtr(hex_array[0])<<16) + (hex_display_cnvtr(hex_array[1])<<8)+ hex_display_cnvtr(hex_array[2]);

    while (1){
            key_value = *KEY_ptr;
            //sw_value = *(sw_switch_ptr) & 0xFF;
            *(green_LED_ptr) = key_value;
            
            // brake pedal first, 0xC because brake pedal takes priority
            if(key_value == 0x8 || key_value == 0xC){
                acceleration = -6; // in m/s^2

            }
            // acceleration
            else if (key_value == 0x4){
                acceleration = 3; // in m/s^2
            }
            // no acceleration
            else{
                acceleration = 0;
            }
            speed_in_km = speed*3600/1000;

            
            //check for if speed is over 999 or < 000. ie set a cap
            if (speed_in_km > 999){
                speed_in_km = 999;
            }
            else if (speed_in_km < 0){
                speed_in_km = 0;
            }


           digit_separator(hex_array, speed_in_km);
            *(hex3_0_ptr) = (hex_display_cnvtr(hex_array[0])<<16) + (hex_display_cnvtr(hex_array[1])<<8)+ hex_display_cnvtr(hex_array[2]);
    }
}


void digit_separator(int* array, int numbers){
    int dig = numbers;
    for(int i=2; i >= 0; i--){

        array[i] = dig%10;
        dig/=10;
    }
}




int hex_display_cnvtr(int value){
    switch(value){
        case 1:
            return 0x06;
        case 2:
            return 0x5b;
        case 3:
            return 0x4f;

        case 4:
            return 0x66;

        case 5:
            return 0x6d;
        case 6:
            return 0x7d;
        case 7:
            return 0x07;
        case 8:
            return 0x7f;
        case 9:
            return 0x6f;
        default:
            // this is for 0
            return 0x3f;

    }

}