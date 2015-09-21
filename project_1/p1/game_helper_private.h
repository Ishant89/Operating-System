/** @file game_helper_private.h
 *  
 *  @brief Contains declarations and Macros
 *
 *  @author Ishant Dawer (idawer@andrew.cmu.edu)
 *  @bug No known bugs
 */

#include<video_defines.h>
#include<malloc.h>

#include<contracts.h>
#include<simics.h>
#include<mt19937int.h>
#include<stdio.h>
#include<string.h>

#define PANEL_X ((CONSOLE_WIDTH)/2 -10)  /*Column*/
#define PANEL_Y ((CONSOLE_HEIGHT)/2 -3)  /*ROW*/

#define SPACE 0x20
#define MAX_COLORS 8
#define SHIFT_BY_FOUR 4
#define ERROR -1
#define OK 1
#define SEPARATOR 12
#define VAL_SEPARATOR 14
#define TEXT_SEPARATOR 7
#define SIDE_ITEMS 6
#define EQUAL_CHARACTER '='
#define TRUE 1
#define FALSE 0
#define SIXTY_SECONDS 60
#define ROW_BEGIN_INDX 0
#define ROW_END_INDX ((CONSOLE_WIDTH)-1)

/* Default color */
#define DEFAULT_COLOR ((FGND_WHITE)|(BGND_BLACK))

/*Title Screen contents*/
#define MAX_STR CONSOLE_WIDTH 
#define MATRIX_SIX 6
#define MATRIX_EIGHT 8
#define MATRIX_TEN 10
#define MATRIX_TWELVE 12
#define MATRIX_FOURTEEN 14

#define FOUR_COLORS 4
#define FIVE_COLORS 5
#define SIX_COLORS 6
#define SEVEN_COLORS 7
#define EIGHT_COLORS 8

#define NUM_BOARD_SIZES 5
#define NUM_DIFF_COLRS 5
/*Array of maximum iterations */
uint8_t max_iter[NUM_BOARD_SIZES][NUM_DIFF_COLRS] ={
	{7,8,10,12,14},
	{9,11,14,16,19},
	{11,14,17,20,23},
	{14,17,21,25,28},
	{16,20,25,29,33}
};



/*Size of the panel & game related params*/
uint8_t matrix_len;
uint8_t matrix_wid;
uint8_t num_colors;
uint8_t max_iterations;
uint8_t curr_user_iteration;
int game_index=-1;
uint8_t cursor_hidden = FALSE;
/* Panel first and last element coordinates */
uint8_t start_x;
uint8_t start_y;
uint8_t end_x;
uint8_t end_y;

/*Game status booleans */
uint8_t pause = TRUE;
uint8_t start = FALSE;
uint8_t finish = FALSE;
uint8_t fail = FALSE;
uint8_t quit = FALSE;
uint8_t help = FALSE;

/* Timer vars */
unsigned int seconds = 0;
unsigned int timer_ticks = 0;

/* Game state buffer */

uint8_t * game_state_buf;


/* Top 5 items in the buffer */
#define MAX_ENTRIES 5
int iterations_used[MAX_ENTRIES];
int completion_time[MAX_ENTRIES];


