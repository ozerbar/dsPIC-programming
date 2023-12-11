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
#define WAVE_FREQUENCY 1
#define SAMPLE_RATE 32
#define WAVE_MIN_VOLTAGE 1000
#define WAVE_MAX_VOLTAGE 3000
#define TIMER_PERIOD ((32768/256)/SAMPLE_RATE)


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
    CLEARBIT(T1CONbits.TON);
    T1CONbits.TCKPS = TCKPS_256;
    SETBIT(T1CONbits.TCS);
    CLEARBIT(T1CONbits.TGATE);
    T1CONbits.TSYNC = 0;
    PR1 = TIMER_PERIOD;
    TMR1 = 0x00;
    IPC0bits.T1IP = 0x01;
    CLEARBIT(IFS0bits.T1IF);
    SETBIT(IEC0bits.T1IE);
    SETBIT(T1CONbits.TON);
}


// interrupt service routine?
void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T1Interrupt(void)
{
    COUNT_IN ++;
    if(COUNT_IN == SAMPLE_RATE )
        COUNT_IN = 0;
    
    FLAG_WAIT_COMPLETE = START_NEXT_DAC;
    
    CLEARBIT(IFS0bits.T1IF);
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
        
        float voltage_float= sin(3.14159265358979323846*2*WAVE_FREQUENCY*step/SAMPLE_RATE) * (float)(WAVE_MAX_VOLTAGE -WAVE_MIN_VOLTAGE) / 2 + (float)(WAVE_MAX_VOLTAGE -WAVE_MIN_VOLTAGE) /2 ;
        
        uint16_t voltage = (uint16_t)voltage_float;


        dac_convert_milli_volt(voltage);

        FLAG_WAIT_COMPLETE = DAC_COMPLETE;
        
        TOGGLELED(LED1_PORT);
        
        
    }
}