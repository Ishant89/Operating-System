/** @file game_helper.c
 *
 *  @brief Definitions for game related functions
 *
 *  This file contains all the macros and game required
 *  variables 
 *
 *  @author Ishant Dawer (idawer@andrew.cmu.edu)
 *  @bug No known bugs
 */

#include "game_helper.h"
#include<video_defines.h>
#include<stdint.h>

/*EDIT:To be removed*/
#include<contracts.h>
#include<simics.h>
#include<mt19937int.h>

#define PANEL_X ((CONSOLE_WIDTH)/2 -10)  /*Column*/
#define PANEL_Y (CONSOLE_HEIGHT)/2  /*ROW*/

#define SPACE 0x20
#define MAX_COLORS 8
#define SHIFT_BY_FOUR 4
#define ERROR -1

/*Color Variants */
#define FOUR_COLORS 4
#define FIVE_COLORS 5
#define SIX_COLORS 6
#define SEVEN_COLORS 7
#define EIGHT_COLORS 8

/* Default color */
#define DEFAULT_COLOR ((FGND_WHITE)|(BGND_BLACK))

/*Size of the panel*/
uint8_t matrix_len;
uint8_t matrix_wid;

/* Panel first element coordinates */
uint8_t start_x;
uint8_t start_y;



unsigned int seconds = 0;

void wait_char(char ch)
{
	char read; 
	while((read=readchar()) != ch)
	{
		continue;
	}
}

void wait_key_press()
{
	char read = readchar();
	while((read=readchar()) == ERROR)
	{
		continue;
	}
}
/** @brief Tick function, to be called by the timer interrupt handler
 * 
 *  In a real game, this function would perform processing which
 *  should be invoked by timer interrupts.
 *
 **/
void tick(unsigned int numTicks)
{
    if (numTicks % NUMBER_CYCLES == 0)
    {
        seconds++;
    }
}

/* EDIT: TO be deleted*/
void test_coordinates()
{
	ASSERT(PANEL_X != 12);
    ASSERT(PANEL_Y != 40);
	//lprintf("X co-ord is %d\n",PANEL_X);
	//lprintf("Y co-ord is %d\n",PANEL_Y);
}

void random_generator()
{
	int time1,time2;
	wait_key_press();
	time1 = seconds;
	wait_key_press();
	time2 = seconds;
	/* Generating random seed based on the
	 * time difference */
	//lprintf("seed is %d\n",time2-time1);
	sgenrand(time2-time1);
}

uint8_t choose_bg_color(unsigned long random,uint8_t num_colors)
{
	if (num_colors < 4 && num_colors > 8)
		return ERROR;
	uint8_t color_index = (random % num_colors) << SHIFT_BY_FOUR;
	//lprintf("Color index is %x \n",color_index);
	uint8_t output;
	switch (color_index)
	{
		case BGND_BLACK:
			output= BGND_BLACK;
			break;

		case BGND_BLUE:
			output= BGND_BLUE;
			break;

		case BGND_GREEN:
			output= BGND_GREEN;
			break;

		case BGND_CYAN:
			output= BGND_CYAN;
			break;

		case BGND_RED:
			output= BGND_RED;
			break;

		case BGND_MAG:
			output= BGND_MAG;
			break;

		case BGND_BRWN:
			output= BGND_BRWN;
			break;

		case BGND_LGRAY:
			output= BGND_LGRAY;
			break;

		default :
			output = ERROR;
			break;
	}
	return output;
}

void draw_game_panel(uint8_t len, uint8_t width,uint8_t num_colors)
{
	//lprintf("I am in panel ");
	int i,j,block_color;
	unsigned long  random;
	/* Find the center of the matrix*/
	uint8_t matrix_mid_x = (width-1)/DIVIDE_BY_TWO;
	uint8_t matrix_mid_y = (len-1)/DIVIDE_BY_TWO;

	/*Calculate the start element of the matrix */
	start_x  = PANEL_X - matrix_mid_x;
	start_y  = PANEL_Y - matrix_mid_y;
	
	/*Calculate the end coordinates of the panel */	
	uint8_t end_x = start_x + width + MINUS_ONE;
	uint8_t end_y = start_y + len + MINUS_ONE;

	/* Generate random numbers */
	/* Seed generation */
	random_generator();
	/* Generate a number */
	for (i = start_x - 1; i <= end_x + 1; i++)
	{
		if ((i == start_x -1) || (i == end_x + 1))
		{
			draw_char(start_y -1,i,'+',DEFAULT_COLOR);
		} else 
		{
			draw_char(start_y -1,i,'-',DEFAULT_COLOR);
		}	
	}
	for	(i=start_y;i <= end_y ; i++) 
	{
	   for (j= start_x-1;j<=end_x+1; j++)
	   {
		   if ((j == start_x -1)||(j==end_x + 1))
		   {

			   draw_char(i,j,'|',DEFAULT_COLOR);	
		   } else 
		   {
			   random = genrand();
			   //lprintf("Generated number is %lu\n",random);
			   block_color=choose_bg_color(random,num_colors);
			   //lprintf("Color is %d\n",block_color);
			   draw_char(i,j,SPACE,FGND_WHITE|block_color);
		   }
	   }	   
	}
	for (i = start_x - 1; i <= end_x + 1; i++)
	{
		if ((i == start_x -1) || (i == end_x + 1))
		{
			draw_char(end_y + 1 ,i,'+',DEFAULT_COLOR);
		} else 
		{
			draw_char(end_y + 1,i,'-',DEFAULT_COLOR);
		}	
	}/* set the cursor to the first element */
	set_cursor(start_y,start_x);

	show_cursor();
}


void game_run()
{
	/* TC 1 */
	test_coordinates();
	/*clear_console();
	draw_game_panel(6,6);
	clear_console();
	draw_game_panel(8,8);
	clear_console();
	draw_game_panel(10,10);
	clear_console();
	draw_game_panel(12,12);
	clear_console();
	draw_game_panel(14,14);*/
	clear_console();
	draw_game_panel(10,10,FOUR_COLORS);
	/*hide_cursor();
	set_cursor(0,0);
	clear_console();
	show_cursor();
	set_term_color(FGND_BLUE|BGND_GREEN);
	*/
}
