/** @file keyboard_driver_asm.S
 *	@brief Keyboard assembly wrapper
 *	
 *	This file contains the definition of the 
 *	assembly wrapper which will be called from 
 *	the Interrupt handler 
 *
 *	Purpose : 
 *	1. Push general purpose registers 
 *	2. Call C handler 
 *	3. Pop all the registers 
 *	4. Return and restore all flags
 *	@author Ishant Dawer (idawer@andrew.cmu.edu)
 */

/** @brief Making this assembly wrapper global */
.global keybd_wrapper_asm

keybd_wrapper_asm:
		pusha /* Save all general purpose registers */
		call keyboard_event_handler /*Call C handler */
		popa /*Restore all general puprose registers */
		iret /* Restore the program execution after interrupt*/
