/** @file timer_driver_asm.S 
 *
 *  @brief Assembly wrapper for timer interrupt
 *
 *	This file contains definitions for assembly handler
 *	which will call C wrapper function which callsback the
 *	main handler 
 *  @author Ishant Dawer (idawer@andrew.cmu.edu)
 */

/** @brief Global declaration for Assembly wrapper */
.global timer_wrapper_asm

timer_wrapper_asm:
		pusha /* Save all general purpose registers*/
		call timer_handler_wrapper /*Call C handler*/
		popa /*Restore all general purpose registers*/
		iret /* Restore the program execution after interrupt*/

