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
#include <stdint.h>

/*Macros */
#define TRUE 1
#define ERROR -1
#define DIVIDE_BY_TWO 2
#define NUMBER_CYCLES 100
#define MINUS_ONE -1

 
#define FOUR_COLORS 4    
#define FIVE_COLORS 5    
#define SIX_COLORS 6    
#define SEVEN_COLORS 7  
#define EIGHT_COLORS 8    


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
 * @return char 
 */

char wait_key_press();



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

/** @brief move_cursor 
 *
 * @param char ch
 * @return int If finished 0,else -1 
 */

void move_cursor(char ch);

