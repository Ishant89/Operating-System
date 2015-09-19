/** @file timer_driver.c
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

/* Number of timer interrupts handlers invoked */

static unsigned int numTicks = 0;


int handler_install_timer(void (*tickback)(unsigned int))
{
	/*
	 * Step1: Install the IDT table entry 
	 */

	/* Get the IDT base address */
	void * idt_base_addr = idt_base();
	void * assembly_wrapper = timer_wrapper_asm;
	/* EDIT:Change and round it up */
	unsigned int number_cycles = (TIMER_RATE * INTERRUPT_DELAY) + 1;
	/* Index is mentioned in the timer defines library*/

	unsigned int index = TIMER_IDT_ENTRY * SIZE_2_WORDS;
	/* Evaluating parameters of the IDT entry */
	callback_function_addr = tickback;

	/* Writing IDT entry for timer handler */
	*(unsigned int*)(idt_base_addr + index) = LOWER_32(assembly_wrapper); 
	*(unsigned int*)(idt_base_addr + index + SIZE_UINT) = UPPER_32(assembly_wrapper); 
	/* Write to IO ports */
	
	outb(TIMER_MODE_IO_PORT,TIMER_SQUARE_WAVE);
	outb(TIMER_PERIOD_IO_PORT,GET_LSB(number_cycles));
	outb(TIMER_PERIOD_IO_PORT,GET_MSB(number_cycles));
	return 1;
}

void send_ack_pic()
{
	outb(INT_CTL_PORT,INT_ACK_CURRENT);
}

/* Handler function wrapper for assembly */

void timer_handler_wrapper()
{
	numTicks++;
	(*callback_function_addr)(numTicks);
	/* sending acknowledgement */
	send_ack_pic();
}


