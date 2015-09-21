/** @file fake.c 
 *
 *  @brief Skeleton implementation of device-driver library.
 *  This file exists so that the "null project" builds and
 *  links without error.
 *
 *  @author Harry Q. Bovik (hbovik) <-- change this
 *  @bug This file should shrink repeatedly AND THEN BE DELETED.
 */

#include <p1kern.h>
#include <stdio.h>
#include <simics.h>
#include <contracts.h>
#include <stdint.h>
#include <asm.h>
#include <video_defines.h>
#include <string.h>
#include <malloc.h>

#define TRUE 1 
#define FALSE 0 
#define ORIGIN_X 0
#define ORIGIN_Y 0
#define ERROR -1
#define SUCCESS 0
#define BEGIN_COLOR 0x00
#define END_COLOR 0x8F
#define SPACE 0x20

#define CLEAR_CHAR ((FGND_WHITE)|(BGND_BLACK))

/* Terminal color vairable */
uint8_t terminal_color = FGND_WHITE | BGND_BLACK;

/** @brief Hide location of cursor
 *
 * This macro places the cursor at the location
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
 * or not 
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

	/* EDIT:Replace 8 with macro */
	location = (msb << 8) + lsb;
	////lprintf("Location is %d\n",location);
	if ( location >= HIDE_LOC_BEGIN)
		return TRUE;
	else 
		return FALSE;
}

/** @brief Get cursor ideal location 
 *
 * This is used to get ideal location of the cursor based 
 * on the (row,column) coordinates
 *
 * @param row row (y-coordinate)
 *
 * @param col column (x co-ordinate)
 *
 * @return Int
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

/** @brief Get Co-ordinates from the location 
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


	/* EDIT: Replace 0 with macro*/
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
 * the cursor 
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
	
	/*EDIT: Replace 8 with macro */
	location = (msb << 8) + lsb;
	return location;
}

/** @brief Write cursor ports for location 
 *
 * This function is used to write cursor based on 
 * the location 
 *
 * @param location Location of the cursor
 * 
 * Writes to unreformed location (both hidden or show)
 * @return Void 
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
 * This function is used to set cursor at a particular 
 * coordinate 
 *
 * @param row Row
 *
 * @param column Column 
 *
 * @return Void 
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
 * when a co-ordinate is given 
 *
 * @param row Row (x-coordinate)
 * @param column Column (y-coordinate)
 *
 * @return Void 
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
 * characters starting from address 
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
 * @return Void 
 */

void scroll_one_line()
{
	int buflen = 2*(CONSOLE_HEIGHT-1) *(CONSOLE_WIDTH);
	void * buf = malloc(buflen);
	if (buf == NULL)
	{
		return;
		//lprintf("Error in memory allocation\n");
	}
	const void * mem_start_cpy = (void *)CONSOLE_MEM_BASE + 2*CONSOLE_WIDTH;
	/*Error handling later */

	void * temp = memcpy(buf,mem_start_cpy,buflen);
	if (temp == NULL)
	{
		//lprintf("Memory to buffer failed \n");
		return;
	}
	temp = memcpy((void *)CONSOLE_MEM_BASE,(const void *)buf,buflen);

	if (temp == NULL)
	{
		//lprintf("Memory to buffer failed \n");
		return;
	}
	free(buf);
	/*Clearing the last line is pending */
	void * start_mem = (void*) CONSOLE_MEM_BASE + buflen;
	clear_console_mem(start_mem,CONSOLE_WIDTH);
}


/** @brief Handle '\n' character 
 *
 * This function is used to handle the case when newline is pressed
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
 *
 * @return VOID 
 */

void handle_backspace_char()
{
	int location = get_current_cursor_loc();
	uint16_t row,column;
	if (location == 0 )
		return;
	location--;
	////lprintf("location is %d\n",location);
	row = get_row_coordinate_console(location);
	column = get_column_coordinate_console(location);
	/*EDIT: Change space to macro and terminal color to static macro
	 * in case of backspace */
	write_video_memory_column(row,column,SPACE,terminal_color);
	write_cursor_IO_coordinates(row,column);
}

/** @brief print a normal character 
 *
 * This function is used to print a character other
 * than special characters 
 *
 * @param char Character 
 *
 * @param color Color 
 *
 * @return Void 
 */

void handle_nonspecial_char(char ch,int color)
{
	uint16_t location = get_current_cursor_loc();
	////lprintf("location is %d\n",location);
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

void
get_term_color( int *color )
{
	int color1 = terminal_color;
	*color = color1;
}

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

void test()
{
}
