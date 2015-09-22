/** @file interrupt_handler.c
 *  
 *  @brief Implementation of timer-device driver library
 *
 *  This file contains handler for the timer device library 
 *  and helper functions.
 *
 *  What it contains : 
 *  1. Timer Interrupt installer 
 *  2. Keyboard Interrupt installer
 *
 *  @author Ishant Dawer (idawer)
 *  @bug No known bugs 
 */

#include "timer_driver.h"
#include "keyboard_driver.h"
#define OK 1
#define ERROR -1

int handler_install(void (*tickback)(unsigned int))
{
	int status_timer,status_keybd;
	status_timer = handler_install_timer(tickback);
	status_keybd = handler_install_keybd();
	if (status_timer && status_keybd)
		return OK;
	else 
		return ERROR;
}
