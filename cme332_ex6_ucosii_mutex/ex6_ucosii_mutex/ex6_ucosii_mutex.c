
/* TODO:
		- adjust IO_Task
		- set up semaphores for tasks
		- set up mailboxes and message queue
		- adjust speed calculations
		- adjust how long the buttons should be held when starting engine
*/

/* NOTES: - Semaphores are for shared resources. In this case since the IO task controls how everything works
			there should be a semaphore for controlling it (performing a write), and then two other semaphores
			for engine_task and vehicle_task that are performing reads on it.
*/

#include <stdio.h>
#include "includes.h"
#include "address_map_nios2.h"
#include "globals.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK	  task3_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define TASK1_PRIORITY      10
#define TASK2_PRIORITY      11
#define TASK3_PRIORITY		12


/* GLOBAL VARIABLES FOR SEMAPHORES??? */
volatile int *red_LED_ptr = (int *) LEDR_BASE;
volatile int *switch_ptr = (int *)SW_BASE;
volatile int *hex3_0_ptr = (int *) HEX3_HEX0_BASE;
volatile int *green_LED_ptr = (int * ) LEDG_BASE;
volatile int * interval_timer_ptr = (int *)TIMER_BASE; // interval timer base address
volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address

// global variables
int calculate = ON;
int speed_timer = 0;
double speed = 0;
double old_time = 0;
double acceleration = 0;
int speed_in_km = 0;

// Cruise control variables for now
int cruise_set = OFF;
int cruise_speed = 0;
int SW3 = OFF;
int SW4 = OFF;


// engine tasks variables
//int throttle;
int engine;
int eng_timer = 0;

// I/O task variables
int key_fob;
int drive_neutral;
int park;
int brake_pedal;
int accel_pedal;
int eng_start_stop;
//old time tick
int prev_tick = 0;

// switch and keys
int sw_value;
int sw9;
int sw4;
int sw3;
int sw2;
int sw1;
int sw0;
int key_value;
int key3;
int key2;
int key1;
int key0;
int* hex_array; // hold 3 numbers

//float location = 0;

/**************************************************** SEMAPHORES**************************************************/
OS_EVENT *Key_Fob_Mut;
OS_EVENT *Brake_Pedal_Mut;
OS_EVENT *Eng_Start_Stop_Mut;
//OS_EVENT *Engine_Sem;
//OS_EVENT *Park_Sem;
OS_EVENT *Accel_Pedal_Mut;
//OS_EVENT *Throttle_Sem;
OS_EVENT *Speed_Mut;
//OS_EVENT *Drive_Neutral_Sem;
OS_EVENT *Speed_kmh_Mut;
OS_EVENT *Cruise_Speed_Mut;
OS_EVENT *SW4_Mut;
OS_EVENT *SW3_Mut;

/***************************************************************Mailbox********************************************/
OS_EVENT *LocationMbox;

/*******************************************************Queue******************************************************/
OS_EVENT *ThrottleQ;
void *throttle_tbl[1];

/******************************************************Event Flags ************************************************/
#define Gear_Park 0x1
#define Gear_Drive 0x2
#define Cruise 0x4
#define Engine 0x8

OS_FLAG_GRP *EngineFlags;
OS_FLAGS engine_status;


//function declarations
int hex_display_cnvtr(int value);
void digit_separator(int *array, int numbers);
void engine_task(void* pdata);
void IO_task(void* pdata);
void vehicle_task(void* pdata);
void LCD_text(char *text_ptr);
void LCD_cursor(int x, int y);
void LCD_speed(int ss);
void LCD_cruise(int cs);

/* The main function creates two task and starts multi-tasking */
int main(void)
{
	INT8U err;
	Key_Fob_Mut = OSMutexCreate(1,&err);
	Brake_Pedal_Mut = OSMutexCreate(2,&err);
	Eng_Start_Stop_Mut = OSMutexCreate(3,&err);
	Accel_Pedal_Mut = OSMutexCreate(4,&err);
	Speed_kmh_Mut = OSMutexCreate(5,&err);
	Speed_Mut = OSMutexCreate(6,&err);
	Cruise_Speed_Mut = OSMutexCreate(7,&err);
	SW3_Mut = OSMutexCreate(8,&err);
	SW4_Mut = OSMutexCreate(9,&err);
	// Key_Fob_Sem = OSSemCreate(1);
	// Brake_Pedal_Sem = OSSemCreate(1);
	// Eng_Start_Stop_Sem =OSSemCreate(1);
	// Accel_Pedal_Sem = OSSemCreate(1);
	// Speed_kmh_Sem = OSSemCreate(1);
	// Speed_Sem = OSSemCreate(1);;
	// Cruise_Speed_Sem = OSSemCreate(1);
	// SW3_Sem = OSSemCreate(1);
	// SW4_Sem = OSSemCreate(1);
	LocationMbox = OSMboxCreate((void*)0); // create Location mailbox;
	ThrottleQ = OSQCreate(&throttle_tbl[0], 1);
	EngineFlags = OSFlagCreate(0x00, &err);

	OSTaskCreateExt(IO_task,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
	OSTaskCreateExt(engine_task,
                  NULL,
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
	OSTaskCreateExt(vehicle_task,
                  NULL,
                  (void *)&task3_stk[TASK_STACKSIZE-1],
                  TASK3_PRIORITY,
                  TASK3_PRIORITY,
                  task3_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
	OSStart();
	return 0;
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

void LCD_cursor( int x, int y){
	volatile char *LCD_display_ptr = (char *) 0xFF203050;

	int instruction = x;
	if (y != 0) instruction |= 0x40; // set bit 6 for bottom row
	instruction |= 0x80; // need to set bit 7 to set the cursor location
	*(LCD_display_ptr) = instruction; // write to the LCD instruction register
}

void LCD_text(char *text_ptr){
	volatile char *LCD_display_ptr = (char *) 0xFF203050; // 16x2 character display
	while( *(text_ptr)){
		*(LCD_display_ptr + 1) = *(text_ptr); // write to LCD data register
		++text_ptr;
	}
}

void LCD_cruise( int cs){
	char text[20];
	sprintf(text, "Cruise: %d km/h", cs);
	LCD_cursor(0,1);
	LCD_text(text);

}

void LCD_speed( int ss){
	char text[20];
	sprintf(text, "Speed: %d km/h", ss);
	LCD_cursor(0,0);
	LCD_text(text);

}

/* Prints "Hello World" and sleeps for three seconds */
void IO_task(void* pdata)
{
	INT8U err;
	double *location_ptr;
	while (1)
	{
		/* do IO tasks */
		//T1_flag = 1;
		sw9 = *switch_ptr & 0x200; //sw9
		sw4 = *switch_ptr & 0x10;
		sw3 = *switch_ptr & 0x8;
		sw2 = *switch_ptr & 0x4; // sw2 for cruise control
		sw1 = *switch_ptr & 0x2; //sw1
		sw0 = *switch_ptr & 0x1; // sw0
		key3 = *KEY_ptr & 0x8;
		key2 = *KEY_ptr & 0x4;
		key1 = *KEY_ptr & 0x2;
		key0 = *KEY_ptr & 0x1;
		*red_LED_ptr = sw9 + sw1 + sw0;
		//*green_LED_ptr = key0 + key2 + key3; // take out key1 for now

		OSMutexPend(Key_Fob_Mut, 0, &err);
		//key fob
		if (sw9 == 0x200){
		   key_fob = ON;
		}
		else{
			key_fob = OFF;
		}
		OSMutexPost(Key_Fob_Mut);

		//drive or neutral
		if (sw1 == 0x2){
			OSFlagPost(EngineFlags, Gear_Drive, OS_FLAG_SET, &err);
		}
		else{
			OSFlagPost(EngineFlags, Gear_Drive, OS_FLAG_CLR, &err);
		}

		OSMutexPend(Speed_Mut,0, &err);
		// park on/off
		if(sw0 == 0x1 && speed == 0){
			OSFlagPost(EngineFlags, Gear_Park, OS_FLAG_SET, &err);
		}
		else{
			OSFlagPost(EngineFlags, Gear_Park, OS_FLAG_CLR, &err);
		}


		OSMutexPend(Brake_Pedal_Mut, 0, &err);
		// brake pedal
		if(key3 == 0x8){
		   brake_pedal = ON;
		   *green_LED_ptr |= 0x8;
		}
		else{
			brake_pedal = OFF;
			*green_LED_ptr &= ~0x8;
		}
		OSMutexPost(Brake_Pedal_Mut);

		OSMutexPend(Accel_Pedal_Mut, 0, &err);
		// accel_pedal
		if(key2 == 0x4){
		   accel_pedal = ON;
		   *green_LED_ptr |= 0x4;
		}
		else{
			accel_pedal = OFF;
			*green_LED_ptr &= ~0x4;
		}
		OSMutexPost(Accel_Pedal_Mut);

		OSMutexPend(Eng_Start_Stop_Mut, 0, &err);
		// eng_start_stop
		if(key0 == 0x1){
			eng_start_stop = ON;
			*green_LED_ptr |= 0x1;
		}
		else{
			eng_start_stop = OFF;
			*green_LED_ptr &= ~0x1;
		}
		OSMutexPost(Eng_Start_Stop_Mut);

		/********************************************************Engine Turn ON/OFF*****************************************/
		OSMutexPend(Accel_Pedal_Mut, 0, &err);
		OSMutexPend(Key_Fob_Mut, 0, &err);
		OSMutexPend(Eng_Start_Stop_Mut, 0, &err);
		OSMutexPend(Brake_Pedal_Mut, 0, &err);

		if((key_fob == ON) && (brake_pedal == ON) && (eng_start_stop == ON)){
		// check that its been held for longer than 1 second. still in the works
			eng_timer ++;
			if(eng_timer > 8){
				OSFlagPost(EngineFlags, Engine, OS_FLAG_SET, &err);
				eng_timer = 0;
			}
		}


	// turn off the engine
		if (((engine_status & 0x1) == ON) && (eng_start_stop == ON) && ((engine_status & 0x8) == 0x8)){
			OSFlagPost(EngineFlags, Engine, OS_FLAG_CLR, &err);
		}

		engine_status = OSFlagAccept(EngineFlags, Gear_Drive + Gear_Park + Engine, OS_FLAG_WAIT_SET_ALL, &err);

		OSMutexPost(Accel_Pedal_Mut);
		OSMutexPost(Key_Fob_Mut);
		OSMutexPost(Eng_Start_Stop_Mut);
		OSMutexPost(Brake_Pedal_Mut);

		OSMutexPend(Speed_kmh_Mut, 0, &err);
		speed_in_km = speed*3.6;

		OSMutexPost(Speed_Mut);
		//check for if speed is over 999 or < 000. ie set a cap
		if (speed_in_km > 999){
			speed_in_km = 999;
		}
		else if (speed_in_km < 0){
			speed_in_km = 0;
		}
		/*********************************************Cruise control ********************************************/

		if ((speed_in_km > 40) && (sw2 == 0x4)){
			OSFlagPost(EngineFlags, Cruise, OS_FLAG_SET, &err);
			*red_LED_ptr |= 0x4;
			//printf("Cruise on");
		}
		else{
			OSFlagPost(EngineFlags, Cruise, OS_FLAG_CLR, &err);
			*red_LED_ptr &= ~0x4;
			*green_LED_ptr &= ~0x2;
		}

		OSMutexPost(Speed_kmh_Mut);
		// set flags again
		engine_status = OSFlagAccept(EngineFlags, Gear_Drive + Gear_Park + Engine + Cruise, OS_FLAG_WAIT_SET_ALL, &err);

		// cruise controls after cruise is turned on
		if ((engine_status & 0x4) == 0x4){
			// set cruise speed
			if (key1 == 0x2){
				cruise_set = ON;
				*green_LED_ptr |= 0x2; // keep ledg2 on
			}
			else{
				cruise_set = OFF;
			}

			OSMutexPend(Brake_Pedal_Mut, 0, &err);
			if (brake_pedal == ON){
				cruise_set = OFF;
				*green_LED_ptr &= ~0x2;

			}
			OSMutexPost(Brake_Pedal_Mut);

			OSMutexPend(SW4_Mut, 0, &err);
			OSMutexPend(SW3_Mut, 0, &err);
			if (cruise_set == ON){
				// set switches
				if (sw4 == 0x10){
					SW4 = ON;
				}
				else{
					SW4 = OFF;
				}

				if (sw3 == 0x8){
					SW3 = ON;
				}
				else{
					SW3 = OFF;
				}


			}
			OSMutexPost(SW4_Mut);
			OSMutexPost(SW3_Mut);

		}


		/********************************************Display******************************************************/
		OSMutexPend(Speed_kmh_Mut, 0, &err);
		OSMutexPend(Cruise_Speed_Mut,0, &err);
		if((engine_status & 0x8) == 0x8){
			digit_separator(hex_array, speed_in_km);
			*(hex3_0_ptr) = (hex_display_cnvtr(hex_array[0])<<16) + (hex_display_cnvtr(hex_array[1])<<8)+ hex_display_cnvtr(hex_array[2]);
			LCD_speed(speed_in_km);
			LCD_cruise(cruise_speed);
		}

		else{
			*(hex3_0_ptr) = 0;
		}
		OSMutexPost(Speed_kmh_Mut);
		OSMutexPost(Cruise_Speed_Mut);

		/*******************************************Location LEDs**************************************/
		location_ptr = (double*)OSMboxPend(LocationMbox, 0 , &err);
		// LEDR3
		if (*location_ptr< 300){
			*red_LED_ptr |= 0x8;
			*red_LED_ptr &= ~0x1F0;
		}
		//LEDR4
		else if ((*location_ptr >= 300) && (*location_ptr< 600) ){
			*red_LED_ptr |= 0x010;
			*red_LED_ptr &= ~0x1E8;
		}
		//LEDR5
		else if ((*location_ptr >= 600) && (*location_ptr < 900) ){
			*red_LED_ptr |= 0x020;
			*red_LED_ptr &= ~0x1D8;
		}
		//LEDR6
		else if ((*location_ptr >= 900) && (*location_ptr < 1200) ){
			*red_LED_ptr |= 0x040;
			*red_LED_ptr &= ~0x1B8;
		}
		//LEDR7
		else if ((*location_ptr >= 1200) && (*location_ptr < 1500) ){
			*red_LED_ptr |= 0x080;
			*red_LED_ptr &= ~0x178;
		}
		//LEDR8
		else if ((*location_ptr>= 1500) && (*location_ptr < 1800) ){
			*red_LED_ptr |= 0x100;
			*red_LED_ptr &= ~0xF8;
		}

		OSTimeDlyHMSM(0,0,0,125);
	  }
}
/* Prints "Hello World" and sleeps for three seconds */
void engine_task(void* pdata)
{
	INT8U err;
	int *throttle;
	*throttle = 0;
	while (1)
	{
	// do engine tasks
		OSMutexPend(Accel_Pedal_Mut, 0, &err);
		OSMutexPend(Speed_kmh_Mut, 0, &err);
		OSMutexPend(Cruise_Speed_Mut, 0, &err);

		int diff_c_s = speed_in_km - cruise_speed;

		OSMutexPost(Speed_kmh_Mut);
		OSMutexPost(Cruise_Speed_Mut);

		if((accel_pedal == ON) && ((engine_status & 0xA) == 0xA)){ //only access if engine is on, accel pedal is pressed, and gear is in drive
			*throttle = 6;

			//throttle = 6;
		}
		else if( ((engine_status & 0x4) == 0x4) && (diff_c_s >= 0)){
			// set throttle to 0 if the current speed is higher than the cruise
			*throttle = 0;
		}

		else if ( ((engine_status & 0x4) == 0x4) && (diff_c_s > -2)){
			// set throttle to 2 if current speed is lower than cruise by a little
			*throttle = 2;
		}
		else if ( ((engine_status & 0x4) == 0x4) && (diff_c_s <= -2)){
			// set throttle to 6 if current speed is much lower than cruise
			*throttle = 6;
		}
		else{
			*throttle = 0;
			//throttle = 0;
		}
		//printf("%d",*throttle);

		OSQPost(ThrottleQ, (void *)throttle);
		OSMutexPost(Accel_Pedal_Mut);
		OSTimeDlyHMSM(0,0,0,125);
	}
}

void vehicle_task(void *pdata)
{
	INT8U err;
	double drag = 0;
	double location = 0;
	double retardation = 0;
	double slope_retardation = 0;
	int* throttle_ptr;
	int throttle;
	float time;
	int new_tick;


	while(1)
	{
		OSMutexPend(Brake_Pedal_Mut, 0, &err);
		// speed must be guarded by a semaphore or must be sent through a mailbox or message queue
		OSMutexPend(Speed_Mut,0, &err);
		OSMutexPend(Accel_Pedal_Mut, 0, &err);

		if ((engine_status & 0x8) == 0x8){ // engine is turned on
			/*********************************Slope**********************************************/
			if (location < 300){
				slope_retardation = 0;
			}
			else if ((location >= 300) && (location < 600) ){
				slope_retardation = 0.3;
			}
			else if ((location >= 600) && (location < 900) ){
							slope_retardation = 0.5;
			}
			else if ((location >= 900) && (location < 1200) ){
							slope_retardation = 0;
			}
			else if ((location >= 1200) && (location < 1500) ){
							slope_retardation = -0.4;
			}
			else if ((location >= 1500) && (location < 1800) ){
							slope_retardation = -0.2;
			}

			/*********************************Drag**********************************************/
			if ((accel_pedal == OFF) && (brake_pedal == OFF)){
				drag = speed*speed/1000;
			}
			else{
				drag = 0;
			}

			// calculate retardation
			retardation = drag + slope_retardation;

			/*********************************Acceleration**********************************************/
			//do vehicle task
			if((brake_pedal == ON)){
				acceleration = -6;
				if(speed <= 0){
					acceleration = 0;
				}

			}

			// calculate net acceleration
			else{
				/*****************************THROTTLEQ PEND***********************************/
				throttle_ptr=(int *) OSQPend(ThrottleQ, 0, &err);
				throttle = *throttle_ptr;
				//printf("%d\n",throttle);
				acceleration = 0.5*throttle - retardation;
			}


			//if (drive_neutral == ON){// && (engine == ON)){
			// calculate the time
			new_tick = OSTimeGet();
			time = (new_tick-prev_tick)*0.125;

			// calculate location
			location = location + speed*time + 0.5*acceleration*time*time;
			if (location >= 1800){
				location = 0;
			}
			//printf("new location %f\n", location);

			// calculate speed
			speed = speed + acceleration *time; // 0.125s delay
			   //printf("%f\n", speed);
			   //calculate = OFF;
			   //speed_timer = 0;
			//}

			if (speed < 0){
				speed = 0;
			}
			prev_tick = new_tick;

			/***************************************** Cruise *****************************/
			OSMutexPend(SW3_Mut,0, &err);
			OSMutexPend(SW4_Mut,0, &err);
			OSMutexPend(Cruise_Speed_Mut,0, &err);
			if ((engine_status & 0x4) == 0x4){ // cruise is on

				// set cruise speed to current speed
				if( (SW4 == OFF) && (SW3 == OFF) && (cruise_set == ON)){
					cruise_speed = speed*3.6;
				}
				// decrease cruise speed by 2.0km/h
				if( (SW4 == OFF) && (SW3 == ON) && (cruise_set == ON) && (cruise_speed > 40)){
					cruise_speed -= 2;
				}

				// increase cruise speed by 2.0km/h
				if( (SW4 == ON) && (SW3 == OFF) && (cruise_set == ON)){
					cruise_speed += 2;
				}

				// decrease cruise speed by 2.0km/h
				if( (SW4 == ON) && (SW3 == ON) && (cruise_set == ON)){
					cruise_speed = cruise_speed + 0;
				}


			}
			OSMutexPost(SW3_Mut);
			OSMutexPost(SW4_Mut);
			OSMutexPost(Cruise_Speed_Mut);

		}
		OSMboxPost(LocationMbox, (void *)&location); // mailbox
		OSMutexPost(Brake_Pedal_Mut);
		OSMutexPost(Speed_Mut);
		OSMutexPost(Accel_Pedal_Mut);
	    OSTimeDlyHMSM(0,0,0,125);
	}
}

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
