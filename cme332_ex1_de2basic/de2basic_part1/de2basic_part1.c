#include "address_map_nios2.h"
volatile int *red_LED_ptr = (int *) LEDR_BASE;
volatile int *sw_switch_ptr = (int *)SW_BASE;
volatile int *hex3_0_ptr = (int *) HEX3_HEX0_BASE;
// global variables
int sw_value;
int *ind_num;
int hex0 = 0;
int hex1 = 0;
int hex2 = 0;
int num_val;
// function declarations
int hex_display_cnvtr(int value);
void digit_separator(int* array,int digits);



int main(void){

    while(1){
        sw_value = *(sw_switch_ptr) & 0xFF;
        *(red_LED_ptr) = sw_value;
        //digit_separator( ind_num,sw_value);
        num_val = sw_value;
        
        // get each digit starting with ones and then go up
        hex0 = (num_val % 10);
        num_val/= 10;
        hex1 = (num_val % 10);
        num_val /= 10;
        hex2 = (num_val % 10);

        
        *(hex3_0_ptr) = (hex_display_cnvtr(hex2)<<16) + (hex_display_cnvtr(hex1)<<8)+ hex_display_cnvtr(hex0);
        //*(hex3_0_ptr) = hex2 + hex1 + hex0;

            
    }
}

/***
 * this function converts the binary switch values to be displayed on a hex display
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

void digit_separator(int* array, int numbers){
    int dig = numbers;

    if (numbers < 100){
        array[0] = 0;
    }
    if (numbers < 10){
        array[1] = 0;
        }
    for (int i = 0; dig != 0; i++){
        if (array[i] != 0){
              array[i] = dig % 10;
              dig /= 10;  
        }

    }
}



