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

#define INTERNAL_CLOCK 0
#define EXTERNAL_CLOCK 1


volatile uint8_t FLAG_5_SECOND = 0;
volatile uint8_t FLAG_10_MS = 0;

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
 * touch screen code
 */

// initializes and configures the touchscreen
void touchscreen_initalize()
{
    // Set up the I/O pins E1, E2, E3 to be output pins
    CLEARBIT(TRISEbits.TRISE1); // I/O pin set to output
    CLEARBIT(TRISEbits.TRISE2); // I/O pin set to output
    CLEARBIT(TRISEbits.TRISE3); // I/O pin set to output
    Nop();

    //setting E1=1, E2=1, E3=0 puts touchscreen into the standby mode
    //SETBIT(PORTEbits.RE1);
    //SETBIT(PORTEbits.RE2);
    //CLEARBIT(PORTEbits.RE3);
    //Nop();
}

// changes the dimension in which the touchscreen reads from
void touchscreen_config_direction(uint8_t direction)
{
    
    if(direction==TOUCHSCREEN_COOR_X)
    {
        // Set up the I/O pins E1, E2, E3 so that the 
        // touchscreen X-coordinate pin connects to the ADC
        CLEARBIT(PORTEbits.RE1);
        SETBIT(PORTEbits.RE2);
        SETBIT(PORTEbits.RE3);
        Nop();
        
        lcd_locate(0, 4);
        lcd_printf("x coor");
    }
    else
    {
        // touchscreen Y-coordinate pin connects to the ADC
        SETBIT(PORTEbits.RE1);
        CLEARBIT(PORTEbits.RE2);
        CLEARBIT(PORTEbits.RE3);
        Nop();
        
        lcd_locate(0, 5);
        lcd_printf("y coor");
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
 * servo code
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


//initalize and start timer 1
void timer1_initialize_start(float intervalMS)
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
    T1CONbits.TCKPS = 0b11;     // prescaler 256
    SETBIT(T1CONbits.TCS);
    CLEARBIT(T1CONbits.TGATE);
    T1CONbits.TSYNC = 0;
    PR1 = calc_timer_value(EXTERNAL_CLOCK, 256, 5000);
    TMR1 = 0x00;
    IPC0bits.T1IP = 0x01;
    CLEARBIT(IFS0bits.T1IF);
    SETBIT(IEC0bits.T1IE);
    SETBIT(T1CONbits.TON);
}

// interrupt service routine for timer 1
void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T1Interrupt(void)
{
    FLAG_5_SECOND = 1;
    CLEARBIT(IFS0bits.T1IF);
}

//initalize and start timer 4
void timer4_initialize_start(float intervalMS)
{
    CLEARBIT(T4CONbits.TON);    
    CLEARBIT(T4CONbits.TCS);        // Select internal instruction cycle clock
    CLEARBIT(T4CONbits.TGATE);
    TMR4 = 0x00;
    T4CONbits.TCKPS = 0b11;         // prescaler 256
    PR4 = calc_timer_value(INTERNAL_CLOCK, 256, intervalMS);
    IPC6bits.T4IP = 0x01;
    CLEARBIT(IFS1bits.T4IF);
    SETBIT(IEC1bits.T4IE);
    SETBIT(T4CONbits.TON);

    FLAG_10_MS = 0;
}

// interrupt service routine for timer 1
void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T4Interrupt(void)
{
    FLAG_10_MS = 1;
    CLEARBIT(IFS1bits.T4IF);
}

void printLocationTouchScreen()
{
    uint8_t i=0;
    for(i=0;i<4;i++)
    {
        timer4_initialize_start(500);
        while( FLAG_10_MS == 0 );
    }
    
    touchscreen_config_direction(TOUCHSCREEN_COOR_X);
    timer4_initialize_start(20);
    while( FLAG_10_MS == 0 );
    uint16_t xLocation = touchscreen_read_result();

    touchscreen_config_direction(TOUCHSCREEN_COOR_Y);
    timer4_initialize_start(20);
    while( FLAG_10_MS == 0 );
    uint16_t yLocation = touchscreen_read_result();

    lcd_locate(0, 3);
    //lcd_printf("X: %03u", xLocation);
    lcd_printf("X/Y: %03u/%03u", xLocation, yLocation);

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
    lcd_printf("Group: someone");
    
    // initialize touchscreen
    touchscreen_initalize();
    // initialize servos
    timer1_initialize_start((float)5000);

    while(TRUE) {
        
        while(FLAG_5_SECOND == 0);
        FLAG_5_SECOND = 0;
        servo_rotate_X_and_Y(1.1, 1.1);
        printLocationTouchScreen();
        
        while(FLAG_5_SECOND == 0);
        FLAG_5_SECOND = 0;
        servo_rotate_X_and_Y(1.1, 1.9);
        printLocationTouchScreen();
        
        while(FLAG_5_SECOND == 0);
        FLAG_5_SECOND = 0;
        servo_rotate_X_and_Y(2.1, 1.9);
        printLocationTouchScreen();
        
        while(FLAG_5_SECOND == 0);
        FLAG_5_SECOND = 0;
        servo_rotate_X_and_Y(2.1, 1.1);
        printLocationTouchScreen();
        
        
    }
}
