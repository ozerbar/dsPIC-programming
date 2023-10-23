#include <xc.h>
#include <p33Fxxxx.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL 
#include <libpic30.h>

#include "lcd.h"
#include "led.h"

/* Configuration of the Chip */
// Initial Oscillator Source Selection = Primary (XT, HS, EC) Oscillator with PLL
#pragma config FNOSC = PRIPLL
// Primary Oscillator Mode Select = XT Crystal Oscillator mode
#pragma config POSCMD = XT
// Watchdog Timer Enable = Watchdog Timer enabled/disabled by user software
// (LPRC can be disabled by clearing the SWDTEN bit in the RCON register)
#pragma config FWDTEN = OFF

uint8_t global_timer_flag = 0;
// Interrupt Service Routine triggered when Timer1’s counter matches the timer period.
// Interrupts will be globally disabled during the ISR’s execution.
void __attribute__((__interrupt__)) _T1Interrupt(void){
    global_timer_flag = 1;   // set a global flag
    IFS0bits.T1IF = 0;    // Clear the interrupt flag
}

// Function to print Group Members name
int print_group_members(){
    // Clear the Screen and reset the cursor
    lcd_clear();
    lcd_locate(0, 0);
    
    // Print Group Members name
    lcd_printf("Group Members: \r");
    lcd_printf("* Baran Özer \r");
    lcd_printf("* Shikang Qi \r");
    lcd_printf("* Zhihang Wei\r");
    return 0;
}

int print_counter(uint8_t counter_var){
    // let us print the counter variable at the last row (7)
    // clear the row first
    lcd_clear_row(7);
    
    // Move the cursor to location (column, row). 
    // Valid values for row are 0 through 7.
    // Valid values for column are 0 through 20
    // let us go to row 7 now
    lcd_locate(0, 7);
    lcd_printf("Counter Variable: %d\r", counter_var);
    return 0;
}

int light_led(uint8_t counter_var){
    //highest to lowest LED:
    //LEN1, LED2, ... ,LED5

    if(counter_var % 2 == 1){
        SETLED(LED5_PORT);
    }else{
        CLEARLED(LED5_PORT);
    }
    if(counter_var % 4 == 1){
        SETLED(LED4_PORT);
    }else{
        CLEARLED(LED4_PORT);
    }
    if(counter_var % 8 == 1){
        SETLED(LED3_PORT);
    }else{
        CLEARLED(LED3_PORT);
    }
    if(counter_var % 16 == 1){
        SETLED(LED2_PORT);
    }else{
        CLEARLED(LED2_PORT);
    }
    if(counter_var % 32 == 1){
        SETLED(LED1_PORT);
    }else{
        CLEARLED(LED1_PORT);
    }

    while(global_timer_flag==0)
        ;

    global_timer_flag = 0
}

//int light_led_regards_reminder(int reminder_bit, int )



int main(){
    //Init LCD and LEDs
    lcd_initialize();
    led_init();
	
    print_group_members();

    uint8_t counter_var;

    CLEARBIT(LED1_TRIS);
    CLEARBIT(LED2_TRIS);
    CLEARBIT(LED3_TRIS);
    CLEARBIT(LED4_TRIS);
    CLEARBIT(LED5_TRIS);

    CLEARBIT(T1CONbits.TON);
    CLEARBIT(T1CONbits.TCS);
    CLEARBIT(T1CONbits.TGATE);
    TMR1 = 0x00;
    T1CONbits.TCKPS = 0b10;
    PR1 = 10;
    IPC0bits.T1IP = 0x01;
    CLEARBIT(IFS0bits.T1IF);
    SETBIT(IEC0bits.T1IE);
    SETBIT(T1CONbits.TON);
    // Disable Timer
    // Select internal instruction cycle clock
    // Disable Gated Timer mode
    // Clear timer register
    // Select 1:64 Prescaler
    // Load the period value
    // Set Timer1 Interrupt Priority Level
    // Clear Timer1 Interrupt Flag
    // Enable Timer1 interrupt
    // Start Timer
    
    for(counter_var = 0; ; counter_var++){
        print_counter(counter_var);
        light_led(counter_var);
    }
    

    // Stop
    while(1)
        ;
}


