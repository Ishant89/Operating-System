/** @file keyboard_driver.h 
 *  @brief Declarations and Macros
 *
 *  This file contains necessary declarations 
 *  for the required functions 
 *
 *  1. Assembly handler
 *  2. Macro to generate Trap gate bits format for the given 
 *	IDT address 
 * 
 *
 *	@author Ishant Dawer (idawer@andrew.cmu.edu)
 *	@bug No known bugs
 */

#include<keyhelp.h>
#include <p1kern.h>
#include <stdio.h>
#include <simics.h>
#include <contracts.h>
#include <asm.h>
#include <interrupt_defines.h>
#include <seg.h>

/** @brief Get the size of 2 words */

#define SIZE_2_WORDS ((2)*(sizeof(void *)))

/** @brief Get offset for first 16 bits */

#define OFFSET_FIRST(x) (((unsigned int)(x)) & (0xFFFF))                    
#define OFFSET_SECOND(x) (((unsigned int)(x)) & (0xFFFF0000)) 


/** @brief Higher 32 bits with D=1*/
#define TRAP_GATE_REM_BITS 0x8F00

/** @brief Higher 32 bits of Trap gate */

#define UPPER_32(x) (TRAP_GATE_REM_BITS | OFFSET_SECOND(x))

/** @brief Lower 32 bits of Trap gate */

#define LOWER_32(x) (((SEGSEL_KERNEL_CS) << 16) | OFFSET_FIRST(x))

/** @brief Size of unsigned int */
#define SIZE_UINT sizeof(unsigned int)

/** @brief Handler keyboard 
 *
 *  @param none
 *
 *  @return none 
 */

int handler_install_keybd();

/** @brief Keyboard event handler 
 *	
 *	This handler does following things : 
 *
 *	1. Install entry into IDT table 
 *	2. Define it as a Trap gate
 *	3. Defines the assembly handler

 *	@param none 
 *
 *	@return Void
 */

void keyboard_event_handler();

/** @brief Send ack signal to interrupt
 * This function writes to PIC after handling the 
 * handler 
 */
void send_ack_pic1();

/** @brief Key board wrapper in assembly
 *
 *	This is an assembly wrapper whose task is to 
 *	store all the general purpose registers 
 *	and retrieve them after its execution
 *	and perform IRET which will restore all the 
 *	Flag regsisters and others.
 *  
 *  @param none 
 *
 *  @return void
 */


void keybd_wrapper_asm();
