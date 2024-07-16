// Jason Xie, xix277, 11255431, CME332 Lab 1
#include "address_map_nios2.h"
//NOTE, KEY1 works sometimes, but that is because nothing was done about the debouncing.

// Declare Volatile pointers
volatile int *red_LED_ptr = (int *) LEDR_BASE;
volatile int *sw_switch_ptr = (int *)SW_BASE;
volatile int *hex3_0_ptr = (int *) HEX3_HEX0_BASE;
volatile int *KEY_ptr = (int *) KEY_BASE;
volatile int *green_LED_ptr = (int *) LEDG_BASE;

// global variables
int sw_value;
int *ind_num;
int hex0 = 0;
int hex1 = 0;
int hex2 = 0;
int num_val;
int key_value;
int man_on_off = 0;
int reset = 0;


// function declarations
int hex_display_cnvtr(int value);
void digit_separator(int* array,int digits);

int main(void){
    // initial value
    *(hex3_0_ptr) = (hex_display_cnvtr(hex2)<<16) + (hex_display_cnvtr(hex1)<<8)+ hex_display_cnvtr(hex0);

    while(1){
        key_value = *KEY_ptr;
        sw_value = *(sw_switch_ptr) & 0xFF;
        num_val = sw_value;
        *(red_LED_ptr) = sw_value;
        *(green_LED_ptr) = key_value;
       

       // get each digit starting with ones and then go up 

        // Key0 Press
        if(key_value == 0x1){
            hex0 = 0;
            hex1 = 0;
            hex2 = 0;
            man_on_off = 0;
            *(hex3_0_ptr) = (hex_display_cnvtr(hex2)<<16) + (hex_display_cnvtr(hex1)<<8)+ hex_display_cnvtr(hex0);  
        }

        // Key1 press, toggle  between manual or auto
        if (key_value == 0x2){
            man_on_off += 1;
            if(man_on_off > 1){
                man_on_off = 0;
            }
        }
        // Manual
        if (man_on_off == 0){
            hex0 = (num_val % 10);
            num_val/= 10;
            hex1 = (num_val % 10);
            num_val /= 10;
            hex2 = (num_val % 10);
        }
        // auto
        else if(man_on_off == 1){
            hex0 = (num_val % 10);
            num_val/= 10;
            hex1 = (num_val % 10);
            num_val /= 10;
            hex2 = (num_val % 10);
            *(hex3_0_ptr) = (hex_display_cnvtr(hex2)<<16) + (hex_display_cnvtr(hex1)<<8)+ hex_display_cnvtr(hex0);  
        }
        
        if (key_value == 0x4){
            *(hex3_0_ptr) = (hex_display_cnvtr(hex2)<<16) + (hex_display_cnvtr(hex1)<<8)+ hex_display_cnvtr(hex0); 
        }    
    }
}

/***
 * this function converts the binary switch values to be displayed on a hex display
 * parameters: an integer from 0-9
 **/

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

