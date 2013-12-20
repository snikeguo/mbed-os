/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stddef.h>
#include "us_ticker_api.h"
#include "cmsis.h"
#include "PeripheralNames.h"

#define US_TICKER_TIMER      NRF_TIMER1
#define US_TICKER_TIMER_IRQn TIMER1_IRQn

int us_ticker_inited = 0;
volatile uint16_t overflow=0; //overflow value that forms the upper 16 bits of the counter
volatile uint32_t overflowShifted=0; //reduces number of times the overflow is shifted
void TIMER1_IRQHandler(void)
{
    if ((US_TICKER_TIMER->EVENTS_COMPARE[1] != 0) && 
       ((US_TICKER_TIMER->INTENSET & TIMER_INTENSET_COMPARE1_Msk) != 0))
    {
		US_TICKER_TIMER->EVENTS_COMPARE[1] = 0;
		overflow++;    
		overflowShifted = ((uint32_t)overflow<<16);
        US_TICKER_TIMER->CC[1] =0;//US_TICKER_TIMER->CC[1]+ 1;

    }
	if ((US_TICKER_TIMER->EVENTS_COMPARE[0] != 0) && 
    ((US_TICKER_TIMER->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0))
    {
        us_ticker_irq_handler();
    }
        
}

void us_ticker_init(void)
{
    if (us_ticker_inited)
    {
        return;
    }
    
    us_ticker_inited = 1;
    
    US_TICKER_TIMER->POWER = 0;
    US_TICKER_TIMER->POWER = 1;
    
    US_TICKER_TIMER->MODE = TIMER_MODE_MODE_Timer;
    
    US_TICKER_TIMER->PRESCALER = 4;
    US_TICKER_TIMER->BITMODE = TIMER_BITMODE_BITMODE_16Bit; 
	US_TICKER_TIMER->CC[1] = 0;
	US_TICKER_TIMER->INTENSET = TIMER_INTENSET_COMPARE1_Set << TIMER_INTENSET_COMPARE1_Pos;

    NVIC_EnableIRQ(US_TICKER_TIMER_IRQn);
    
    US_TICKER_TIMER->TASKS_START = 0x01;
}

uint32_t us_ticker_read() 
{
    if (!us_ticker_inited)
    {
        us_ticker_init();
    }
    uint32_t bufferedOverFlow   =         overflowShifted;
	
    US_TICKER_TIMER->TASKS_CAPTURE[2] = 1;
	__NOP(); //essential to register the capture
	if(overflowShifted!=bufferedOverFlow){
		bufferedOverFlow   =         overflowShifted;
		US_TICKER_TIMER->TASKS_CAPTURE[2] = 1;
	}
		
    return (bufferedOverFlow | US_TICKER_TIMER->CC[2]);
}

void us_ticker_set_interrupt(unsigned int timestamp)
{
    if (!us_ticker_inited)
    {
        us_ticker_init();
    }
    
    US_TICKER_TIMER->INTENSET |= TIMER_INTENSET_COMPARE0_Set << TIMER_INTENSET_COMPARE0_Pos;
    US_TICKER_TIMER->TASKS_CAPTURE[0] = 1;	
    US_TICKER_TIMER->CC[0] += timestamp;// * 16;
}

void us_ticker_disable_interrupt(void) 
{
    US_TICKER_TIMER->INTENCLR = TIMER_INTENCLR_COMPARE0_Clear << TIMER_INTENCLR_COMPARE0_Pos;
}
void us_ticker_clear_interrupt(void) 
{
    US_TICKER_TIMER->EVENTS_COMPARE[0] = 0;
}