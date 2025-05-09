/*
 * File:   main.c
 * Author: Rahul H H
 *
 * Created on March 17, 2025, 8:48 AM
 */


#include <xc.h>
#include "main.h"
#include "clcd.h"
#include "ds1307.h"
#include "External_eeprom.h"
#include "i2c.h"

// global declarations 
unsigned char flag =0;
unsigned char key;

unsigned char hr;
unsigned char min;
unsigned char sec;

unsigned char sec1, sec2;
unsigned char count=0;

unsigned char event_count =0;
unsigned char star_flag =0;


static void get_time(void)
{
	 hr = read_ds1307(HOUR_ADDR);
	 min = read_ds1307(MIN_ADDR);
	 sec = read_ds1307(SEC_ADDR);

	if (hr & 0x40)
	{
		time[0] = '0' + ((hr >> 4) & 0x01);
		time[1] = '0' + (hr & 0x0F);
	}
	else
	{
		time[0] = '0' + ((hr >> 4) & 0x03);
		time[1] = '0' + (hr & 0x0F);
	}
	time[2] = ':';
	time[3] = '0' + ((min >> 4) & 0x0F);
	time[4] = '0' + (min & 0x0F);
	time[5] = ':';
    time[6] = '0' + ((sec >> 4) & 0x0F);
	time[7] = '0' + (sec & 0x0F);
    
	sec2 = (((sec >> 4) & 0x0F) *10) + (sec & 0x0F); // reading the second
    
       if(sec1 != sec2)
           count++;
    
    sec1 = sec2;
    
	time[8] = '\0';
}

static void get_date(void)
{
	calender_reg[0] = read_ds1307(YEAR_ADDR);
	calender_reg[1] = read_ds1307(MONTH_ADDR);
	calender_reg[2] = read_ds1307(DATE_ADDR);
	calender_reg[3] = read_ds1307(DAY_ADDR);

//	date[6] = '';
//	date[7] = '0';
	date[6] = '0' + ((calender_reg[0] >> 4) & 0x0F);
	date[7] = '0' + (calender_reg[0] & 0x0F);
	date[5] = '-';
	date[3] = '0' + ((calender_reg[1] >> 4) & 0x0F);
	date[4] = '0' + (calender_reg[1] & 0x0F);
	date[2] = '-';
	date[0] = '0' + ((calender_reg[2] >> 4) & 0x0F);
	date[1] = '0' + (calender_reg[2] & 0x0F);
	date[8] = '\0';
}


void init_config(void)
{
	init_matrix_keypad();  // initializing matrix keypad
	init_clcd(); // initializing clcd
    init_i2c();
    init_ds1307();
    TRISB = TRISB & 0xfe;   // making led1 as output
    RB0 = 0;   // initializing led
    
}
unsigned char once_date;
extern unsigned char e_m_index;
extern unsigned char once_flag;
void main() 
{
    init_config(); // calling configuration function to configure matrix keypad, clcd peripherals

//    // Inside the logic where the event is triggered:
//    if (e_m_index == 0) {  // Once Event
//    once_date = calender_reg[3];  // Store the current date from calendar register
//    once_flag = 1;  // Mark the event as triggered
//    }   
    
    //once_date = calender_reg[3];
    while(1)
    {
        get_time(); // function call to get time
        get_date(); // function call to get date
        key = read_switches(1);  // edge triggering
        
        
        if(flag == DASHBOARD)  // default case
        { 
            dashboard();

        }
        else if(flag == MAIN_MENU)
        {
            main_menu();
            if(key == 4)
            {
                if((star_flag+ 2) == SUB_MENU1)  // 0 + 2 =2 
                {
                    flag = SUB_MENU1;
                }
                else if((star_flag + 2) == SUB_MENU2)  // 1 + 2 = 3 
                {
                    flag = SUB_MENU2;
                }
                else if((star_flag + 4) == SET_EVENT)   // 0 + 4 = 4 
                {
                    flag = SET_EVENT;
                }
                else if((star_flag + 4) == VIEW_EVENT)   // 1 + 4 = 5 
                {
                    flag = VIEW_EVENT;
                }
                else if((star_flag + 6) == SET_TIME)   // 0 + 6 = 6
                {
                    flag = SET_TIME;
                }
                else if((star_flag + 6) == SET_DATE)    // 1 + 6 = 7
                {
                    flag = SET_DATE;
                }
                
    
            }
        }
        else if(flag == SUB_MENU1)
        {
            sub_menu1();
        }
        else if(flag == SUB_MENU2)
        {
            sub_menu2();
        }
         else if(flag == SET_EVENT)
        {
            set_event();
        }
        else if(flag == VIEW_EVENT)
        {
            view_event();
        }
         else if(flag == SET_TIME)
        {
            set_time();
        }
        else if(flag == SET_DATE)
        {
            set_date();
        }
        
        //for(int i=0;i<500;i++);  // delay to slow down the process
    }
    
    return;
}
