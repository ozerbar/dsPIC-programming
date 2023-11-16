#include "lab03.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"
#include "lcd.h"
#include "led.h"

/*
 * DAC code
 */

#define DAC_CS_TRIS TRISDbits.TRISD8
#define DAC_SDI_TRIS TRISBbits.TRISB10
#define DAC_SCK_TRIS TRISBbits.TRISB11
#define DAC_LDAC_TRIS TRISBbits.TRISB13
    
#define DAC_CS_PORT PORTDbits.RD8
#define DAC_SDI_PORT PORTBbits.RB10
#define DAC_SCK_PORT PORTBbits.RB11
#define DAC_LDAC_PORT PORTBbits.RB13

#define DAC_SDI_AD1CFG AD1PCFGLbits.PCFG10
#define DAC_SCK_AD1CFG AD1PCFGLbits.PCFG11
#define DAC_LDAC_AD1CFG AD1PCFGLbits.PCFG13

#define DAC_SDI_AD2CFG AD2PCFGLbits.PCFG10
#define DAC_SCK_AD2CFG AD2PCFGLbits.PCFG11
#define DAC_LDAC_AD2CFG AD2PCFGLbits.PCFG13

volatile uint8_t FLAG_WAIT_COMPLETE =0;


void dac_initialize()
{
    // set AN10, AN11 AN13 to digital mode
    // this means AN10 will become RB10, AN11->RB11, AN13->RB13
    // see datasheet 11.3
    SETBIT(DAC_SDI_AD1CFG);
    SETBIT(DAC_SCK_AD1CFG);
    SETBIT(DAC_LDAC_AD1CFG);
    SETBIT(DAC_SDI_AD2CFG);
    SETBIT(DAC_SCK_AD2CFG);
    SETBIT(DAC_LDAC_AD2CFG);
    Nop();
    // set RD8, RB10, RB11, RB13 as output pins
    CLEARBIT(DAC_CS_TRIS);
    CLEARBIT(DAC_SDI_TRIS);
    CLEARBIT(DAC_SCK_TRIS);
    CLEARBIT(DAC_LDAC_TRIS);
    Nop();
    // set default state: CS=??, SCK=??, SDI=??, LDAC=??
    SETBIT(DAC_CS_PORT);
    Nop();
    SETBIT(DAC_SCK_PORT);
    Nop();
    CLEARBIT(DAC_SDI_PORT);
    Nop();
    SETBIT(DAC_LDAC_PORT);
    Nop();
}

void dac_output1(uint8_t data[])
{
    CLEARBIT(DAC_CS_PORT);
    Nop();

    uint8_t i = 0;
    for(i=0 ; i<16 ; i++)
    {
        if(data[i]==0)
            CLEARBIT(DAC_SDI_PORT);
        else
            SETBIT(DAC_SDI_PORT);
        Nop();
        CLEARBIT(DAC_SCK_PORT);
        Nop();
        Nop();
        SETBIT(DAC_SCK_PORT);
        Nop();

    }

    SETBIT(DAC_CS_PORT);
    Nop();
    CLEARBIT(DAC_SDI_PORT);
    Nop();
    CLEARBIT(DAC_LDAC_PORT);
    Nop();
    SETBIT(DAC_LDAC_PORT);
    Nop();

}

/*
 * Timer code
 */

#define FCY_EXT   32768UL

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

void timer_initialize_start(int interval)
{
    // Enable RTC Oscillator -> this effectively does OSCCONbits.LPOSCEN = 1
    // but the OSCCON register is lock protected. That means you would have to 
    // write a specific sequence of numbers to the register OSCCONL. After that 
    // the write access to OSCCONL will be enabled for one instruction cycle.
    // The function __builtin_write_OSCCONL(val) does the unlocking sequence and
    // afterwards writes the value val to that register. (OSCCONL represents the
    // lower 8 bits of the register OSCCON)
    __builtin_write_OSCCONL(OSCCONL | 2);
    // configure timer
    CLEARBIT(T1CONbits.TON);
    T1CONbits.TCKPS = 0b11;
    SETBIT(T1CONbits.TCS);
    CLEARBIT(T1CONbits.TGATE);
    T1CONbits.TSYNC = 0;
    PR1 = interval;
    TMR1 = 0x00;
    IPC0bits.T1IP = 0x01;
    CLEARBIT(IFS0bits.T1IF);
    SETBIT(IEC0bits.T1IE);
    SETBIT(T1CONbits.TON);
}

// interrupt service routine?
void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T1Interrupt(void)
{
    FLAG_WAIT_COMPLETE = 1;
    CLEARBIT(IFS0bits.T1IF);
}




/*
 * main loop
 */

void main_loop()
{
    // print assignment information
    lcd_printf("Lab03: DAC");
    lcd_locate(0, 1);
    lcd_printf("Group: 2-2");
    CLEARLED(LED1_PORT);
    
    //dac_initialize();
    uint8_t data1[16] = {0,0,0,1,0,0,1,1,1,1,1,0,1,0,0,0};
    uint8_t data2[16] = {0,0,0,1,1,0,0,1,1,1,0,0,0,1,0,0};
    uint8_t data3[16] = {0,0,0,1,1,1,0,1,1,0,1,0,1,1,0,0};
    //dac_output(data);
    while(TRUE)
    {
        dac_output(data1);
        timer_initialize_start(128);
        while(FLAG_WAIT_COMPLETE == 0);
        FLAG_WAIT_COMPLETE = 0;
        
        dac_output(data2);
        timer_initialize_start(512);
        while(FLAG_WAIT_COMPLETE == 0);
        FLAG_WAIT_COMPLETE = 0;
        
        dac_output(data3);
        timer_initialize_start(256);
        while(FLAG_WAIT_COMPLETE == 0);
        FLAG_WAIT_COMPLETE = 0;

        TOGGLELED(LED1_PORT);


        // main loop code
    }
}
