/***************************************************************************
 *   Copyright (C) 2013 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//#include <SDL/SDL.h>
#include <wiringPi.h>
#include "system/input.h"
#include "system/system.h"
#include "misc/config.h"

//pin definitions
const int pin_led = 18;
const int pin_reset = 17;
const int pin_clk = 5;
const int pin_out = 6;
const int pin_p0_d0 = 13;
const int pin_p0_d3 = 19;
const int pin_p0_d4 = 26;
const int pin_p1_d0 = 16;
const int pin_p1_d3 = 20;
const int pin_p1_d4 = 21;

int input_init()
{
	wiringPiSetupGpio();
    pinMode(pin_led, PWM_OUTPUT);
    pinMode(pin_reset, INPUT);     // Set regular LED as output
//    pinMode(butPin, INPUT);      // Set button as INPUT
//    pullUpDnControl(butPin, PUD_UP); // Enable pull-up resistor on button
	return(0);
}

void input_kill()
{
}

void input_poll()
{
}

int input_poll_mouse(int *x,int *y)
{
	return(0);
}

void input_update_config()
{
}
