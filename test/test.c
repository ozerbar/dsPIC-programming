#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define INTERNAL_CLOCK 0
#define EXTERNAL_CLOCK 1

uint16_t calc_timer_value(uint8_t clockSource, uint8_t preScaler, uint16_t periodMS)
{
    uint16_t timerValue = 0;


    if(clockSource == INTERNAL_CLOCK)
        timerValue = (uint16_t)( (float)(12800000) / (float)(preScaler) * (float)(periodMS) / 1000.0 );
    else
        timerValue = (uint16_t)( (float)(32768) / (float)(preScaler) * (float)(periodMS) / 1000.0 );
    
    return timerValue;

}

void main()
{
    uint16_t a_timer = calc_timer_value(INTERNAL_CLOCK, 64, 20);
    printf("timer1 %u\n", a_timer);

}