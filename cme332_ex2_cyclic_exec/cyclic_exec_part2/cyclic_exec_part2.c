// Jason Xie, xix277, 11255431, CME332 LAB 2

#include "address_map_nios2.h"
#include "globals.h" // defines global values
#include "nios2_ctrl_reg_macros.h"
#include <stdio.h>

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
volatile int *red_LED_ptr = (int *) LEDR_BASE;
volatile int *switch_ptr = (int *)SW_BASE;
volatile int *hex3_0_ptr = (int *) HEX3_HEX0_BASE;
volatile int *green_LED_ptr = (int * ) LEDG_BASE;
volatile int * interval_timer_ptr = (int *)TIMER_BASE; // interal timer base address
volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address

// global variables
volatile int frame = 0;
volatile int T1_flag = 0;
volatile int T2_flag = 0;
volatile int T3_flag = 0;
volatile int frame_overun = 0;

volatile int calculate = ON;
volatile int speed_timer = 0;
double speed = 0;
int acceleration = 0;
int speed_in_km = 0;

// engine tasks variables
int throttle;
int engine;
//int turn_on_off_flag;
volatile int eng_timer;

// I/O task variables
int key_fob;
int drive_neutral;
int park;
volatile int brake_pedal;
volatile int accel_pedal;
volatile int eng_start_stop;


int sw_value;
int sw9;
int sw1;
int sw0;
int key_value;
int key3;
int key2;
int key0;
int* hex_array; // hold 3 numbers

//time the execution of the tasks
int T1_start;
int T1_end;
int T2_start;
int T2_end;
int T3_start;
int T3_end;
int T1_max = 0;
int T2_max = 0;
int T3_max = 0;

//function declarations
int hex_display_cnvtr(int value);
void digit_separator(int *array, int numbers);
void engine_task(void);
void IO_task(void);
void vehicle_task(void);




int main(void) {
    /* set the interval timer*/
    int counter                 = 25000; // 1/(50 MHz) x (25000) = 0.5 msec frame
    *(interval_timer_ptr + 0x2) = (counter & 0xFFFF);
    *(interval_timer_ptr + 0x3) = (counter >> 16) & 0xFFFF;

    /* start interval timer, enable its interrupts */
    *(interval_timer_ptr + 1) = 0x7; // STOP = 0, START = 1, CONT = 1, ITO = 1
    /* set interrupt mask bits for levels 0 (interval timer) and level 1 */
    NIOS2_WRITE_IENABLE(0x1);

    NIOS2_WRITE_STATUS(1); // enable Nios II interrupts

    //display initial speed

    while (1){
        if(frame == 1){
            // *(interval_timer_ptr+4) = 1;
            // T1_start = *(interval_timer_ptr + 4) + (*(interval_timer_ptr) << 16);
            T1_flag = 1;
            IO_task();
            T1_flag = 0;
            // *(interval_timer_ptr+4) = 1;
            // T1_end = *(interval_timer_ptr + 4) + (*(interval_timer_ptr) << 16);

        }
        if (frame == 2){
            // *(interval_timer_ptr+4) = 1;
            // T2_start = *(interval_timer_ptr + 4) + (*(interval_timer_ptr) << 16);
            T2_flag = 1;
            engine_task();
            T2_flag = 0;
            // *(interval_timer_ptr+4) = 1;
            // T2_end = *(interval_timer_ptr + 4) + (*(interval_timer_ptr) << 16);

            // *(interval_timer_ptr+4) = 1;
            // T3_start = *(interval_timer_ptr + 4) + (*(interval_timer_ptr) << 16);
        if (frame == 3){
            T3_flag = 1;    
            vehicle_task();
            T3_flag = 0;
            // *(interval_timer_ptr+4) = 1;
            // T3_end = *(interval_timer_ptr + 4) + (*(interval_timer_ptr) << 16);         
        }
        if (frame_overun){
            *green_LED_ptr |= 0x100;
        }
        // // ignore restart values
        // if ((T1_start - T1_end) > 0){
        //     int dummy1 = (T1_start - T1_end);

        //     if (dummy1 > T1_max){
        //         printf("%d this is the new max execution time in for T1 in clock cycles\n", dummy1);
        //         T1_max = dummy1;
        //     }
        // }
        // if ((T2_start - T2_end) > 0){
        //     int dummy2 = (T2_start - T2_end);

        //     if (dummy2 > T2_max){
        //         printf("%d this is the new max execution time in for T2 in clock cycles\n", dummy2);
        //         T2_max = dummy2;
        //     }
        // }
        // if ((T3_start - T3_end) > 0){
        //     int dummy3 = (T3_start - T3_end);

        //     if (dummy3 > T3_max){
        //         printf("%d this is the new max execution time in for T3 in clock cycles\n", dummy3);
        //         T3_max = dummy3;
        //     }

        }
}
}

/**
 * takes in a pointer to an array, and an integer of 3 digits and appends them into a 3 digit array
 * 
 */
void digit_separator(int* array, int numbers){
    int dig = numbers;
    for(int i=2; i >= 0; i--){

        array[i] = dig%10;
        dig/=10;
    }
}



/**
 * converts integer value to the corresponding hex segments value. 
 * 
 */
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


/**
 * 
 * Engine Task:
 * Responsible for throttle based input status and vehicle speed
 * constraints:
 * Engine must be turned on in order to generate force to accelerate the vehicle.
 * Engine status and gear position are handled by I/O task
 * If accelerator is depressed, throttle equals 6
 * to start the engine: (1) key fob is inside vehicle; (2) step on brake; and (3) press and hold the engine start/stop button for longer than 1 second
 * to turn engine off: (1) gear is in park; (2) press the engine start/stop button
 * to keep the vehicle model simple, engine cannot be turned off during driving
 */
void engine_task(void){
    //T2_flag = 1;

    // remeber to put in the 1 second counter
    if((key_fob == ON) && (brake_pedal == ON) && (eng_start_stop == ON)){
        
        // check that its been held for longer than 1 second. still in the works
        if(eng_timer > 2000){
            engine = ON;
            //eng_timer = 0;
        }
    }

    // turn off the engine
    if ((park == ON) && (eng_start_stop == ON) && (engine == ON)){
        engine = OFF;
    }

    if((accel_pedal == ON) && (engine == ON) ){
        throttle = 6;
    }
    else{
        throttle = 0;
    }
    //T2_flag = 0;
}


/**
 * I/O Task:
 * responsible for handling inputs to the system, and displaying outputs to the user. This task does not modify any of the properties
 * constraints on I/O
 * I/O tasks records the status of the inputs for the engine task to reference to
 * I/O tasks updates vehicle speed (from the vehicle task) on HEX2-HEX0.
 */
void IO_task(void){

    //T1_flag = 1;
    sw9 = *switch_ptr & 0x200; //sw9
    sw1 = *switch_ptr & 0x2; //sw1
    sw0 = *switch_ptr & 0x1; // sw0
    key3 = *KEY_ptr & 0x8;
    key2 = *KEY_ptr & 0x4;
    key0 = *KEY_ptr & 0x1;
    *red_LED_ptr = sw9 + sw1 + sw0;
    *green_LED_ptr = key0 + key2 + key3; 

    //key fob
    if (sw9 == 0x200){
       key_fob = ON; 
    }
    else{
        key_fob = OFF;
    }

    //drive or neutral
    if (sw1 == 0x2){
        drive_neutral = ON;
    }
    else{
        drive_neutral = OFF;
    }

    // park on/off
    if(sw0 == 0x1 && speed == 0){
        park = ON;    
    }
    else{
        park = OFF;
    }

    // brake pedal
    if(key3 == 0x8){
       brake_pedal = ON; 
    }
    else{
        brake_pedal = OFF;
    }
    // accel_pedal
    if(key2 == 0x4){
       accel_pedal = ON; 
    }
    else{
        accel_pedal = OFF;
    }
    
    // eng_start_stop
    if(key0 == 0x1){
        eng_start_stop = ON;
    }
    else{
        eng_start_stop = OFF;
    }
    

    speed_in_km = speed*3.6;
    //check for if speed is over 999 or < 000. ie set a cap
    if (speed_in_km > 999){
        speed_in_km = 999;
    }
    else if (speed_in_km < 0){
        speed_in_km = 0;
    }

    if(engine == ON){
      digit_separator(hex_array, speed_in_km);
        *(hex3_0_ptr) = (hex_display_cnvtr(hex_array[0])<<16) + (hex_display_cnvtr(hex_array[1])<<8)+ hex_display_cnvtr(hex_array[2]);  
    }
    else{
        *(hex3_0_ptr) = 0;
    }
    

    //T1_flag = 0;


}



/**
 * 
 * Vehicle_task:
 * Responsible for acceleration and speed of vehicle based on input and external factor. 
 * also modifies the location of the vehicle acccording to the acceleration of the speed
 * if brake pedal is not depressed, input acceleration of the vehicle is equal too acceleration = 0.5 x throttle
 * if brake pedal is depressed, it overriedes the accelerator and the input acceleration is set to -6m/s^2 until speed reaches 0
 * new speed is calculated by new_speed = speed + acceleration x time
 * if neither accelerator and brake is depressed, speed is unchanged
 * 
 */
void vehicle_task(void){
    

    if((brake_pedal == ON)){
        acceleration = -6;
        if(speed <= 0){
            acceleration = 0;
        }

    }
    else if((accel_pedal == ON) && (brake_pedal == OFF)){
        acceleration = 0.5*throttle;
    }
    else{
        acceleration = 0;
    }
    // calculate speed
    if ((drive_neutral == ON) && (calculate == ON)){
       speed = speed + acceleration * 0.0015; // 0.0015 because it updates every 1.5 ms
       calculate = OFF; 
       //speed_timer = 0;
    }

    if (speed < 0){
        speed = 0;
    }
    
}