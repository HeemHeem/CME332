#include "address_map_nios2.h"
#include "globals.h" // defines global values

extern volatile int acceleration, speed;
/*******************************************************************************
 * Interval timer interrupt service routine
 *
 * Shifts a PATTERN being displayed on the LED lights. The shift direction
 * is determined by the external variable key_dir.
 ******************************************************************************/
void interval_timer_ISR() {
    volatile int * interval_timer_ptr = (int *)TIMER_BASE; // timer addresses
    // volatile int * LEDG_ptr           = (int *)LED_BASE; // LED address
    volatile int * key_ptr            = (int *)KEY_BASE; // KEY address
    int key_value = *key_ptr;

    *(interval_timer_ptr) = 0; // clear the interrupt

    speed = speed + acceleration;

    // set cap so its easier to go back down from 999 instead of waiting
    if (speed >= 277.5){
        speed = 277.5;
    }

    if (speed < 0){
        speed = 0;
    }

}

