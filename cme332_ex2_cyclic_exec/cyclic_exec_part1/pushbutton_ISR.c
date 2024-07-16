// #include "address_map_nios2.h"
// #include "globals.h" // defines global values

// extern volatile int acceleration, seconds, old_seconds;
// /*******************************************************************************
//  * Pushbutton - Interrupt Service Routine
//  *
//  * This routine checks which KEY has been pressed and updates the global
//  * variables as required.
//  ******************************************************************************/
// void pushbutton_ISR(void) {
//     volatile int * KEY_ptr           = (int *)KEY_BASE;
//     volatile int * slider_switch_ptr = (int *)SW_BASE;
//     int            press;

//     press          = *(KEY_ptr + 3); // read the pushbutton interrupt register
//     *(KEY_ptr + 3) = press;          // Clear the interrupt

//     if (press & 0x8) // KEY3 deaccelerate
//     {
//         acceleration = -6;

//     }
        

//     else if (press & 0x4) // KEY2 accelerate
//     {
//         acceleration = 3;
//     }

//     else // no accerleration
//     {

//         acceleration = 0;
//     }


//     return;
// }
