/** @file interrupt_handler.c
 *  
 *  @brief Implementation of timer-device driver library
 *
 *  This file contains handler for the timer device library 
 *  and helper functions.
 *
 *  @author Ishant Dawer (idawer)
 *  @bug No known bugs 
 */

#include "timer_driver.h"
#include "keyboard_driver.h"

int handler_install(void (*tickback)(unsigned int))
{
	handler_install_timer(tickback);
	handler_install_keybd();
	return 1;
}
