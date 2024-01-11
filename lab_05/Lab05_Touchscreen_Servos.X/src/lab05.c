#include "lab05.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"
#include "lcd.h"
#include "led.h"

/*
 * PWM code
 */

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

#define PWM_MIN_US 1000
#define PWM_MID_US 1500
#define PWM_MAX_US 2000
#define PWM_CYC_US 20000

#define TOUCHSCREEN_COOR_X 0
#define TOUCHSCREEN_COOR_Y 1
#define SERVO_X 0
#define SERVO_Y 1

/*
 * touch screen code
 */

void touchscreen_initalize()
{
    // Set up the I/O pins E1, E2, E3 to be output pins
    CLEARBIT(TRISEbits.TRISE1); // I/O pin set to output
    CLEARBIT(TRISEbits.TRISE2); // I/O pin set to output
    CLEARBIT(TRISEbits.TRISE3); // I/O pin set to output

    //For example, setting E1=1, E2=1, E3=0 puts touchscreen into the standby mode
    SETBIT(PORTEbits.RE1);
    SETBIT(PORTEbits.RE2);
    CLEARBIT(PORTEbits.RE3);
}


/*
touchscreen’s X-coordinate: AN15 
touchscreen’s Y-coordinate: AN9. There is also a circuit
designed to control the touchscreen measurement. This circuit is controlled through three I/O pins
E1, E2, E3. For example, setting E1=1, E2=1, E3=0 puts touchscreen into the standby mode. */

void touchscreen_config_direction(uint8_t direction)
{
    
    if(direction==TOUCHSCREEN_COOR_X)
    {
        // Set up the I/O pins E1, E2, E3 so that the touchscreen X-coordinate pin
        // connects to the ADC
        CLEARBIT(PORTEbits.RE1);
        SETBIT(PORTEbits.RE2);
        SETBIT(PORTEbits.RE3);
    }
    else
    {
        // Y connects to the ADC
        SETBIT(PORTEbits.RE1);
        CLEARBIT(PORTEbits.RE2);
        CLEARBIT(PORTEbits.RE3);
    }
   
    
    // Disable ADC
    CLEARBIT(AD1CON1bits.ADON);
    //initialize PIN
    SETBIT(TRISEbits.TRISE8);
    // Set TRISE RE8 to input

    if(direction==TOUCHSCREEN_COOR_X)
    {
        CLEARBIT(AD1PCFGHbits.PCFG15); // Set AD1 AN15 input pin as analog
    }
    else
    {
        CLEARBIT(AD1PCFGHbits.PCFG9); // Set AD1 AN9 input pin as analog
    }

    //Configure AD1CON1
    CLEARBIT(AD1CON1bits.AD12B)
    // Set 10b Operation Mode
    AD1CON1bits.FORM = 0;
    // Set integer output
    AD1CON1bits.SSRC = 0x7;
    // Set automatic conversion
    // Configure AD1CON2
    AD1CON2 = 0;
    // Not using scanning sampling
    //Configure AD1CON3
    CLEARBIT(AD1CON3bits.ADRC);
    // Internal clock source
    AD1CON3bits.SAMC = 0x1F;
    // Sample-to-conversion clock = 31Tad
    AD1CON3bits.ADCS = 0x2;
    // Tad = 3Tcy (Time cycles)
    // Leave AD1CON4 at its default value
    // Enable ADC
    SETBIT(AD1CON1bits.ADON);

    if(direction==TOUCHSCREEN_COOR_X)
    {
        AD1CHS0bits.CH0SA = 0x00f;
    }
    else
    {
        AD1CHS0bits.CH0SA = 0x009;
    }

}

uint16_t touchscreen_read_result() //should not be read, there's return value
{
    //AD1CHS0bits.CH0SA = 0x014;
    SETBIT(AD1CON1bits.SAMP);
    while(!AD1CON1bits.DONE);       // Start to sample
    CLEARBIT(AD1CON1bits.DONE);     // Wait for conversion to finish
    return ADC1BUF0;                // Return sample
}


void servo_initalize(uint8_t servoNum)
{
    //The following code sets up OC8 to work in PWM mode and be controlled by Timer 2. When
    //operating, the OC8 pin will be set to high for 5ms every 40ms.
    // Setup Timer 2 to control servo X, OC8
    // Setup Timer 3 to control servo Y, OC7
    if(servoNum == SERVO_X)
    {
        CLEARBIT(T2CONbits.TON);
        CLEARBIT(T2CONbits.TCS);
        CLEARBIT(T2CONbits.TGATE);
        TMR2 = 0x00;
        T2CONbits.TCKPS = 0b10;
        CLEARBIT(IFS0bits.T2IF);
        CLEARBIT(IEC0bits.T2IE);
        PR2 = 4000;
        // Disable Timer
        // Select internal instruction cycle clock
        // Disable Gated Timer mode
        // Clear timer register
        // Select 1:64 Prescaler
        // Clear Timer2 interrupt status flag
        // Disable Timer2 interrupt enable control bit
        // Set timer period 40 ms:
        // 8000 = 40*10^-3 * 12.8*10^6 * 1/64
        // Setup OC8
        CLEARBIT(TRISDbits.TRISD7); // Set OC8 as output

        // Turn Timer 2 on
    }
    else
    {
        CLEARBIT(T3CONbits.TON);
        CLEARBIT(T3CONbits.TCS);
        CLEARBIT(T3CONbits.TGATE);
        TMR3 = 0x00;
        T3CONbits.TCKPS = 0b10;
        CLEARBIT(IFS0bits.T3IF);
        CLEARBIT(IEC0bits.T3IE);
        PR3 = 4000;
        // Disable Timer
        // Select internal instruction cycle clock
        // Disable Gated Timer mode
        // Clear timer register
        // Select 1:64 Prescaler
        // Clear Timer2 interrupt status flag
        // Disable Timer2 interrupt enable control bit
        // Set timer period 40 ms:
        // 8000 = 40*10^-3 * 12.8*10^6 * 1/64
        // Setup OC8
        CLEARBIT(TRISDbits.TRISD6); // Set OC7 as output

        // Turn Timer 2 on
    }

    
    
    

}

void servo_set_duty_cycle(uint8_t servoNum, uint16_t dutyCycle)
{
    if(servoNum == SERVO_X)
    {

        OC8R = dutyCycle;
        // Set the initial duty cycle to 5 ms
        OC8RS = dutyCycle;
        // Load OCRS: next pwm duty cycle
        OC8CON = 0x0006;
        // Set OC8: PWM, no fault check, Timer2
        SETBIT(T2CONbits.TON);
        // Turn Timer 2 on
    }
    else
    {

        OC7R = dutyCycle;
        // Set the initial duty cycle to 5 ms
        OC7RS = dutyCycle;
        // Load OCRS: next pwm duty cycle
        OC7CON = 0x000e;
        // Set OC8: PWM, no fault check, Timer2
        SETBIT(T3CONbits.TON);
        // Turn Timer 2 on
    }
}




/*
 * main loop
 */

void main_loop()
{
    // print assignment information
    lcd_printf("Lab05: Touchscreen &\r\n");
    lcd_printf("       Servos");
    lcd_locate(0, 2);
    lcd_printf("Group: GroupName");
    
    // initialize touchscreen
    
    // initialize servos
    
    while(TRUE) {
        
        servo_initalize(0);
        servo_set_duty_cycle(0, 4000-180)
        servo_initalize(1);
        servo_set_duty_cycle(1, 4000-180)
        for(uint16_t=0;i<100000;i++)
        {
            Nop();
        }
        servo_initalize(0);
        servo_set_duty_cycle(0, 4000-180)
        servo_initalize(1);
        servo_set_duty_cycle(1, 4000-420)
        for(uint16_t=0;i<100000;i++)
        {
            Nop();
        }
        
    }
}
