#include "lab04.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "math.h"

/*#ifndef M_PI
#define MPI 3.14159265358979323846
#endif
*/
#include "types.h"
#include "lcd.h"
#include "led.h"
#include "dac.h"

// signal parameter
#define WAVE_FREQUENCY 10
#define SAMPLE_RATE 500
#define WAVE_MIN_VOLTAGE 1000
#define WAVE_MAX_VOLTAGE 3000
#define TIMER_PERIOD ((12800000/256)/SAMPLE_RATE)


volatile uint8_t FLAG_WAIT_COMPLETE =0;
volatile uint16_t COUNT_IN =0;

/*
 * Timer code
 */

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03
#define START_NEXT_DAC 1
#define DAC_COMPLETE 0

void timer_initialize()
{
    __builtin_write_OSCCONL(OSCCONL | 2);
    CLEARBIT(T3CONbits.TON);
    T3CONbits.TCKPS = TCKPS_256;
    CLEARBIT(T3CONbits.TCS);
    CLEARBIT(T3CONbits.TGATE);
    PR3 = TIMER_PERIOD;
    TMR3 = 0x00;
    IPC2bits.T3IP = 0x01;
    CLEARBIT(IFS0bits.T3IF);
    SETBIT(IEC0bits.T3IE);
    SETBIT(T3CONbits.TON);
}


// interrupt service routine?
void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T3Interrupt(void)
{
    COUNT_IN ++;
    if(COUNT_IN == SAMPLE_RATE* WAVE_FREQUENCY)
        COUNT_IN = 0;
    
    FLAG_WAIT_COMPLETE = START_NEXT_DAC;
    
    CLEARBIT(IFS0bits.T3IF);
}


/*
 * main loop
 */

void main_loop()
{
    // print assignment information
    lcd_printf("Lab04: Wave");
    lcd_locate(0, 1);
    lcd_printf("Group: 2-2");
    
    
    // Initialize Timers
    timer_initialize();

    
    while(TRUE) { 
        while(FLAG_WAIT_COMPLETE != START_NEXT_DAC);
        lcd_locate(0, 2);
        
        
        uint16_t step = COUNT_IN;
        lcd_printf("COUNT: %u", step);
        
        float voltage_float= sin(3.14159265358979323846*2*WAVE_FREQUENCY*step/SAMPLE_RATE) * (float)(WAVE_MAX_VOLTAGE -WAVE_MIN_VOLTAGE) / 2 + (float)(WAVE_MAX_VOLTAGE +WAVE_MIN_VOLTAGE) /2 ;
        
        uint16_t voltage = (uint16_t)voltage_float;


        dac_convert_milli_volt(voltage);

        FLAG_WAIT_COMPLETE = DAC_COMPLETE;
        
        TOGGLELED(LED1_PORT);
        
        
    }
}