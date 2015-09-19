/** @file keyboard_driver.h 
 *
 * @author Ishant Dawer (idawer@andrew.cmu.edu)
 * @bug No known bugs
 */

#include<keyhelp.h>
#include <p1kern.h>
#include <stdio.h>
/* EDIT: Remove later */
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
 *	@param none 
 *
 *	@return Void
 */

void keyboard_event_handler();

/** @brief Send ack signal to interrupt
 */
/* EDIT:Single definition */
void send_ack_pic1();

/** @brief Key board wrapper in assembly
 */

void keybd_wrapper_asm();
