/** @file game_helper.h
 *
 *  @brief Declarations for all game helper functions
 *
 *  This file contains all the macros and game required
 *  variables 
 *
 *  @author Ishant Dawer (idawer@andrew.cmu.edu)
 *  @bug No known bugs
 */

#include <p1kern.h>

/*Macros */
#define TRUE 1
#define ERROR -1
#define DIVIDE_BY_TWO 2
#define NUMBER_CYCLES 100
#define MINUS_ONE -1

/** @brief Game run start point 
 *  
 */

void game_run();

/** @brief Wait on a character
 *
 * @param char 
 * @return void
 */

void wait_char(char ch);

/** @brief Wait on a key press
 *
 * @param char 
 * @return void
 */

void wait_key_press();



/** @brief Tick function, to be called by the timer interrupt handler     
 *                                                                       
 *  In a real game, this function would perform processing which        
 *  should be invoked by timer interrupts.                             
 *                                                                    
 **/ 
/* Think about where this declaration
 * should be... probably not here!
 */
void tick(unsigned int numTicks);

