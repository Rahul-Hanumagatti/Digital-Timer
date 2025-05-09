/*
 * File:   sub.c
 * Author: Rahul H H
 *
 * Created on March 17, 2025, 9:06 AM
 */


#include <xc.h>
#include "main.h"
#include "External_eeprom.h"
#include "ds1307.h"
#include "i2c.h"
#include "clcd.h"
#include<string.h>


extern unsigned char flag;
extern unsigned char key;

extern unsigned char hr;
extern unsigned char min;
extern unsigned char sec;

extern unsigned char count;

extern unsigned char sec1;
extern unsigned char sec2;

extern unsigned char event_count;
extern unsigned char star_flag;


char setf = 0;

signed char fflag = -1;

unsigned char sethr;
unsigned char setmin;
unsigned char setsec;

unsigned char dy;
unsigned char dy2;
unsigned char dy3;
unsigned char dy4;

unsigned char temphr;
unsigned char tempmin;
unsigned char tempsec;
unsigned char ap_index = 0;

char datef = 0;
// for felid change
signed char sflag = -1;
unsigned char dat;
unsigned char month;
unsigned char year;

// to update date
unsigned char tempdat;
unsigned char tempmonth;
unsigned char tempyear;

// delay for set date
unsigned char d;
unsigned char d2;
unsigned char d3;
unsigned char d4;
unsigned char d_index = 0;
unsigned char day_index = 0;


// variables for set event
unsigned char m_index = 0;
unsigned eflag = 0;

unsigned char ed;
unsigned char ed2;
unsigned char ed3;
unsigned char ed4;

unsigned char lap = 0;
int i = 0;
char aflag = 0;

extern unsigned char once_date;
unsigned char next_date;

unsigned char once_flag=0, week_flag=0;
unsigned char e_hr, e_min, e_ap_index, e_m_index, event_triggered_flag=0;
unsigned char prev_once =0, curr_once =0;
unsigned char d=0;
unsigned static char on=1;

void dashboard(void) {
    if (key == 4) {
        flag = MAIN_MENU;
    }

    if (count >=0 && count < 5) {
//        clcd_print("                ", LINE1(0));  // clear line 1
//        clcd_print("                ", LINE2(0));  // clear line 2
        
         clcd_print("DATE ", LINE1(0)); // default 1 st line will print date i.e, dashboard
        clcd_print("TIME ", LINE2(0)); // default 2 nd line will print time i.e, dashboard

        clcd_print(date, LINE1(5)); // date

        clcd_print(time, LINE2(5)); // printing time
        if (hr & 0x40) {
            if (hr & 0x20) {
                clcd_print("PM      ", LINE2(14));
            } else {
                clcd_print("AM      ", LINE2(14));
            }
        }

    }else if (count < 7) {
    clcd_print("TIME ", LINE1(0)); // Line 1 label
    clcd_print("EVENT ", LINE2(0)); // Line 2 label

    clcd_print(time, LINE1(5)); // Display current time

    // Display AM/PM based on RTC
    if (hr & 0x40) {
        if (hr & 0x20) {
            clcd_print("PM  ", LINE1(14));
        } else {
            clcd_print("AM  ", LINE1(14));
        }
    }

    // Get current time as integers
    char hour = (time[0] - '0') * 10 + (time[1] - '0');
    char min = (time[3] - '0') * 10 + (time[4] - '0');
    unsigned char current_ap_index = (hr & 0x20) ? 1 : 0; // 0 = AM, 1 = PM

    unsigned char store[3];
    unsigned char led_flag = 0;
    unsigned char event_found = 0;
    unsigned char next_event_index = 0xFF; // Invalid index initially
     
    if (lap == 0) {
        clcd_print("No Events          ", LINE2(6)); // No alarms
        RB0 = 0; // LED off
    } else {
        // Find the next upcoming event
        for (int j = 0; j < lap; j++) {
            store[0] = read_external_eeprom(j * 3);
            store[1] = read_external_eeprom(j * 3 + 1);
            store[2] = read_external_eeprom(j * 3 + 2);

            e_hr = store[0];
            e_min = store[1];
            e_ap_index = store[2] / 10;

            next_date = calender_reg[3];  // reading the day 

            // Only consider future events
            if (e_ap_index > current_ap_index ||
                (e_ap_index == current_ap_index && (e_hr > hour || (e_hr == hour && e_min >= min)))) {

                if (!event_found ||
                    e_ap_index < store[2] / 10 ||
                    (e_ap_index == store[2] / 10 && (e_hr < store[0] || (e_hr == store[0] && e_min < store[1])))) {

                    next_event_index = j;
                    event_found = 1;
                }
            }
        }

        // If no future event, cycle to earliest one
        if (!event_found) {
            unsigned char min_hr = 99, min_min = 99;
            for (int j = 0; j < lap; j++) {
                store[0] = read_external_eeprom(j * 3);
                store[1] = read_external_eeprom(j * 3 + 1);
                store[2] = read_external_eeprom(j * 3 + 2);

                if (store[0] < min_hr || (store[0] == min_hr && store[1] < min_min)) {
                    min_hr = store[0];
                    min_min = store[1];
                    next_event_index = j;
                }
            }
        }

        // Show the upcoming event on the display
        if (next_event_index != 0xFF) {
            store[0] = read_external_eeprom(next_event_index * 3);
            store[1] = read_external_eeprom(next_event_index * 3 + 1);
            store[2] = read_external_eeprom(next_event_index * 3 + 2);

            e_hr = store[0];
            e_min = store[1];
            e_ap_index = store[2] / 10;
            e_m_index = store[2] % 10;

            clcd_putch((e_hr / 10) + '0', LINE2(6));
            clcd_putch((e_hr % 10) + '0', LINE2(7));
            clcd_putch(':', LINE2(8));
            clcd_putch((e_min / 10) + '0', LINE2(9));
            clcd_putch((e_min % 10) + '0', LINE2(10));
            clcd_putch(' ', LINE2(11));
            clcd_print((e_ap_index == 1) ? "PM   " : "AM   ", LINE2(12));
            clcd_putch(' ', LINE2(14));

            if (e_m_index == 0) 
            {
                if(on ==1)
                {
                    d = day_index;
                    on =0;
                }
                once_flag =1;
                 clcd_print("O ", LINE2(15)); // Once event
            }
            else if (e_m_index == 1) 
                clcd_print("D ", LINE2(15)); // Daily event
            else if (e_m_index == 2) 
                clcd_print("W ", LINE2(15)); // Weekly event

            // Trigger alarm logic
            led_flag = 0;
            if (hour == e_hr && min == e_min && current_ap_index == e_ap_index) {

                if (e_m_index == 0) {  // Once Alarm
                   // if (once_flag == 1) {  // If it hasn't been triggered before
                        if (d == day_index) {  // If today's date matches the stored date
                            led_flag = 1;
                            //once_flag = 0;  // Mark it as triggered
                        }
                    //}
                }
                else if (e_m_index == 1) {  // Daily Alarm
                    led_flag = 1;
                }
                else if (e_m_index == 2) {  // Weekly Alarm
                    if (calender_reg[4] == week_flag) {
                        led_flag = 1;
                    }
                }
            }

            if (led_flag == 1)
                RB0 = 1;  // Trigger the alarm (LED on)
            else
                RB0 = 0;  // Turn off the alarm (LED off)
        }
    }
}else if (count > 7) {
        CLEAR_DISP_SCREEN;
        count = 0;
    }




}

void main_menu(void) {

    if (star_flag) {
        clcd_print("  ", LINE1(0));
        clcd_print("->", LINE2(0));
    } else {
        clcd_print("->", LINE1(0));
        clcd_print("  ", LINE2(0));
    }

    if (key == 2) // scroll down the arrow 
    {
        if (star_flag == 0)
            star_flag = 1;
    } else if (key == 1) // scroll up
    {
        if (star_flag == 1)
            star_flag = 0;
    }


    if (key == 5) {
        flag = DASHBOARD;
        CLEAR_DISP_SCREEN;
    } else if (key == 4) {
        //CLEAR_DISP_SCREEN;
    }

    clcd_print("SET/VIEW EVENT", LINE1(2)); // default 1 st line will print date i.e, dashboard
    clcd_print("SET TIME/DATE ", LINE2(2)); // default 2 nd line will print time i.e, dashboard





}

void sub_menu1(void) {
    if (key == 5) {
        flag = MAIN_MENU;
        CLEAR_DISP_SCREEN;
    }

    if (star_flag) {
        clcd_print("  ", LINE1(0));
        clcd_print("->", LINE2(0));
    } else {
        clcd_print("->", LINE1(0));
        clcd_print("  ", LINE2(0));
    }

    if (star_flag) {
        if (key == 4)
            flag = VIEW_EVENT;
    } else {
        if (key == 4)
            flag = SET_EVENT;
    }

    if (key == 2) // scroll down the arrow 
    {
        if (star_flag == 0)
            star_flag = 1;
    } else if (key == 1) // scroll up
    {
        if (star_flag == 1)
            star_flag = 0;
    }


    clcd_print("SET EVENT           ", LINE1(2)); // default 1 st line will print date i.e, dashboard
    clcd_print("VIEW EVENT         ", LINE2(2)); // default 2 nd line will print time i.e, dashboard
}

void sub_menu2(void) {
    if (key == 5) {
        flag = MAIN_MENU;
        CLEAR_DISP_SCREEN;
    }

    if (star_flag) {
        clcd_print("  ", LINE1(0));
        clcd_print("->", LINE2(0));
    } else {
        clcd_print("->", LINE1(0));
        clcd_print("  ", LINE2(0));
    }

    if (star_flag) {
        if (key == 4)
            flag = SET_DATE;
    } else {
        if (key == 4) {
            hr = ((time[0] - '0') *10) + time[1] - '0';

            min = ((time[3] - '0') *10) + time[4] - '0';

            sec = ((time[6] - '0') *10) + time[7] - '0';
            flag = SET_TIME;
        }

    }

    if (key == 2) // scroll down the arrow 
    {
        if (star_flag == 0)
            star_flag = 1;
    } else if (key == 1) // scroll up
    {
        if (star_flag == 1)
            star_flag = 0;
    }


    clcd_print("SET TIME         ", LINE1(2)); // default 1 st line will print date i.e, dashboard
    clcd_print("SET DATE         ", LINE2(2)); // default 2 nd line will print time i.e, dashboard
}

void set_event(void) {
    if (key == 5) {
        setf = 0;
        flag = SUB_MENU1;
    }

    clcd_print(" HH:MM:A/P:M   ", LINE1(0));

    if (setf == 0) // Initialize time only once
    {
        sethr = ((time[0] - '0') * 10) + (time[1] - '0');
        setmin = ((time[3] - '0') * 10) + (time[4] - '0');
        setf = 1;
    }

    clcd_putch(' ', LINE2(0));

    // **Blinking Hours**
    if (eflag == 0) {
        if (ed++ >= 200) ed = 0;
        if (ed < 100) {
            clcd_putch(' ', LINE2(1));
            clcd_putch(' ', LINE2(2));
        } else {
            clcd_putch(sethr / 10 + 48, LINE2(1));
            clcd_putch(sethr % 10 + 48, LINE2(2));
        }
    } else {
        clcd_putch(sethr / 10 + 48, LINE2(1));
        clcd_putch(sethr % 10 + 48, LINE2(2));
    }

    clcd_putch(':', LINE2(3));

    // **Blinking Minutes**
    if (eflag == 1) {
        if (ed2++ >= 200) ed2 = 0;
        if (ed2 < 100) {
            clcd_putch(' ', LINE2(4));
            clcd_putch(' ', LINE2(5));
        } else {
            clcd_putch(setmin / 10 + 48, LINE2(4));
            clcd_putch(setmin % 10 + 48, LINE2(5));
        }
    } else {
        clcd_putch(setmin / 10 + 48, LINE2(4));
        clcd_putch(setmin % 10 + 48, LINE2(5));
    }

    clcd_putch(' ', LINE2(6));

    // **Blinking AM/PM**
    if (eflag == 2) {
        if (ed3++ >= 200) ed3 = 0;
        if (ed3 < 100) {
            clcd_putch(' ', LINE2(7));
            clcd_putch(' ', LINE2(8));
        } else {
            clcd_print(ap[ap_index], LINE2(7));
        }
    } else {
        clcd_print(ap[ap_index], LINE2(7));
    }

    clcd_putch(' ', LINE2(9));

    // **Blinking Mode (O, D, W)**
    if (eflag == 3) {
        if (ed4++ >= 200) ed4 = 0;
        if (ed4 < 100) {
            clcd_putch(' ', LINE2(11));
        } else {
            clcd_print(mode[m_index], LINE2(11));
        }
    } else {
        clcd_print(mode[m_index], LINE2(11));
    }

    if (key == 2) // Increment value
    {
        if (eflag == 0) // Hour
        {
            sethr = (sethr >= 12) ? 1 : sethr + 1;
        } else if (eflag == 1) // Minute
        {
            setmin = (setmin >= 59) ? 0 : setmin + 1;
        } else if (eflag == 2) // AM/PM toggle
        {
            ap_index = !ap_index; // Toggle between 0 (AM) and 1 (PM)
        } else if (eflag == 3) // ? **Fixed Mode Switching**
        {
            m_index = (m_index + 1) % 3; // Cycles `0 ? 1 ? 2 ? 0`
        }
    } else if (key == 3) // Change field (cycle through hour, min, AM/PM, mode)
    {
        eflag = (eflag + 1) % 4; // Ensure it cycles between 0, 1, 2, 3
    } else if (key == 4) // Save the event
    {
        arr[0] = sethr;
        arr[1] = setmin;
        arr[2] = (ap_index * 10) + m_index; // ? Ensure `m_index` is valid
        store(arr); // Store the event in EEPROM

        setf = 0;
        flag = SUB_MENU1;
    }
}

void store(char *arr) {
    char index = 0, curr, temp;
    char store[3]; // Stores event data from EEPROM for comparison

    if (lap == 0) { // First event stored at index 0
        for (int i = 0; i < 3; i++) {
            write_external_eeprom(i, arr[i]);
        }
        lap++;
        return;
    }

    if (lap >= 24) { // Wrap around if maximum storage is exceeded
        lap = 0;
    }

    // Extract AM/PM value from the current event
    curr = arr[2] / 10;

    // Find the correct insertion point based on priority
    for (index = 0; index < lap; index++) {
        store[0] = read_external_eeprom(index * 3);
        store[1] = read_external_eeprom(index * 3 + 1);
        store[2] = read_external_eeprom(index * 3 + 2);

        temp = store[2] / 10; // Extract AM/PM from stored event

        // Higher priority event check
        if (curr < temp) {
            break;
        } else if (curr > temp) {
            continue;
        } else {
            if (arr[0] < store[0]) {
                break;
            } else if (arr[0] > store[0]) {
                continue;
            } else {
                if (arr[1] < store[1]) {
                    break;
                } else if (arr[1] > store[1]) {
                    continue;
                } else {
                    return; // Duplicate event, no need to store
                }
            }
        }
    }

    // Shift stored events forward to make space for the new entry
    for (int i = lap; i > index; i--) {
        for (int j = 0; j < 3; j++) {
            char data = read_external_eeprom((i - 1) * 3 + j);
            write_external_eeprom(i * 3 + j, data);
        }
    }
    


    // Write the new event to EEPROM at the determined index
    for (int i = 0; i < 3; i++) {
        write_external_eeprom(index * 3 + i, arr[i]);
    }

    lap++; // Increment lap count for next event
}



char no = 0;
char serial = 0;

void view_event(void) {
    if (key == 5) {
        serial = 0;
        flag = SUB_MENU1;
    }

    clcd_print("   HH:MM A/P:M  ", LINE1(0));

    if (lap == 0) {
        clcd_print("   :NO EVENT:  ", LINE2(0));
    } else {
        // Read stored values from EEPROM
        char hour = read_external_eeprom(serial * 3);
        char minutes = read_external_eeprom(serial * 3 + 1);
        char am_pm = read_external_eeprom(serial * 3 + 2) / 10;
        char mode_index = read_external_eeprom(serial * 3 + 2) % 10;

        // **Check if EEPROM contains valid data**
        if (hour == 0xFF || minutes == 0xFF) {
            clcd_print("  --:-- -- -  ", LINE2(0)); // Display empty event
        } else {
            // Serial number
            clcd_putch(serial / 10 + 48, LINE2(0));
            clcd_putch(serial % 10 + 48, LINE2(1));

            clcd_putch(' ', LINE2(2));

            // Hour
            clcd_putch(hour / 10 + 48, LINE2(3));
            clcd_putch(hour % 10 + 48, LINE2(4));

            clcd_putch(':', LINE2(5));

            // Minutes
            clcd_putch(minutes / 10 + 48, LINE2(6));
            clcd_putch(minutes % 10 + 48, LINE2(7));

            clcd_putch(' ', LINE2(8));

            // AM/PM
            clcd_print(ap[am_pm], LINE2(9));

            clcd_print("  ", LINE2(11));

            // Mode (O, D, M)
            clcd_print(mode[mode_index], LINE2(13));
        }

    }

    // Move to next event on key press
    if (key == 2) {
        if (serial < lap - 1)
            serial++;
    }
}

void set_time(void) {
    if (key == 5) {
        setf = 0;
        flag = SUB_MENU2;
    }

    clcd_print(" HH:MM:SS AM/PM ", LINE1(0));


    if (setf == 0) {
        sethr = ((time[0] - '0') *10) + time[1] - '0';

        setmin = ((time[3] - '0') *10) + time[4] - '0';

        setsec = ((time[6] - '0') *10) + time[7] - '0';

        setf = 1;
    }


    clcd_putch(' ', LINE2(0));
    clcd_putch(sethr / 10 + 48, LINE2(1));
    clcd_putch(sethr % 10 + 48, LINE2(2));
    clcd_putch(':', LINE2(3));
    clcd_putch(setmin / 10 + 48, LINE2(4));
    clcd_putch(setmin % 10 + 48, LINE2(5));
    clcd_putch(':', LINE2(6));
    clcd_putch(setsec / 10 + 48, LINE2(7));
    clcd_putch(setsec % 10 + 48, LINE2(8));
    clcd_putch(' ', LINE2(9));
    clcd_print(ap[ap_index], LINE2(10));
    clcd_print("     ", LINE2(12));


    if (fflag == 0) { // Blink hours
        if (dy++ >= 200)
            dy = 0;
        if (dy < 100) {
            clcd_putch(' ', LINE2(1));
            clcd_putch(' ', LINE2(2));
        } else {
            clcd_putch(sethr / 10 + 48, LINE2(1));
            clcd_putch(sethr % 10 + 48, LINE2(2));
        }
    } else {
        clcd_putch(sethr / 10 + 48, LINE2(1));
        clcd_putch(sethr % 10 + 48, LINE2(2));
    }

    clcd_putch(':', LINE2(3));

    if (fflag == 1) { // Blink minutes
        if (dy2++ >= 200)
            dy2 = 0;
        if (dy2 < 100) {
            clcd_putch(' ', LINE2(4));
            clcd_putch(' ', LINE2(5));
        } else {
            clcd_putch(setmin / 10 + 48, LINE2(4));
            clcd_putch(setmin % 10 + 48, LINE2(5));
        }
    } else {
        clcd_putch(setmin / 10 + 48, LINE2(4));
        clcd_putch(setmin % 10 + 48, LINE2(5));
    }

    clcd_putch(':', LINE2(6));

    if (fflag == 2) { // Blink seconds
        if (dy3++ >= 200)
            dy3 = 0;
        if (dy3 < 100) {
            clcd_putch(' ', LINE2(7));
            clcd_putch(' ', LINE2(8));
        } else {
            clcd_putch(setsec / 10 + 48, LINE2(7));
            clcd_putch(setsec % 10 + 48, LINE2(8));
        }
    } else {
        clcd_putch(setsec / 10 + 48, LINE2(7));
        clcd_putch(setsec % 10 + 48, LINE2(8));
    }

    clcd_putch(' ', LINE2(10));
    //clcd_print("               ",LINE2(9));

    if (fflag == 3) // blink AM/PM
    {
        if (dy4++ >= 200)
            dy4 = 0;

        if (dy4 < 100) {
            //clcd_print("  ", LINE2(10));
            clcd_putch(' ', LINE2(11));
            clcd_putch(' ', LINE2(12));
        } else {
            clcd_print(ap[ap_index], LINE2(10));
        }
    } else {
        clcd_print(ap[ap_index], LINE2(10));
    }


    if (key == 2) // increment
    {

        if (fflag == 0) // hr
        {

            if (sethr > 11)
                sethr = 1;
            else
                sethr++;


        } else if (fflag == 1) // min
        {

            if (setmin > 58)
                setmin = 0;
            else
                setmin++;
        } else if (fflag == 2) // sec
        {


            if (setsec > 58)
                setsec = 0;
            else
                setsec++;


        } else if (fflag == 3) // AM / PM
        {
            //ap_index == !ap_index;
            //            if(ap_index > 0)
            //                ap_index =0;
            //            
            //            ap_index++;
            if (ap_index == 0)
                ap_index = 1;
            else
                ap_index = 0;

        }



    } else if (key == 3) // change field
    {
        if (fflag >= 3)
            fflag = 0;
        else
            fflag++;

    } else if (key == 4) // save in RTC and go back
    {
        // saving time before going back to menu

        sethr = (sethr / 10) << 4 | (sethr % 10) | 1 << 6 | ap_index << 5;
        write_ds1307(HOUR_ADDR, sethr);


        setmin = (setmin / 10) << 4 | (setmin % 10);
        write_ds1307(MIN_ADDR, setmin);


        setsec = (setsec / 10) << 4 | (setsec % 10);
        write_ds1307(SEC_ADDR, setsec);



        setf = 0;
        flag = SUB_MENU2;
    }
}

void set_date(void) {
    if (key == 5) {
        datef = 0;
        flag = SUB_MENU2;

    }

    clcd_print(" DD-MM-YY DAY  ", LINE1(0));

    if (datef == 0) // reading current date/month/year
    {
        dat = ((date[0] - '0')*10) + date[1] - '0';
        month = ((date[3] - '0')*10) + date[4] - '0';
        year = ((date[6] - '0')*10) + date[7] - '0';
        datef = 1;
    }

    clcd_putch(' ', LINE2(0));
    clcd_putch(dat / 10 + 48, LINE2(1));
    clcd_putch(dat % 10 + 48, LINE2(2));
    clcd_putch(':', LINE2(3));
    clcd_putch(month / 10 + 48, LINE2(4));
    clcd_putch(month % 10 + 48, LINE2(5));
    clcd_putch(':', LINE2(6));
    clcd_putch(year / 10 + 48, LINE2(7));
    clcd_putch(year % 10 + 48, LINE2(8));
    clcd_putch(' ', LINE2(9));
    clcd_print(day[day_index], LINE2(10));
    clcd_putch(' ', LINE2(13));


    if (sflag == 0) { // Blink date
        if (d++ >= 200)
            d = 0;
        if (d < 100) {
            clcd_putch(' ', LINE2(1));
            clcd_putch(' ', LINE2(2));
        } else {
            clcd_putch(dat / 10 + 48, LINE2(1));
            clcd_putch(dat % 10 + 48, LINE2(2));
        }
    } else {
        clcd_putch(dat / 10 + 48, LINE2(1));
        clcd_putch(dat % 10 + 48, LINE2(2));
    }

    clcd_putch(':', LINE2(3));

    if (sflag == 1) { // Blink month
        if (d2++ >= 200)
            d2 = 0;
        if (d2 < 100) {
            clcd_putch(' ', LINE2(4));
            clcd_putch(' ', LINE2(5));
        } else {
            clcd_putch(month / 10 + 48, LINE2(4));
            clcd_putch(month % 10 + 48, LINE2(5));
        }
    } else {
        clcd_putch(month / 10 + 48, LINE2(4));
        clcd_putch(month % 10 + 48, LINE2(5));
    }

    clcd_putch(':', LINE2(6));

    if (sflag == 2) { // Blink year
        if (d3++ >= 200)
            d3 = 0;
        if (d3 < 100) {
            clcd_putch(' ', LINE2(7));
            clcd_putch(' ', LINE2(8));
        } else {
            clcd_putch(year / 10 + 48, LINE2(7));
            clcd_putch(year % 10 + 48, LINE2(8));
        }
    } else {
        clcd_putch(year / 10 + 48, LINE2(7));
        clcd_putch(year % 10 + 48, LINE2(8));
    }

    clcd_putch(' ', LINE2(10));

    if (sflag == 3) // blink day
    {
        if (d4++ >= 200)
            d4 = 0;

        if (d4 < 100) {
            //clcd_print("  ", LINE2(10));
            clcd_putch(' ', LINE2(11));
            clcd_putch(' ', LINE2(12));
        } else {
            clcd_print(day[d_index], LINE2(10));
        }
    } else {
        clcd_print(day[d_index], LINE2(10));
    }



    if (key == 2) // increment
    {

        if (sflag == 0) // date
        {
            if (dat > 30)
                dat = 1;
            else
                dat++;


        } else if (sflag == 1) // month
        {
            if (month > 11)
                month = 1;
            else
                month++;

        } else if (sflag == 2) // year
        {

            if (year > 98)
                year = 1;
            else
                year++;

        } else if (sflag == 3) // day
        {
            if (d_index >= 6)
                d_index = 0;

            d_index++;

        }

    } else if (key == 3) // change field
    {
        if (sflag >= 3)
            sflag = 0;
        else
            sflag++;

    } else if (key == 4) // save in RTC and go back
    {
        // saving time before going back to menu

        dat = (dat / 10) << 4 | (dat % 10);
        write_ds1307(DATE_ADDR, dat);


        month = (month / 10) << 4 | (month % 10);
        write_ds1307(MONTH_ADDR, month);


        year = (year / 10) << 4 | (year % 10);
        write_ds1307(YEAR_ADDR, year);

        d_index = (d_index / 10) << 4 | (d_index % 10);
        write_ds1307(DAY_ADDR, year);




        datef = 0;
        flag = SUB_MENU2;
    }


}
