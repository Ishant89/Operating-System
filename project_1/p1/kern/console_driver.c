/** @file console_driver.c
 *
 *  @brief Implementation of Console device library
 *  
 *  This file contains all library functions required for console
 *  library. Some of the examples are : get_char, putbyte, putbytes
 *  explained below. 
 *
 *  Console is a memory-mapped device with console width = 80 and 
 *  console height = 25.
 *  Cursor is I/O mapped device with 16-bit I/O port.
 *
 *  Cursor can be hidden and shown as per the requirement.
 *  There is a function which will implement this with following 
 *  consideration: 
 *	Total number of elements are : 25 * 80 = 2000.
 *	
 *	Cursor Location : 
 *
 *	Cursor location is the location to which a (row,column) coordinate was
 *	mapped. E.g. Location 0 of cursor was mapped to (0,0) coordinate on console
 *
 *	Memory-mapped Addressing : 
 *
 *	Memory cells has one byte for content and other byte for color. 
 *	Memory cells were linerally mapped to (row,column) co-ordinate of console
 *	E.g. (0,0) corresponed to Base memory of I/O port and increasing in 
 *	row-major form both were linked together. 
 *
 *	Cursor-Memory Mapping:
 *	Cursor (location 0) was mapped to 0,0 coordinate on console and similarly,
 *	memory is mapped to console. Hence, they were related in the same way.
 *	
 *  HIDDEN:
 *	
 *	Cursor is made hidden by adding 2000 to its current value.
 *	therefore, the location of the cursor does not give accurate
 *	infomartion of the memory mapped I/O cell. Moreover, everytime
 *	we perform write to cursor or to memory, we calculate (row,column)
 *	co-ordinate from the location of the cursor. 
 *
 *	If cursor is > 2000 (means hidden), we compute row,column by reducing
 *	the current location by 2000 (which was the location of the cursor before
 *	it was hidden)
 *
 *	Not HIDDEN:
 *	
 *	Cursor is hidden if its location is more than 2000 in this case. 
 *	It is not hidden otherwise. Location is translated to row,column
 *	as explained above. 
 *
 *
 *
 *  There are functions which take care
 *
 *  @author Ishant Dawer (idawer)
 *  @bug No known bugs
 */

#include <p1kern.h> /*Library header file with drivers(timer,keybd,console)*/
#include <stdio.h>/* Std I/O*/
#include <simics.h> /*Simics debugging library */
#include <contracts.h>
#include <stdint.h> /*Contains stdint */
#include <asm.h> /*Contains all assembly funcitons*/
#include <video_defines.h>/* Contains all constants related to console */
#include <string.h>/*Contains string related functions*/
#include <malloc.h>

#define TRUE 1 
#define FALSE 0 
#define ORIGIN_X 0 /*X-coordinate begin */
#define ORIGIN_Y 0 /* Y-cooridnate begin */
#define ERROR -1 /*Error code */
#define SUCCESS 0 /* Success code */
#define BEGIN_COLOR 0x00 /*Color range begin */
#define END_COLOR 0x8F
#define SPACE 0x20 /* Space character */
#define EIGHT 8

#define CLEAR_CHAR ((FGND_WHITE)|(BGND_BLACK)) /* Clear character */

/* Terminal color vairable */
/** @brief Terminal color variable */
uint8_t terminal_color = FGND_WHITE | BGND_BLACK; /*Default terminal color*/

/** @brief Hide location of cursor
 *
 * This macro defines the first hidden location for cursor
 *  i.e 2000 in this case
 *
 * @return Location
 */

#define HIDE_LOC_BEGIN ((CONSOLE_WIDTH) * (CONSOLE_HEIGHT)) 

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

/** @brief Cursor is hidden 
 *
 * This function is to determine if the cursor is hidden 
 * or not by looking at the value of the location from 
 * cursor I/O ports. 
 * If location is above 2000, then cursor is hidden   
 *
 * @return True or False
 */

int is_cursor_hidden()
{
	uint16_t msb,lsb;
	uint16_t location;
	outb(CRTC_IDX_REG,CRTC_CURSOR_LSB_IDX);
	lsb =  inb(CRTC_DATA_REG);
	outb(CRTC_IDX_REG,CRTC_CURSOR_MSB_IDX);
	msb =  inb(CRTC_DATA_REG);

	location = (msb << EIGHT) + lsb;
	if ( location >= HIDE_LOC_BEGIN)
		return TRUE;
	else 
		return FALSE;
}

/** @brief Get cursor original location 
 *
 * This is used to get actual location of the cursor based 
 * on the (row,column) coordinates. It takes into account the fact 
 * that if cursor is hidden, then it adds 2000 while computing the 
 * ideal location from coordinates
 *
 * @param row row (y-coordinate)
 *
 * @param col column (x co-ordinate)
 *
 * @return int Location of the cursor
 */

int get_cursor_location(int row, int column)
{
	ENSURES( row < CONSOLE_HEIGHT && column < CONSOLE_WIDTH);
	int location ;

	location = row * CONSOLE_WIDTH  + column ;
	if (is_cursor_hidden()) 
	{
		/* location cannot be greater than 2000 when 
		 * computed from co-ordinates */
		ASSERT(location < HIDE_LOC_BEGIN);
		location += HIDE_LOC_BEGIN;
		//lprintf("Yes hidden and loc is %d\n",location);
	}
	REQUIRES(location >= 0 && location < HIDE_LOC_BEGIN);
	return location;
}

/** @brief Get Console/Memory co-ordinates from cursor location 
 *
 * This function is used to fetch the co-ordinates of the 
 * memory from the location of the cursor 
 * 
 * Gets the row from the same location where cursor is 
 * @param location 16-bit integer (Cursor location)
 *
 * @return short 16-bit integer (LSB is column,MSB is row)
 */

short get_row_coordinate_console(int location)
{
	ENSURES(location >= 0  && location <= HIDE_LOC_BEGIN);
	short row;
	/* Since cursor appears after the text, we require co-ordinates 
	 * one location before the cursor*/
	if (is_cursor_hidden()) 
	{ 
		/* location cannot be smaller than 2000 if hidden*/
		ASSERT(location >= HIDE_LOC_BEGIN);
		location -= HIDE_LOC_BEGIN;
	}
	/* EDIT: Replace 0 with macro */
	if (location <0 ) 
		return 0;
	/* If location exceeds the maximum */
	if (location > HIDE_LOC_BEGIN)
		return (CONSOLE_HEIGHT -1 );
	row = location  /CONSOLE_WIDTH ;

	REQUIRES(row >= 0 );
	return row;
}

/** @brief Get Co-ordinates from the location 
 *
 * This function is used to fetch the co-ordinates of the 
 * memory from the location of the cursor 
 * 
 * Gets column from the same location as cursor  
 *
 * If cursor is hidden, cursor's location is translated to its ideal
 * location by subtracting 2000 in this case and gets the co-ordinates.
 * @param location 16-bit integer (Cursor location)
 *
 * @return short 16-bit integer (LSB is column,MSB is row)
 */

short get_column_coordinate_console(int location)
{
	ENSURES(location >= 0 && location <= HIDE_LOC_BEGIN);
	short column;
	/* Since cursor appears after the text, we require co-ordinates 
	 * one location before the cursor*/
	if (is_cursor_hidden()) 
	{
		/* location cannot be smaller than 2000 if hidden*/
		ASSERT(location >= HIDE_LOC_BEGIN);
		location -= HIDE_LOC_BEGIN;
	}


	/* Location == 0 means console is clear with no character */
	if (location < 0 )
		return 0;
	/* If location is > max */
	if (location > HIDE_LOC_BEGIN)
		return (CONSOLE_WIDTH -1);
	column = location %CONSOLE_WIDTH ;

	REQUIRES(column >= 0 );
	return column;
}

/** @brief Get current location of the cursor
 *
 * This function is used to find the current location of
 * the cursor and it reads from the cursor ports 
 *
 * This gives unreformed location (both hidden or show)
 * @return uint16_t
 */

uint16_t get_current_cursor_loc()
{
	uint16_t msb,lsb,location;
	outb(CRTC_IDX_REG,CRTC_CURSOR_LSB_IDX);
	lsb = inb(CRTC_DATA_REG);
	outb(CRTC_IDX_REG,CRTC_CURSOR_MSB_IDX);
	msb = inb(CRTC_DATA_REG);
	
	location = (msb << EIGHT) + lsb;
	return location;
}

/** @brief Write cursor ports for location 
 *
 * This function is used to write cursor based on 
 * the location of the cursor. 
 * 
 * Writes to both cursor location (both hidden or show)
 *
 * @param location Location of the cursor
 * 
 * @return void 
 */

void write_cursor_IO_ports(uint16_t location)
{
	ENSURES(location< 2*HIDE_LOC_BEGIN);
	uint16_t msb,lsb;
	msb = GET_MSB(location);
	lsb = GET_LSB(location);
	/* writing Data register for LSB*/
	outb(CRTC_IDX_REG,CRTC_CURSOR_LSB_IDX);
	outb(CRTC_DATA_REG,lsb);
	/* writing Data register for MSB*/
	outb(CRTC_IDX_REG,CRTC_CURSOR_MSB_IDX);
	outb(CRTC_DATA_REG,msb);
}

/** @brief write cursor at a coordinate
 *
 * This function set the cursor at the 
 * particular (row,column) co-ordinate.
 *
 * @param row Row
 *
 * @param column Column 
 *
 * @return void 
 */


void write_cursor_IO_coordinates(uint16_t row,uint16_t column)
{
	/* Get the location of the cursor 
	 * get_cursor_location() API will give the cursor of the location < 2000
	 * if not hidden 
	 * get_cursor_location() API will give the cursor of the location > 2000
	 * if hidden 
	 * */
	ENSURES(row >= 0 && column >= 0);
	ENSURES(row < CONSOLE_HEIGHT && column < CONSOLE_WIDTH);
	uint16_t location = get_cursor_location(row,column);
	//lprintf("current location is %d\n",location);
	write_cursor_IO_ports(location);	
}


/** @brief write video manager memory 
 *
 * This function is used to write to a specific memory location
 * of the memory-mapped console I/O when a co-ordinate of that
 * console is given.
 *
 * @param row Row (x-coordinate)
 * @param column Column (y-coordinate)
 *
 * @return void 
 */
void write_video_memory_column(int row, int column, char data,int color)
{
	ENSURES(row >= 0 && column >= 0);
	ENSURES(row < CONSOLE_HEIGHT && column < CONSOLE_WIDTH);
	int location = get_cursor_location(row,column);
	if (location >= HIDE_LOC_BEGIN )
		location -= HIDE_LOC_BEGIN;
	
	/* EDIT: Replace 2 with macro */
	/* Writing character at location*/
	*(char *)(CONSOLE_MEM_BASE + 2*location) = data;
	/* Specifying the color at the same location */
	*(char *)(CONSOLE_MEM_BASE + 2*location + 1 ) = color;
}

/** @brief Clear console 
 *
 * This function is used to clear console for n number of 
 * characters starting from address which is an input to 
 * this function
 *
 * @param addr Address
 * @param num_char Characters
 *
 * @return void 
 */

void clear_console_mem(void * addr, int num_char)
{
	/* Change contants to Macro */
	char space = ' ';
	int i = 0 ;
	for(;i < num_char;i++)
	{
		*(char*)(addr + 2*i) = space;
		*(char*)(addr + 2*i + 1) = FGND_WHITE|BGND_BLACK;
	}
}



/** @brief Scroll down 
 *
 * This function is to push the data up when the line wraps
 *
 * It clears the memory contents of first line on the console
 * and shifts the remaining elements in the backward direction.
 * Finally, it frees the last line.
 *
 * @return Void 
 */

void scroll_one_line()
{
	int buflen = 2*(CONSOLE_HEIGHT-1) *(CONSOLE_WIDTH);
	void * buf = malloc(buflen);
	if (buf == NULL)
	{
		return;
	}
	const void * mem_start_cpy = (void *)CONSOLE_MEM_BASE + 2*CONSOLE_WIDTH;

	void * temp = memcpy(buf,mem_start_cpy,buflen);
	if (temp == NULL)
	{
		return;
	}
	temp = memcpy((void *)CONSOLE_MEM_BASE,(const void *)buf,buflen);

	if (temp == NULL)
	{
		return;
	}
	free(buf);
	/*Clearing the last line is pending */
	void * start_mem = (void*) CONSOLE_MEM_BASE + buflen;
	clear_console_mem(start_mem,CONSOLE_WIDTH);
}


/** @brief Handle '\n' character 
 *
 * This function is used to handle a newline character is pressed
 * wrt cursor location 
 * Functions:
 *
 * It increments the row by one and sets the column to zero.
 *
 * @return VOID 
 */

void handle_newline_char()
{
	int location = get_current_cursor_loc();
	uint16_t row,column;
	row = get_row_coordinate_console(location);
	if (row == CONSOLE_HEIGHT -1 )
	{
		scroll_one_line();
	} else 
	{
		row++;
	}
	column  = 0 ;
	write_cursor_IO_coordinates(row,column);
}

/** @brief handle '\r' character 
 *
 * This function is used to handle '\r' character
 * wrt cursor location.
 * Functions: 
 *
 * It sets the current column to zero
 * It keeps the row as it is. 
 *
 * @return VOID 
 */

void handle_carriage_char()
{
	int location = get_current_cursor_loc();
	uint16_t row,column;
	row = get_row_coordinate_console(location);
	column  = 0 ;
	write_cursor_IO_coordinates(row,column);
}

/** @brief handle backspace character
 *
 * This function is used to handle backspace character
 * wrt cursor location.
 * 
 * Functions: 
 * 1. This function decrements the column by 1 
 * 2. This keeps the row as it is.
 * @return VOID 
 */

void handle_backspace_char()
{
	int location = get_current_cursor_loc();
	uint16_t row,column;
	if (location == 0 )
		return;
	location--;
	row = get_row_coordinate_console(location);
	column = get_column_coordinate_console(location);
	write_video_memory_column(row,column,SPACE,terminal_color);
	write_cursor_IO_coordinates(row,column);
}

/** @brief print a normal character 
 *
 * This function is used to print a character other
 * than special characters 
 *
 * Functions: 
 * 1. It finds the location of the cursor
 * 2. Writes character to its location 
 * 3. Increments the cursor location
 *
 * @param char Character 
 *
 * @param color Color 
 *
 * @return void 
 */

void handle_nonspecial_char(char ch,int color)
{
	uint16_t location = get_current_cursor_loc();
	uint16_t row,column;
	row = get_row_coordinate_console(location);
	column = get_column_coordinate_console(location);
	write_video_memory_column(row,column,ch,color);
	if ((row == (CONSOLE_HEIGHT -1)) && (column == (CONSOLE_WIDTH -1)))
	{
		scroll_one_line();
		column = 0;
	} else 
	{
		location++;
		row = get_row_coordinate_console(location);
		column = get_column_coordinate_console(location);
	}
	/* Handle scrolling case */
	write_cursor_IO_coordinates(row,column);
}

/*
 * All declarations and comments are in p1kern.h
 */

/*
 * Console library functions 
 */

/*
 * Writes a character to the cursor location
 */

int putbyte( char ch )
{
	switch(ch)
	{
		case '\n':
			handle_newline_char();
			break;
		case '\r':
			handle_carriage_char();
			break;
		case '\b':
			handle_backspace_char();
			break;
		default:
			handle_nonspecial_char(ch,terminal_color);
			break;
	}
	return ch; 
}

/*
 * Writes a string of length len to the location 
 */

void 
putbytes( const char *s, int len )
{
	int i = 0 ;
	if (s != NULL || len != 0 ) 
	{
		for (;i < len; i++)
		{
			if (i <= strlen(s))
			{
				putbyte(s[i]);
			}
		}
	}
	return ;
}

/*
 * Sets the terminal color after which the 
 * characters will be print of color 'color' 
 */

int
set_term_color( int color )
{
	if ( color >= BEGIN_COLOR && color <= END_COLOR)
	{
		terminal_color = color;
		return SUCCESS;
	}	
	return ERROR;
}

/*
 * Gets the terminal color of the console 
 */

void
get_term_color( int *color )
{
	int color1 = terminal_color;
	*color = color1;
}

/* 
 * set the cursor to console's co-ordinate
 * */
int
set_cursor( int row, int col )
{
	uint16_t location;
	location = get_cursor_location(row,col);
	if (location < 2 * HIDE_LOC_BEGIN)
	{
		write_cursor_IO_ports(location);
		return SUCCESS;
	}
	return ERROR;
}

/*
 * Gets the co-ordinates of the location 
 * where cursor is located 
 */
 
void
get_cursor( int *row, int *col )
{
	uint16_t location = get_current_cursor_loc();
	int row1,column1;
	row1 = get_row_coordinate_console(location);
	column1 = get_column_coordinate_console(location);
	*row = row1;
	*col = column1;
}

/*
 * Hides the cursor by adding 2000 to it
 */
void
hide_cursor()
{
	uint16_t location;
	if (!is_cursor_hidden())
	{
		location = get_current_cursor_loc();
		location += HIDE_LOC_BEGIN;
		write_cursor_IO_ports(location);
	}
}

/*
 * Makes the cursor appear by reducing its value
 * by 2000
 */

void
show_cursor()
{
	uint16_t location;
	if (is_cursor_hidden())
	{
		location = get_current_cursor_loc();
		location = location - HIDE_LOC_BEGIN;
		write_cursor_IO_ports(location);
	}

}

/*
 * Clears the console 
 */

void 
clear_console()
{	
	int number_of_items = CONSOLE_HEIGHT * CONSOLE_WIDTH;
	clear_console_mem((void*)CONSOLE_MEM_BASE,number_of_items);
	/* Set the cursor */
	set_cursor(ORIGIN_Y,ORIGIN_X);
	//lprintf("Curreent loc is %d\n",get_current_cursor_loc());
	//lprintf("isHiden  %d\n",is_cursor_hidden());
}

/*
 * Draws the character with color 'color' at the 
 * location defined by console co-ordinate
 */
void
draw_char( int row, int col, int ch, int color )
{
	if (row < CONSOLE_HEIGHT && col < CONSOLE_WIDTH)
	{
		write_video_memory_column(row,col,ch,color);
	} else 
	{
		//lprintf("Location out of the bounds\n");
	}
}

/*
 * Gets the character from console location 
 */

char
get_char( int row, int col )
{
	uint16_t location;
	char ch;
	location = get_cursor_location(row,col);
	if (is_cursor_hidden())
		location -= HIDE_LOC_BEGIN;
	ch = *(char*)(CONSOLE_MEM_BASE + 2*location);
	return ch;
}
