/** @file keyboard_driver.c
 *  
 *  @brief Implementation of keyboard-device driver library
 *
 *  This file contains handler for the keyboard device library 
 *  and helper functions.
 *
 *  @author Ishant Dawer (idawer)
 *  @bug No known bugs 
 */

#include "keyboard_driver.h"

#define BUFFER_ITEMS 4
#define BUFFER_START_ENTRY_INDEX 0
#define ERROR -1
#define OK 0
/* Buffer to store keyboard events */
char buf[100];

/* Buffer iterator */
volatile char * put_buf_iter = buf;

/* Buffer iterator */
volatile char * rem_buf_iter = buf;

/* Current number of buffer items */
#define ITEMS_IN_BUFF ((((char*)(put_buf_iter)) - ((char*)(buf))))
#define ITEMS_READ (((rem_buf_iter) - (buf)))

int handler_install_keybd()
{
    /*
     * Step1: Install the IDT table entry 
     */

    /* Get the IDT base address */
    void * idt_base_addr = idt_base();
    void * assembly_wrapper = keybd_wrapper_asm;
    /* Index is mentioned in the timer defines library*/

    unsigned int index = KEY_IDT_ENTRY * SIZE_2_WORDS;

    /* Writing IDT entry for timer handler */
    *(unsigned int*)(idt_base_addr + index) = LOWER_32(assembly_wrapper);
    *(unsigned int*)(idt_base_addr + index + SIZE_UINT) = UPPER_32(assembly_wrapper);
    return 1;
}

void send_ack_pic1()
{
	outb(INT_CTL_PORT,INT_ACK_CURRENT);
}


void keyboard_event_handler()
{
	/* Read the keyboard event scancode */
	char event_scancode = inb(KEYBOARD_PORT);

	/* Add the event scancode to the buffer */
	if (ITEMS_IN_BUFF >= BUFFER_ITEMS)
	{
		put_buf_iter = buf;
	}
	*put_buf_iter = event_scancode;
	lprintf("buf: %p put_buf_iter: %p data:%d\n",buf,put_buf_iter,
			*put_buf_iter);
	put_buf_iter++;
	/*Send ack signal to PIC */
	send_ack_pic1();	
}

int readchar()
{
	return -1;
}
