/** @file keyboard_driver_asm.S
 *
 *	@brief Keyboard assembly wrapper
 *
 *	@author Ishant Dawer (idawer@andrew.cmu.edu)
 */

.global keybd_wrapper_asm

keybd_wrapper_asm:
		pusha /* Save all general purpose registers */
		call keyboard_event_handler /*Call C handler */
		popa /*Restore all general puprose registers */
		iret /* Restore the program execution after interrupt*/
