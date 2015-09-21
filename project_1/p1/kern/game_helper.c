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
#include "game_helper_private.h"

void wait_char(char ch)
{
	char read; 
	while((read=readchar()) != ch)
	{
		continue;
	}
}

char wait_key_press()
{
	char read = readchar();
	while((read=readchar()) == ERROR)
	{
		continue;
	}
	return read;
}
/** @brief Tick function, to be called by the timer interrupt handler
 * 
 *  In a real game, this function would perform processing which
 *  should be invoked by timer interrupts.
 *
 **/
void tick(unsigned int numTicks)
{
	timer_ticks = numTicks;
    if (numTicks % NUMBER_CYCLES == 0)
    {
		int actual_cursor_row,actual_cursor_col;
		if (!pause) 
		{
			seconds++;
		} 
		if (start)
		{
			get_cursor(&actual_cursor_row,&actual_cursor_col);
			set_cursor(start_y-1,end_x + VAL_SEPARATOR);
			printf("%02d:%02d",seconds/SIXTY_SECONDS,
					seconds%SIXTY_SECONDS);
			set_cursor(end_y+4,start_x+matrix_wid/2-8);
			printf("Current Iteration:%d/%d",curr_user_iteration+1,max_iterations);
			set_cursor(actual_cursor_row,actual_cursor_col);

			/* Blink */
			if(cursor_hidden)
			{
				cursor_hidden = FALSE;
				show_cursor();
			} else 
			{
				cursor_hidden = TRUE;
				hide_cursor();
			}
		}

    }
}


void clear_string_row(int row)
{
	int i = ROW_BEGIN_INDX;
	for(;i<=ROW_END_INDX;i++)
	{
		draw_char(row,i,SPACE,DEFAULT_COLOR);
	}
}

void put_str(int row,int column,char * str)
{
	set_cursor(row,column);
	putbytes(str,strlen(str));
}

uint8_t choose_bg_color(unsigned long random,int num_colors)
{
	if (num_colors < 4 && num_colors > 8)
		return ERROR;
	uint8_t color_index = random % num_colors;
	uint8_t output;
	switch (color_index)
	{
		case 0:
			output= BGND_BLACK;
			break;

		case 1:
			output= BGND_BLUE;
			break;

		case 2:
			output= BGND_GREEN;
			break;

		case 3:
			output= BGND_CYAN;
			break;

		case 4:
			output= BGND_RED;
			break;

		case 5:
			output= BGND_MAG;
			break;

		case 6:
			output= BGND_BRWN;
			break;

		case 7:
			output= BGND_BRWN;
			//output= BGND_LGRAY|BGND_BRWN;
			break;
	}
	return output;
}
void draw_screen_sidebar()
{
	/*Print timer menu*/
	int column = end_x + SEPARATOR,row = start_y-1;
	int actual_cursor_row,actual_cursor_col;
	int i = 0 ;
	/*Draw equal operator */
	for (;i< SIDE_ITEMS;i++)
	{
		draw_char(row,column,EQUAL_CHARACTER,DEFAULT_COLOR);
		row++;
	}
	
	/*Print time and other parameters */
	get_cursor(&actual_cursor_row,&actual_cursor_col);
	set_cursor(start_y-1,end_x + TEXT_SEPARATOR);

	printf("time");
	/* Setting the seconds to 0 */
	if (!help)
		seconds = 0;
	pause = FALSE;

	set_cursor(start_y,end_x + TEXT_SEPARATOR);
	printf("wasd");

	set_cursor(start_y,end_x + VAL_SEPARATOR);
	printf("move");

	set_cursor(start_y+1,end_x + TEXT_SEPARATOR);
	printf("space");

	set_cursor(start_y+1,end_x + VAL_SEPARATOR);
	printf("mark");

	set_cursor(start_y+2,end_x + TEXT_SEPARATOR);
	printf("p");

	set_cursor(start_y+2,end_x + VAL_SEPARATOR);
	printf("pause");

	set_cursor(start_y+3,end_x + TEXT_SEPARATOR);
	printf("h");

	set_cursor(start_y+3,end_x + VAL_SEPARATOR);
	printf("help");

	set_cursor(start_y+4,end_x + TEXT_SEPARATOR);
	printf("q");

	set_cursor(start_y+4,end_x + VAL_SEPARATOR);
	printf("quit");

	set_cursor(actual_cursor_row,actual_cursor_col);
}

void draw_game_panel(uint8_t len, uint8_t width,uint8_t num_colors)
{
	int i,j,block_color,item_index=0;
	unsigned long  random;
	/* Find the center of the matrix*/
	uint8_t matrix_mid_x = (width-1)/DIVIDE_BY_TWO;
	uint8_t matrix_mid_y = (len-1)/DIVIDE_BY_TWO;

	/*Calculate the start element of the matrix */
	start_x  = PANEL_X - matrix_mid_x;
	start_y  = PANEL_Y - matrix_mid_y;
	
	/*Calculate the end coordinates of the panel */	
	end_x = start_x + width + MINUS_ONE;
	end_y = start_y + len + MINUS_ONE;

	/* Generate random numbers */
	/* Seed generation */
	//random_generator();
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
			   block_color=choose_bg_color(random,num_colors);
			   game_state_buf[item_index]=block_color;
			   draw_char(i,j,SPACE,FGND_WHITE|block_color);
			   item_index++;
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
	/* Draw side bar menu */

	draw_screen_sidebar();

	set_cursor(start_y,start_x);

	show_cursor();
}

int move_up(int row)
{
	/*Show cursor if hidden */
	if ((row - 1) >= start_y)
	{
		row--;
	}
	return row;
}

int move_left(int col)
{
	if ((col - 1) >= start_x)
	{
		col--;
	}
	return col;
}

int move_down(int row)
{
	if ((row + 1) <= end_y)
	{
		row++;
	}
	return row;
}

int move_right(int col)
{
	if ((col + 1) <= end_x)
	{
		col++;
	}
	return col;
}

int get_elem_bg_color(int row,int col)
{
	int elem_index,elem_row,elem_col;
	int color;
    elem_row = row - start_y;
	elem_col = col - start_x;
	elem_index = elem_row * matrix_wid + elem_col;
	color = game_state_buf[elem_index];
	return color;
}	
void update_elem_bg_color(int row,int col,int color)
{
	int elem_index,elem_row,elem_col;
    elem_row = row - start_y;
	elem_col = col - start_x;
	elem_index = elem_row * matrix_wid + elem_col;
	game_state_buf[elem_index] = color;
}

void flood_it(int row,int column,int new_color,int old_color)
{
	/*Get the old color associated with this element */
	int curr_elem_color = get_elem_bg_color(row,column);
	/* Check if new color matches old color */
	if (old_color != curr_elem_color)
		return;
	else 
	{
		/* Update the elem with new color */
		draw_char(row,column,SPACE,FGND_WHITE|new_color);
		update_elem_bg_color(row,column,new_color);
		/*Check if next elem to its right is valid */
		if ((column + 1) <= end_x)
		{
			flood_it(row,column+1,new_color,curr_elem_color);
		}
		/*Check if next elem in down direction is valid */
		if ((row + 1) <= end_y)
		{
			flood_it(row+1,column,new_color,curr_elem_color);
		}
		/*Check if next elem in up dir is valid */
		if ((row - 1) >= start_y)
		{
			flood_it(row-1,column,new_color,curr_elem_color);
		}
		/*Check if next elem to its left is valid */
		if ((column - 1) >= start_x)
		{
			flood_it(row,column-1,new_color,curr_elem_color);
		}
	}
}

int is_game_over(int top_elem_color)
{
	int item_index = 0;
	int total_elems = matrix_len * matrix_wid;
	int i,j,block_color;
	for	(i=start_y;i <= end_y ; i++) 
	{
	   for (j= start_x;j<=end_x; j++)
	   {
		   block_color=game_state_buf[item_index];
		   if (block_color == top_elem_color)
			   item_index++;
		   else 
			   break;
	   }	   
	}
	if (total_elems == item_index)
		return TRUE;
	else 
		return FALSE;
}

void mark(int color)
{
	/*Get the top elements color*/
	int top_elem_color = get_elem_bg_color(start_y,start_x);
	if (top_elem_color != color )
	{
		curr_user_iteration++;
		if(curr_user_iteration >= max_iterations)
		{
			fail = TRUE;
			pause = TRUE;
			start = FALSE;
			hide_cursor();
			iterations_used[game_index]=curr_user_iteration;
			completion_time[game_index]= seconds;
			lprintf("Failed");
			return ;
		}
		flood_it(start_y,start_x,color,top_elem_color);
		finish = is_game_over(color);
		if (finish)
		{
			start = FALSE;
			pause = TRUE;
			hide_cursor();
			iterations_used[game_index]=curr_user_iteration;
			completion_time[game_index]= seconds;
			lprintf("Finsihed");
			return;
		}
	}
	/* Else no op */
}

void pause_op()
{
	/* Save cursor position */
	int actual_cursor_row,actual_cursor_col;
	int block_color,index=0;
	get_cursor(&actual_cursor_row,&actual_cursor_col);
	hide_cursor();
	pause = TRUE;
	int i,j;
	for	(i=start_y;i <= end_y ; i++) 
	{
	   for (j= start_x;j<=end_x; j++)
	   {
		   /*Obscure with brown color */
		   block_color = BGND_BRWN;
		   draw_char(i,j,SPACE,FGND_WHITE|block_color);
	   }	   
	}

	set_cursor(start_y-4,start_x);
	printf("Game paused");
	set_cursor(start_y-3,start_x);
	printf("Press 'c' to continue.");

	wait_char('c');
	pause = FALSE;	
	set_cursor(actual_cursor_row,actual_cursor_col);
	/* Restore cursor and color */
	clear_string_row(start_y -4);
	clear_string_row(start_y -3);
	show_cursor();
	for	(i=start_y;i <= end_y ; i++) 
	{
	   for (j= start_x;j<=end_x; j++)
	   {
		   block_color = game_state_buf[index];
		   draw_char(i,j,SPACE,FGND_WHITE|block_color);
		   index++;
	   }	   
	}
}

void quit_op()
{
	char ch = 'y';
	set_cursor(end_y + 6, start_x - 5);
	printf("You are quitting? Click 'y' to confirm");
	wait_char(ch);
	pause = TRUE;
	quit = TRUE;
	start = FALSE;
}

void help_menu_game_screen()
{
	int row,column;
	int i,j,block_color,item_index=0;
	row = PANEL_Y;
	column = PANEL_X -10 ;
	clear_console();
	help = TRUE;
	start = FALSE;
	pause = TRUE;
	put_str(row,column,"Instructions:-");
	put_str(row+1,column,
			"1. Use w,s for movement in up,down dir resp.");
	put_str(row+2,column,
			"2. Use a,d for movement in left,down dir resp.");
	put_str(row+3,column,
			"2. Hit space on an element which color should be flooded");
	put_str(row+4,column,
			"3.For color to flood, select elem with color diff. than Top Left");


	put_str(row+6,column,
		"Press 'k' to go back to game screen");
	wait_char('k');
	

	clear_console();	
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
			   block_color=game_state_buf[item_index];
			   draw_char(i,j,SPACE,FGND_WHITE|block_color);
			   item_index++;
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
	/* Draw side bar menu */

	seconds = seconds;
	draw_screen_sidebar();

	set_cursor(start_y,start_x);

	show_cursor();
	
}

void move_cursor(char ch)
{
	int row,col;
	get_cursor(&row,&col);
	int color;
	switch (ch)
	{
		case 'w':
			row = move_up(row);
			break;

		case 'a':
			col = move_left(col);
			break;

		case 's':
			row = move_down(row);
			break;
		
		case 'd':
			col = move_right(col);
			break;

		case SPACE:
			color=get_elem_bg_color(row,col);	
			mark(color);
			break;

		case 'h':
			help_menu_game_screen();
			break;

		case 'p':
			pause_op();
			break;

		case 'q':
			quit_op();
			break;

		default :
			break;
	}
	set_cursor(row,col);
}

int help_menu()
{
	int row,column,status;
	row = PANEL_Y;
	column = PANEL_X -10 ;
	clear_console();
	put_str(row,column,"Instructions:-");
	put_str(row+1,column,
			"1. Use w,s for movement in up,down dir resp.");
	put_str(row+2,column,
			"2. Use a,d for movement in left,down dir resp.");
	put_str(row+3,column,
			"2. Hit space on an element which color should be flooded");
	put_str(row+4,column,
			"3.For color to flood, select elem with color diff. than Top Left");


	put_str(row+6,column,
		"Press 'k' to go back to title screen");
	wait_char('k');
	status = ERROR;
	return status;

}


void title_screen()
{
	int row,column;
	int i;
	/*Display Author name */ 
	row = PANEL_Y - 8;
	column = PANEL_X;
	put_str(row,column,"Author: ISHANT DAWER");
	
	/* Display the fields of last 5 games */
	row = row + 4;
	column = PANEL_X -20 ;
	put_str(row,column,"Sequence");
	
	for (i=0;i< MAX_ENTRIES;i++)
	{
		set_cursor(row+i+1,column+3);
		printf("%d",i+1);
	}
	column += 20;
	put_str(row,column,"Flood Count");

	for (i=0;i< MAX_ENTRIES;i++)
	{
		set_cursor(row+i+1,column+3);
		printf("%d",iterations_used[i]);
	}
	column += 20;
	put_str(row,column,"Completion Time");
	for (i=0;i< MAX_ENTRIES;i++)
	{
		set_cursor(row+i+1,column+3);
		printf("%02d:%02d",completion_time[i]/SIXTY_SECONDS,completion_time[i]%SIXTY_SECONDS);
	}

	/*EDIT:Add data in sometime */
	row = PANEL_Y + 7;
	column = PANEL_X - 15;
	put_str(row,column,"Press 'b' to start the game or 'h' to open help menu");

}

int select_board_size_and_color()
{
	/* Press options 1-5 to select board size*/
	char ch;
	ch = wait_key_press();
	int index_board,index_num_color;
	int time1,time2;
	switch(ch) 
	{
		case '1':
			/*Select 6*6*/
			matrix_len=MATRIX_SIX;
			matrix_wid=MATRIX_SIX;
			index_board = 0;
			break;
		case '2':
			/*Select 8*8 */
			matrix_len=MATRIX_EIGHT;
			matrix_wid=MATRIX_EIGHT;
			index_board = 1;
			break;
		case '3':
			/*Select 10*10*/
			matrix_len=MATRIX_TEN;
			matrix_wid=MATRIX_TEN;
			index_board = 2;
			break;
		case '4':
			/*Select 12*12*/
			matrix_len=MATRIX_TWELVE;
			matrix_wid=MATRIX_TWELVE;
			index_board = 3;
			break;
		case '5':
			/*Select 14*14*/
			matrix_len=MATRIX_FOURTEEN;
			matrix_wid=MATRIX_FOURTEEN;
			index_board = 4;
			break;
		default:
			return ERROR;
	}

	/* Generate random event */
	time1 = timer_ticks;
	/*Press again 1-5 to select number of color */
	ch = wait_key_press();
	switch(ch) 
	{
		case '1':
			/*Select 4 colors*/
			num_colors=FOUR_COLORS;
			index_num_color = 0;
			break;
		case '2':
			/*Select 5 colors*/
			num_colors=MATRIX_EIGHT;
			index_num_color = 1;
			break;
		case '3':
			/*Select 6 colors*/
			num_colors=MATRIX_TEN;
			index_num_color = 2;
			break;
		case '4':
			/*Select 7 colors*/
			num_colors=MATRIX_TWELVE;
			index_num_color = 3;
			break;
		case '5':
			/*Select 8 colors*/
			num_colors=MATRIX_FOURTEEN;
			index_num_color = 4;
			break;
		default:
			return ERROR;
	}
	time2 = timer_ticks;
	sgenrand(time2-time1);
	max_iterations = max_iter[index_board][index_num_color];	
	return OK; 
}

void get_game_options_page()
{
	int row,column;
	int row1;
	/*Display Author name */ 
	row = PANEL_Y - 8;
	column = PANEL_X -5 ;
	put_str(row,column,"Select following options(one by one) for both(1-5):");
	
	/* Display the fields of last 5 games */
	row = row + 2;
	column = PANEL_X -20 ;
	put_str(row,column,"Choose Board Size(1-5)");
	
	row1 = row;
	
	row1++;
	put_str(row1,column,"1) 6 x 6");

	row1++;
	put_str(row1,column,"2) 8 x 8");

	row1++;
	put_str(row1,column,"3) 10 x 10");
	
	row1++;
	put_str(row1,column,"4) 12 x 12");
	
	row1++;
	put_str(row1,column,"5) 14 x 14");

	column += 40;
	put_str(row,column,"Choose Number Colors(1-5)");
	
	row1 = row;
	
	row1++;
	put_str(row1,column,"1) 4 colors");

	row1++;
	put_str(row1,column,"2) 5 colors");

	row1++;
	put_str(row1,column,"3) 6 colors");
	
	row1++;
	put_str(row1,column,"4) 7 colors");
	
	row1++;
	put_str(row1,column,"5) 8 colors");


	/*EDIT:Add data in sometime */
	while(select_board_size_and_color()!= OK)
	{
		continue;
	}
}

void handle_finish()
{
	set_cursor(end_y + 5, start_x-10);
	printf("Successfully completed. Press 'r' to restart the game");
}

void handle_fail()
{
	set_cursor(end_y + 5, start_x-10);
	printf("Unsuccessfull Attempt. Press 'r' to restart the game");
}

void game_run()
{
	/* TC 1 */
	while (TRUE) 
	{
		char ch;
		int status;
		/*Show Title page */
		clear_console();
		title_screen();
		/*Wait for user to press 'b' or 'h'*/
		ch = wait_key_press();

		switch (ch)
		{
			case 'b':
				/* Prepare options page */
				clear_console();
				get_game_options_page();
				status = OK;
				break;

			case 'h':
				/* Prepare options page */
				clear_console();
				status = help_menu(FALSE);
				break;

			default:
				status = ERROR;
		}
		if (status != ERROR)
		{
			/* Prepare Game panel page */
			game_state_buf =(uint8_t*) malloc(matrix_len*matrix_wid
					*sizeof(uint8_t));
			clear_console();
			draw_game_panel(matrix_len,matrix_wid,num_colors);
			start = TRUE;
			if (game_index  > 4)
			{
				game_index = 0;
			}
			game_index++;
			while (1) 
			{
				char ch=readchar();
				/* Free state buffer */
				if (ch != ERROR)
					move_cursor(ch);
				if (fail)
				{	
					char ch1='r'; 
					handle_fail();
					wait_char(ch1);
					fail = FALSE;
					curr_user_iteration = 0;
					free(game_state_buf);
					break;
				}
				if (finish)
				{	
					char ch1='r'; 
					handle_finish();
					wait_char(ch1);
					finish = FALSE;
					curr_user_iteration = 0;
					free(game_state_buf);
					break;
				}
				if (quit)
				{	
					quit = FALSE;
					curr_user_iteration = 0;
					free(game_state_buf);
					break;
				}
				if (help)
				{
					start = TRUE;
					pause = FALSE;
					help = FALSE;
				}

				continue;
			}
		}
	}
}
