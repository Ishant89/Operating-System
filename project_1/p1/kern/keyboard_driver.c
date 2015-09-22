/** @file keyboard_driver.c
 *  
 *  @brief Implementation of keyboard-device driver library
 *
 *  This file contains following things:
 *  1. Keyboard Install handler: 
 *
 *  --This handler installs the keyboard interrupt entry in the 
 *  IDT table
 *  -- This handler defines the assemnly wrapper to execute when
 *  interrupt is received 
 *  -- Defines a handler in C which basically keeps on adding 
 *  items read from the keyboard in the circular buffer. 
 *  Once the buffer gets filled,it takes a circular loop and overwrites
 *  the date in earlier locations 
 *
 *  -- Interrrupt handler keeps on writing till it again meets the read
 *  pointer whose job is to read from the buffer. 
 *
 *  --
 *  (kernel  interrupt pointer) write pointer will check 
 *  if the next location is not being pointed
 *  by read pointer. Because if it writes to that location pointed by read,
 *  there will be a collision. 
 *
 *  -- Above logic will not accept further data from keyboard if buffer is 
 *  not being read.
 *
 *	
 *	2. readchar (Library function to read chars from keyboard);
 *
 *	-- Readchar is an API library used by kernel to read from the circular 
 *	buffer
 *	-- It will read till it reaches the write pointer to the buffer 
 *	from the handler.
 *
 *	- Separate pointers to read and write the buffer helps in saving the 
 *	concurrency
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
/** @brief Circular buffer to store keyboard events data */
char buf[BUFFER_ITEMS];

/* Buffer iterator */
/** @brief put pointer to buf */
volatile char * put_buf_iter = buf;

/** @brief remove pointer to buf */
volatile char * rem_buf_iter = buf;

/** @brief Items put by put pointer */
#define ITEMS_IN_BUFF ((((char*)(put_buf_iter)) - ((char*)(buf))))

/** @brief Item read by remove pointer */
#define ITEMS_READ ((((char*)(rem_buf_iter)) -((char*) (buf))))

/** @brief Installl keyboard handler 
 * 
 *	This handler does following things : 
 *
 *	1. Install entry into IDT table 
 *	2. Define it as a Trap gate
 *	3. Defines the assembly handler
 */
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

/** @brief Send acknowledge to PIC
 *
 * This function writes to PIC after handling the 
 * handler 
 */

void send_ack_pic1()
{
	outb(INT_CTL_PORT,INT_ACK_CURRENT);
}

/** @brief keyboard event handler
 *  
 *  This function handler the interrupt
 *
 *  1. Checks if the next location is not 
 *  captured by remove pointer 
 *  2. If yes, then stops
 *  3. Installs the keyboard event into circular buffer
 *  4. Sends back the acknowledgement
 */

void keyboard_event_handler()
{
	/* Read the keyboard event scancode */
	char event_scancode = inb(KEYBOARD_PORT);

	/* Add the event scancode to the buffer */
	if ((put_buf_iter +1) != rem_buf_iter)
	{
		*put_buf_iter = event_scancode;
		put_buf_iter++;
	
	
		if (ITEMS_IN_BUFF >= BUFFER_ITEMS)
		{
			put_buf_iter = buf;
		}
	}
	/*Send ack signal to PIC */
	send_ack_pic1();	
}

/** @brief Read character library function
 *
 *  This function does following:
 *  1. Reads from the circular buffer
 *  2. Stops when it reaches put pointer 
 *	3. Parses the event from the buffer into scan_code engine
 *	4. Checks when key was pressed 
 *	5. Gets the character 
 *	6. Returns -1 if no character otherwise character
 *
 *	@param void
 *	@return void
 */

int readchar()
{
	char output,scan_code;
	int augmented_char;
	int result = OK;
	/* Get the latest from the buffer */
	if (put_buf_iter != rem_buf_iter)
	{	
		scan_code = *rem_buf_iter;
		augmented_char = process_scancode(scan_code);
		if (KH_HASDATA(augmented_char))
		{
			if (KH_ISMAKE(augmented_char))
				output = KH_GETCHAR(augmented_char);
			else 
				result = ERROR;
		} else 
		{
			result = ERROR;
		}
		/* Removing from the buffer */
		if ( ITEMS_READ >= (BUFFER_ITEMS -1))
		{
			rem_buf_iter = buf;
		} else 
		{
			rem_buf_iter++;
		}
	} else 
	{
		result = ERROR;
	}
	if (result == OK)
		return output;
	else 
		return result;
}
