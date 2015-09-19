/** @file Timer header file 
 *
 *  @brief header file for timer driver 
 *
 *  @author Ishant Dawer(idawer@andrew.cmu.edu)
 *
 *  @bug No known bugs
 */

#include <p1kern.h>
#include <stdio.h>
/* EDIT: Remove later */
#include <simics.h>
#include <contracts.h>
#include <asm.h>
#include <timer_defines.h>
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

/** @brief Number of timer cycles between interrupts */
#define INTERRUPT_DELAY 10/1000

/** @brief Get LSB
 *
 * This is used to get the LSB out of the 16-bit 
 * integer. 
 *
 * @param x 16-bit number 
 *
 * @return LSB
 */

#define GET_LSB(x) ((x) & (0x00FF))

/** @brief Get MSB
 *
 * This is used to get the MSB out of the 16-bit integer.
 *
 * @param x 16-bit number 
 *
 * @return MSB
 */

#define GET_MSB(x) (((x) & (0xFF00)) >> 8)


/* Callback function for timer handler */

void (*callback_function_addr)(unsigned int) ;


/** @brief */
void timer_wrapper_asm();

/** @brief Handler for timer interrupt 
 *
 * @param tickback function pointer 
 *
 * @return int
 */

int handler_install_timer(void (*tickback)(unsigned int));

/** @brief Wrapper for C handler 
 *
 * @param none
 *
 * @return void
 */

void timer_handler_wrapper();


/** @brief Send ack signal to interrupt
 *
 */

void send_ack_pic();
