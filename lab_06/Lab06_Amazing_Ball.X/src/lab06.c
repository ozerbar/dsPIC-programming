#include "lab06.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>
#include <stdint.h>

#include <math.h>

#include "types.h"
#include "lcd.h"
#include "led.h"


/*
 * Parameter
 */



/*
 * Common Definitions
 */
#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

#define TOUCHSCREEN_COOR_X 0
#define TOUCHSCREEN_COOR_Y 1

#define SERVO_X 0
#define SERVO_Y 1

#define INTERNAL_CLOCK 0
#define EXTERNAL_CLOCK 1

/*
 * Global Variables
 */
volatile uint8_t FLAG_START = 0;
volatile uint8_t FLAG_FIN = 1;
volatile uint8_t COOR_CHOOSE = TOUCHSCREEN_COOR_Y;
volatile uint32_t MISSED = 0;

/*
 * Helper Function
 */
//calculate timer period value
uint16_t calc_timer_value(uint8_t clockSource, uint16_t preScaler, float periodMS)
{
    uint16_t timerValue = 0;


    if(clockSource == INTERNAL_CLOCK)
    {
        timerValue = (uint16_t)( ((float)(12800000) / (float)(preScaler)) * (periodMS) / 1000 );
    }
        
    else
    {
        timerValue = (uint16_t)( ((float)(32768) / (float)(preScaler)) * (periodMS) / 1000 );
    }
    
    return timerValue;

}

/*
 * Timer Code
 */
/* Timer 2 for SERVO_X
 * Timer 3 for SERVO_Y 
 * Set Timer 1 for 100 Hz*/
//initalize and start timer 1, at 100hz, interval 10ms
void timer1_initialize_start()
{

    // configure timer
    CLEARBIT(T1CONbits.TON);
    CLEARBIT(T1CONbits.TCS);
    CLEARBIT(T1CONbits.TGATE);
    TMR1 = 0x00;
    T1CONbits.TCKPS = 0b11;     // prescaler 256
    PR1 = 500
    IPC0bits.T1IP = 0x01;
    CLEARBIT(IFS0bits.T1IF);
    SETBIT(IEC0bits.T1IE);
    SETBIT(T1CONbits.TON);
}

// interrupt service routine for timer 1
void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T1Interrupt(void)
{
    if(FLAG_FIN == 0) //still not finished, missed deadline
    {
        MISSED++;
    }
    else
    {
        FLAG_FIN = 0; // reset FLAG_FIN
        COOR_CHOOSE = 1 - COOR_CHOOSE;
        FLAG_START = 1;
    }
    
    
    CLEARBIT(IFS0bits.T1IF);
}









/*
 * Servo Code
 */
// initializes and configures the servos
void servo_initalize(uint8_t servoNum)
{
    //PWM mode, timer period 20ms
    // Setup Timer 2 to control servo X, OC8
    // Setup Timer 3 to control servo Y, OC7
    if(servoNum == SERVO_X)
    {
        CLEARBIT(T2CONbits.TON);        // Disable Timer
        CLEARBIT(T2CONbits.TCS);        // Select internal instruction cycle clock
        CLEARBIT(T2CONbits.TGATE);      // Disable Gated Timer mode
        TMR2 = 0x00;                    // Clear timer register
        T2CONbits.TCKPS = 0b10;         // Select 1:64 Prescaler
        CLEARBIT(IFS0bits.T2IF);        // Clear Timer2 interrupt status flag
        CLEARBIT(IEC0bits.T2IE);        // Disable Timer2 interrupt enable control bit
        PR2 = calc_timer_value(INTERNAL_CLOCK, 64, 20);    // Set timer period 20 ms:
        CLEARBIT(TRISDbits.TRISD7); // Set OC8 as output
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
        PR3 = calc_timer_value(INTERNAL_CLOCK, 64, 20);
        CLEARBIT(TRISDbits.TRISD6); // Set OC7 as output
    }
}

// sets the duty cycle of the servo
void servo_set_duty_cycle(uint8_t servoNum, uint16_t dutyCycle)
{
    if(servoNum == SERVO_X)
    {

        OC8R = dutyCycle;       // Set the initial duty cycle        
        OC8RS = dutyCycle;      // Load OCRS: next pwm duty cycle        
        OC8CON = 0x0006;        // Set OC8: PWM, no fault check, Timer2
        SETBIT(T2CONbits.TON);  // Turn Timer 2 on        
    }
    else
    {
        OC7R = dutyCycle;       // Set the initial duty cycle
        OC7RS = dutyCycle;      // Load OCRS: next pwm duty cycle
        OC7CON = 0x000e;        // Set OC8: PWM, no fault check, Timer3
        SETBIT(T3CONbits.TON);  // Turn Timer 3 on
    }
}

// helper function
// actuate servo X and Y to rotate designated angle
void servo_rotate_X_and_Y(float servoXPauseMS, float servoYPauseMS)
{

    servo_initalize(0);
    servo_set_duty_cycle(0, calc_timer_value(INTERNAL_CLOCK, 64, 20.0-servoXPauseMS));
    servo_initalize(1);
    servo_set_duty_cycle(1, calc_timer_value(INTERNAL_CLOCK, 64, 20.0-servoYPauseMS));
    
}



/*
 * Touch screen code
 */
// initializes and configures the touchscreen
void touchscreen_initalize()
{

    // Set up the I/O pins E1, E2, E3 to be output pins
    CLEARBIT(TRISEbits.TRISE1); // I/O pin set to output
    CLEARBIT(TRISEbits.TRISE2); // I/O pin set to output
    CLEARBIT(TRISEbits.TRISE3); // I/O pin set to output
    Nop();

}

// changes the dimension in which the touchscreen reads from
void touchscreen_config_direction(uint8_t direction)
{
    
    if(direction==TOUCHSCREEN_COOR_X)
    {
        // Set up the I/O pins E1, E2, E3 so that the 
        // touchscreen X-coordinate pin connects to the ADC
        CLEARBIT(PORTEbits.RE1);
        Nop();
        SETBIT(PORTEbits.RE2);
        Nop();
        SETBIT(PORTEbits.RE3);
        Nop();
    }
    else
    {
        // touchscreen Y-coordinate pin connects to the ADC
        SETBIT(PORTEbits.RE1);
        Nop();
        CLEARBIT(PORTEbits.RE2);
        Nop();
        CLEARBIT(PORTEbits.RE3);
        Nop();

    }
   
    
    // Disable ADC
    CLEARBIT(AD1CON1bits.ADON);
    //initialize PIN
    

    if(direction==TOUCHSCREEN_COOR_X)
    {
        SETBIT(TRISBbits.TRISB15);
        CLEARBIT(AD1PCFGLbits.PCFG15); // Set AD1 AN15 input pin as analog
    }
    else
    {
        SETBIT(TRISBbits.TRISB9); 
        CLEARBIT(AD1PCFGLbits.PCFG9); // Set AD1 AN9 input pin as analog
    }

    //Configure AD1CON1
    CLEARBIT(AD1CON1bits.AD12B);
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
        AD1CHS0bits.CH0SA = 0b01111;
    }
    else
    {
        AD1CHS0bits.CH0SA = 0b01001;
    }

}

// reads the current ball position
uint16_t touchscreen_read_result() 
{

    SETBIT(AD1CON1bits.SAMP);
    
    while(!AD1CON1bits.DONE);       // Start to sample
    
    CLEARBIT(AD1CON1bits.DONE);     // Wait for conversion to finish

    uint16_t result = ADC1BUF0; 

    return result;                // Return sample
}


/*
 * PD Controller
 */



/*
 * Butterworth Filter N=1, Cutoff 3 Hz, sampling @ 50 Hz
 */
uint16_t SINGLE_FILTER(uint16_t POS, uint16_t POS_1, uint16_t FILT_1)
{
    float res_f = 0.1602*(float)(POS+POS_1) + 0.6796*(float)(FILT_1);
    return (uint16_t)res_f
}




/*
 * main loop
 */
void main_loop()
{
    // print assignment information
    lcd_printf("Lab06: Amazing Ball");
    lcd_locate(0, 1);
    lcd_printf("Group: GroupName");
    lcd_locate(0, 2);

    uint16_t X_POS_0 = 0;
    uint16_t X_POS_1 = 0;
    uint16_t Y_POS_0 = 0;
    uint16_t Y_POS_1 = 0;
    
    uint16_t X_FILT_0 = 0;
    uint16_t X_FILT_1 = 0;
    uint16_t Y_FILT_0 = 0;
    uint16_t Y_FILT_1 = 0;

    uint16_t X_GOAL = 0;
    uint16_t Y_GOAL = 0;

    uint16_t X_ERR_0 = 0;
    uint16_t X_ERR_1 = 0;
    uint16_t Y_ERR_0 = 0;
    uint16_t Y_ERR_1 = 0;

    uint16_t resTemp = 0;

    touchscreen_initalize();
    touchscreen_config_direction(TOUCHSCREEN_COOR_X);

    
    
    while(TRUE) {
        while(FLAG_START == 0);
        FLAG_START = 0;
        if(COOR_CHOOSE == TOUCHSCREEN_COOR_X)
        {
            X_POS_1 = X_POS_0;
            X_POS_0 = touchscreen_read_result();
            touchscreen_config_direction(TOUCHSCREEN_COOR_Y);
            X_FILT_1 = X_FILT_0;
            X_FILT_0 = SINGLE_FILTER(X_POS_0, X_POS_1, X_FILT_1);
            X_GOAL = 400;
            X_ERR_1 = X_ERR_0;
            X_ERR_0 = X_FILT_0 - X_GOAL;




            
        }
        else
        {
            Y_POS_1 = Y_POS_0;
            Y_POS_0 = touchscreen_read_result();
            touchscreen_config_direction(TOUCHSCREEN_COOR_X);
            Y_FILT_1 = Y_FILT_0;
            Y_FILT_0 = SINGLE_FILTER(Y_POS_0, Y_POS_1, Y_FILT_1);
            



        }

        COOR_CHOOSE = 1 - COOR_CHOOSE;
        FLAG_FIN = 1;
        
    }
}
