/** @file traceback.c
 *  @brief Definition for traceback function
 *
 *  This file contains traceback function (A traceback library) which
 *  prints down the stack trace of the program when it is called. This
 *  stack crawler has following functionalities : 
 *
 *  1. When called, it lists down all the functions on the stack in the
 *  order of the recently called function to the first function called.
 *  2. It displays the function names and their arguments & values.
 *  3. It can  handle following arguments datatypes :
 *	   a). Int
 *	   b). Float
 *	   c). Double
 *	   d). String (char *)
 *	   e). String Array (char **)
 *	   f). Unknown 
 *	   g). Voidstar 
 *	   h). Char 
 *
 *
 *	Refer: ./README.dox for more details about the design
 *  @author Ishant Dawer (idawer)
 *  @bug No known bugs
 */

#include "traceback_internal.h" /*contains declarations for functsym_t
								  ,argsym_t*/
#include "get_ebp_info.h" /* contains functions: get_ebp()*/
/*
 * contains wrapper functions for syscalls such as
 * Sigprocmask etc
 * */
#include "user_functions_wrapper.h" 
#include<stdlib.h> /*standard c library: malloc etc */
#include<signal.h> /*signal library: Sigprocmask etc*/
#include<errno.h> /* errno strings & enum */
#include<string.h> /*string library */
#include<setjmp.h> /* fns:setjmp,sigsetjmp,siglongjmp*/
#include<ctype.h> /* fns : isprint */
#include<unistd.h> /* contains write system call */


/* General purpose Macros */
#define TRUE 1 /*Success code */
#define FALSE 0 /*Failure code */
#define STOP -1 /* Condition to stop */
#define ERROR -1 /* Error code */
#define ADDR_LOC_32 4 /*Size of pointer in bytes*/
#define MAX_TRACEBACK_ENTRY_SIZE 1024/*Max size of one traceback entry*/
#define ARG_STRING_SIZE 256 /*Max string size for an argument */
#define MAX_STRING_SIZE 29 /*Max value of a string value */
#define MAX_STRING_WO_DOTS 25 /*Max value of a string wo dots*/
#define SIG_SET_ENV 1 /*Sigsetjmp buf value */

/** @brief Retrieve previous frame address.
 *
 * This macro helps in getting previous frame address
 * based on the current base pointer. 
 * For a valid frame, current base pointer points to previous
 * base pointer.
 *
 *  @param curr current base pointer 
 *
 *  @return Previous frame's base pointer
 */

#define PREV_FRAME_PTR(curr) (unsigned long *)(*((unsigned long *)curr))

/** @brief Get return address for the caller site
 *
 * This helps in retrieving the caller site's address from the 
 * stack.
 *
 * @param ebp Base pointer of the frame
 *
 * @return Return address of the caller function
 */

#define GET_RET_ADDR(ebp) (void *)(*(((unsigned long *)(ebp)) + 1))

/* Buffer for setjmp and longjmp */
sigjmp_buf buf;

/* Sigset to save the previous state of signal set */
sigset_t old_set;

/* Sigaction to save the old action of SIGSEGV handler */
struct sigaction old_act;

/** @brief Memory address checker
 * 
 * This function scans through the memory beginning from
 * address: addr and checks if address are valid for "num_bytes" 
 * number of bytes.
 *
 * What it does :
 * 1. It fetches the value byte by byte starting from address: addr
 * 2. It defines the setjmp points 
 * 3. If segfault occurs, handler will set the value and return back
 * to setjmp point
 * 4. Accordingly, return status is set
 *
 * @param addr Beginning address 
 *
 * @param num_bytes Number of bytes to scan
 *
 * @return  True if valid
 * @return  False if invalid
 *  
 */
int check_if_addr_valid(void * addr,int num_bytes)
{
	/*TC : Size shd be > 1 & addr should be valid */
	REQUIRES(num_bytes >= 1 && addr != NULL);
	int i = 0 ;
	for (; i < num_bytes ; i++)
	{
		if (!sigsetjmp(buf,SIG_SET_ENV))
		{
			/* Assess the memory to raise segfault if invalid*/
			char value = *(char *)(addr + i);
			value = value;
		} else 
		{
			return FALSE;
		}
	}
    return TRUE;
}

/** @brief Memory write checker test
 *
 * This function is a checker which writes a byte into the memory
 * pointer by previous frame's base pointer (evaluated from current
 * base pointer). Following are the events discussed : 
 * 
 * 1. This check is only done when frame is invalid (current base pointer
 * is higher than previous frame's base pointer) to differentiate
 * between 2 scenarios (report a Fatal error if frame is valid or 
 * stop iterating down the stack after stack is traversed completely).
 * 
 * 2. If this check passes, stack is traversed completely else frame is 
 * invalid.
 *
 * 3. It saved the memory content before writing and then restore it.
 *
 * @param base_ptr Current frame's base pointer
 *
 * @return TRUE if stack is traversed completely 
 * @return FALSE if invalid frame
 */

int is_protected_mem_writable(void * base_ptr)
{
	void ** prev_frame_ptr = (void *)PREV_FRAME_PTR(base_ptr);
	char value_restore ;
	if (!sigsetjmp(buf,SIG_SET_ENV))
	{
		value_restore = *(char*)prev_frame_ptr;
		memset(*prev_frame_ptr,0,sizeof(char));
	} else 
	{
		return TRUE;
	}
	memset(*prev_frame_ptr,value_restore,sizeof(char));
	return FALSE;
}


/**
 * @brief Stack frame validity checker
 *
 * This checker is used to validate if a frame in the stack is valid.
 * As per the stack conventions, current frame's base_ptr (ebp) points to 
 * frame's base_ptr (ebp). Rule for validity (implemented here) are as follows:
 *
 * 1. Current frame's base pointer should be a valid accessible memory (Should
 * not give any fault while accessing the previous base pointer).
 *
 * 2. Current base_pointer (ebp) should be lower than previous base_ptr 
 * (prev_ebp).
 *
 * 3. If step 2 fails, it checks for memory write check (as defined in 
 * is_protected_mem_writable() function ) in mem location defined by 
 * previous ebp. If this check passes, it gives back STOP return value
 * to traceback function to stop traversing further as stack has been 
 * traversed fully. (More info in README.dox) 
 *   
 *  
 * Return code : 
 *    If frame is not valid, it returns False 
 *    If frame is valid, it returns True
 *    If frame is not valid and stack is traversed completely,
 *    it returns Stop
 *
 * @param base_ptr Current frame's base pointer
 *
 * @return True, frame valid
 * @return False : frame invalid 
 * @return Stop : End of the stack 
 */


int check_if_frame_valid(void * base_ptr)
{
	/* TC: Base ptr should never be null */
	ENSURES(base_ptr != NULL);
	
	int is_addr_valid = FALSE ;
	unsigned long * curr_frame_ptr = (unsigned long *) base_ptr,
				  * prev_frame_ptr = NULL;

	/* 
	 * Check if base pointer is a valid address 
	 * and should be able to retrieve previous frame base pointer
	 * (4 bytes of memory)
	 * */
	is_addr_valid = check_if_addr_valid(base_ptr,ADDR_LOC_32);

	if (is_addr_valid)
	{
		/* Base pointer comparison */
		prev_frame_ptr = PREV_FRAME_PTR(curr_frame_ptr);
		if (prev_frame_ptr > curr_frame_ptr)
		{
			return TRUE;
		} else 
		{
			/* Memory write check to find out Termination
			 * condition (STOP) */
			if (is_protected_mem_writable(base_ptr))
				return STOP;
			return FALSE;
		}
	} else 
	{
		   return STOP;
	} 
}

/** @brief Get function index by address (function name,type)
 * 
 * This function is used to fetch function details such as name,type
 *  & args based on return address. It iterates over all the items in
 *  the functions list and computes the difference between return address
 *  and item.address. The smallest difference (which is less than 1M) is
 *  considered the item (function) from where call happened. 
 *  
 *  Returns -1 in case no entry is found. 
 *  @param ret_addr Return address from the stack 
 *
 *  @return index of the function (in Functions list)
 */

int get_func_index_by_ret_addr(void * ret_addr)
{
	/*TC: ret_addr cannot be zero*/

	ENSURES(ret_addr != NULL);
	int i = 0,prev_addr_diff = 0,index=-1,
		curr_addr_diff;
	for (;strlen(functions[i].name) != 0 && i < FUNCTS_MAX_NAME; i++)
	{
		/* difference */
		curr_addr_diff = (unsigned int)ret_addr - 
			(unsigned int)functions[i].addr;
		/* difference should be > 0 and < 1M */
		if (curr_addr_diff > 0  && curr_addr_diff <= 
				MAX_FUNCTION_SIZE_BYTES) 
		{
			/* 
			 * if diff is 0, iterate again 
			 * as return address and function address
			 * cannot point to same location
			 */
		   if ( prev_addr_diff == 0 ) 
		   {
			   prev_addr_diff = curr_addr_diff;
			   index = 0 ;
			   continue;
		   }
		   if (curr_addr_diff < prev_addr_diff)
		   {
			   index = i;
			   prev_addr_diff = curr_addr_diff;
		   }
		}
	}        
	return index;
}

/** @brief Handling char argument in the function
 *
 * This is a char argument handler which outputs 
 * the argument in the format "char <name>=<value>".
 *
 * What it does:
 * 1. Takes the pointer where it concatenates the resultant char arg
 * string.
 * 2. Check if the argument pointer from stack is a valid accessible
 * memory otherwise returns null if memory is invalid which is a FATAL
 * error. 
 * 3. Checks if character is printable otherwise value is printed as
 * octal escaped.
 * 
 * @param entry Pointer where string gets concatenated
 * @param ptr  Pointer to argument(char) on Stack 
 * @param  name Name of the function
 *
 * @return TRUE or False
 */

int handle_char(char * entry,void * ptr,const char * name)
{
	char value;
	if (check_if_addr_valid(ptr,sizeof(char)))
	{
		value = *(char*)ptr;
		/* Check if char is printable */
		if (isprint(value))
		{
			snprintf(entry,ARG_STRING_SIZE,
					"char %s='%c',",name,value);
		} else 
		{
			/* Print escaped octal character */
			snprintf(entry,ARG_STRING_SIZE,
					"char %s='\\%o',",name,value);
		}
		return TRUE;
	} else 
	{
		return FALSE;
	}
}

/** @brief Handling int argument in the function
 *
 * This is a int function argument handler which outputs 
 * the argument in the format "int <name>=<value>".
 *
 * What it does:
 * 1. Takes the pointer where it concatenates the resultant int arg
 * string.
 * 2. Check if the argument pointer from stack is a valid accessible
 * memory otherwise returns null if memory is invalid which is a FATAL
 * error. 
 * 
 * @param entry Pointer where string gets concatenated
 * @param ptr  Pointer to argument(int) on Stack 
 * @param  name Name of the function
 *
 * @return TRUE or False
 */


int handle_int(char * entry,void * ptr,const char * name)
{
	int value;
	if (check_if_addr_valid(ptr,sizeof(int)))
	{
		value = *(int *) ptr;
		snprintf(entry,ARG_STRING_SIZE,
				"int %s=%d,",name,value);
		return TRUE;
	}
	return FALSE;
}

/** @brief Handling float argument in the function
 *
 * This is a float function argument handler which outputs 
 * the argument in the format "float <name>=<value>".
 *
 * What it does:
 * 1. Takes the pointer where it concatenates the resultant float arg
 * string.
 * 2. Check if the argument pointer from stack is a valid accessible
 * memory otherwise returns null if memory is invalid which is a FATAL
 * error. 
 * 
 * @param entry Pointer where string gets concatenated
 * @param ptr  Pointer to argument(float) on Stack 
 * @param  name Name of the function
 *
 * @return TRUE or False
 */


int handle_float(char * entry,void * ptr,const char * name)
{
	float value;
	if (check_if_addr_valid(ptr,sizeof(float)))
	{
		value = *(float *) ptr;
		snprintf(entry,ARG_STRING_SIZE,
				"float %s=%f,",name,value);
		return TRUE;
	}
	return FALSE;
}

/** @brief Handling double argument in the function
 *
 * This is a double function argument handler which outputs 
 * the argument in the format "double <name>=<value>".
 *
 * What it does:
 * 1. Takes the pointer where it concatenates the resultant double arg
 * string.
 * 2. Check if the argument pointer from stack is a valid accessible
 * memory otherwise returns null if memory is invalid which is a FATAL
 * error. 
 * 
 * @param entry Pointer where string gets concatenated
 * @param ptr  Pointer to argument(double) on Stack 
 * @param  name Name of the function
 *
 * @return TRUE or False
 */
int handle_double(char * entry,void * ptr,const char * name)
{
	double value;
	if (check_if_addr_valid(ptr,sizeof(double)))
	{
		value = *(double *) ptr;
		snprintf(entry,ARG_STRING_SIZE,
				"double %s=%lf,",name,value);
		return TRUE;
	}
	return FALSE;
}

/** @brief String argument print conditions handler
 *
 * This function fulfils all the reqd conditions when the argument
 * of the function is a string which is described in following 
 * actions:
 *
 * 1. Takes in input as the base address of the string
 * 2. Starting with the base address, it scans for invalid addr and
 * printable character till it encounters a null character.
 * 3. If invalid address or non-printable, then the return string is 
 * "char *<name> = Address of the string.
 * 4. Otherwise string will be printed, char *<name>= "STRING"
 * 5. Then it checks if the length of the string is more than 25 chars
 * If yes, it prints tilll 25 chars and appends "..." after it.
 *
 * @param input Address of the string to be sanitized
 *
 * @return Sanitized string (char *NAME="STRING" or 
 * char *NAME=0xAddress)
 *  
 *
 */

char * verify_string_conditions(char * input)
{
	char ch;
	char *  output_str = (char *) Malloc(MAX_STRING_SIZE); 
	char *  temp_str = (char *) Malloc(MAX_STRING_SIZE); 
	int i = 0,error=0;
	while (!error)
	{
		if (check_if_addr_valid(input + i,sizeof(char)))
		{
			ch = input[i];
			if (ch != '\0')
			{
				/* check if char is printable */
				if (isprint(ch))
				{
					if (i < MAX_STRING_WO_DOTS)
						output_str[i]=ch;
				} else 
				{
					error = TRUE;
					/* print address if not printable*/
					snprintf(output_str,MAX_STRING_SIZE,
							"%p",input);
					break;
				}
				i++;
			} else 
			{
				break;
			}

		} else 
		{
			/* print address if not printable*/
			snprintf(output_str,MAX_STRING_SIZE,
						    "%p",input);
			error = TRUE;
		}
	}
	if (error)
	{
		return output_str;
	} else 
	{
		if (i > MAX_STRING_WO_DOTS -1)
			/* append dots if strlen > 25 */
			for (i = MAX_STRING_WO_DOTS; i < MAX_STRING_SIZE-1;i++)
			{
				output_str[i]='.';
			}
		output_str[i] = '\0';
		snprintf(temp_str,ARG_STRING_SIZE,"\"%s\"",output_str);
		free(output_str);
		return temp_str;
	}
}

/** @brief Handling string argument in the function
 *
 * This is a string function argument handler which outputs 
 * the argument in the format "char *<name>=<value>".
 *
 * What it does:
 * 1. Takes the pointer where it concatenates the resultant string arg
 * string.
 * 2. Check if the argument pointer from stack is a valid accessible
 * memory otherwise returns null if memory is invalid which is a FATAL
 * error. 
 * 
 * @param entry Pointer where string gets concatenated
 * @param ptr  Pointer to argument(char*) on Stack 
 * @param  name Name of the function
 *
 * @return TRUE or False
 */


int handle_string(char * entry,void ** ptr,const char * name)
{
	void * value;
	char * output_str;
	if (check_if_addr_valid(ptr,sizeof(char *)))
	{
		value =  *ptr;
		output_str = verify_string_conditions((char *)value);
		snprintf(entry,ARG_STRING_SIZE,"char *%s=%s,",name,output_str);
		free(output_str);
		return TRUE;
	}
	return FALSE;
}

/** @brief Handling string array argument in the function
 *
 * This is a string array function argument handler which outputs 
 * the argument in the format "char **<name>={"string1","string2"}".
 *
 * What it does:
 * 1. Takes the pointer where it concatenates the resultant string array 
 * arg string.
 * 2. Check if the argument pointer from stack is a valid accessible
 * memory otherwise returns null if memory is invalid which is a FATAL
 * error.
 * 3. Then it will get either a string or its address  using function 
 * verify_string_conditions() depending upon if string is valid and each
 * character in the string has a valid address.
 * 4. If one of the string element in the string array has an invalid 
 * address, it prints the address (char**) of that string and stops
 * 5. If it encounters a null pointer, it stops traversing further.
 * 6. If string array has more than 4 strings, it should append "..."
 * in the output.
 *
 * @param entry Pointer where string gets concatenated
 * @param ptr  Pointer to argument(char**) on Stack 
 * @param  name Name of the function
 *
 * @return TRUE or False
 */
int handle_string_array(char * entry,void *** ptr,const char * name)
{
	void ** value;
	int i = 0,error = 0 ;
	char * string_entry;
	char * iterator;
	snprintf(entry,ARG_STRING_SIZE,"char **%s={",name);
	/* check address */
	if (check_if_addr_valid(ptr,sizeof(char **)))
	{
		value = * ptr;
		while (TRUE)
		{
			int result = check_if_addr_valid(value + i,
					sizeof(char*)) && value[i] != NULL;
			if (result) 
			{
				/* Get the sanitized string or address for addr
				 * value + i */
				string_entry = verify_string_conditions(
						(char*)value[i]);
				/*moving ahead in ptr by string length*/ 
				iterator = entry + strlen(entry);
				if (i < 3) 
				{
					if (string_entry != NULL)
					{
					    snprintf(iterator,ARG_STRING_SIZE,"%s,",string_entry);
						/* Freeing the memory*/
						free(string_entry);
					} 

				} else 
				{
					snprintf(iterator,
							ARG_STRING_SIZE,"...");
					break;
				}
				i++;
			} else 
			{
				if (!check_if_addr_valid(value + i,sizeof(char*)))
				{
					iterator = entry + strlen(entry);
			/*print the address if string is invalid */
					snprintf(iterator,ARG_STRING_SIZE,
							"%p",value + i);
				}
				break;
			}
		}
		if (i >= 3) 
			iterator = entry + strlen(entry);
		else 
			iterator = entry + strlen(entry) -1;
		snprintf(iterator,ARG_STRING_SIZE,"},");
	} else 
	{
		error = TRUE;
	}
	if (error)
		return FALSE;
	else
		return TRUE;
}

/** @brief Handling voidstar argument in the function
 *
 * This is a string function argument handler which outputs 
 * the argument in the format "void *<name>=<value>".
 *
 * What it does:
 * 1. Takes the pointer where it concatenates the resultant void* arg
 * string.
 * 2. Check if the argument pointer from stack is a valid accessible
 * memory otherwise returns null if memory is invalid which is a FATAL
 * error. 
 * 
 * @param entry Pointer where string gets concatenated
 * @param ptr  Pointer to argument(void*) on Stack 
 * @param  name Name of the function
 *
 * @return TRUE or False
 */
int handle_voidstar(char * entry,
					unsigned int * ptr,
					const char * name)
{
	unsigned int value;
	if (check_if_addr_valid(ptr,sizeof(void*)))
	{
		value = *ptr;
		snprintf(entry,ARG_STRING_SIZE,
				"void *%s=0v%x,",name,value);
		return TRUE;
	}
	return FALSE;
}

/** @brief Handling unknown argument in the function
 *
 * This is a string function argument handler which outputs 
 * the argument in the format "UNKNOWN <name>=<value>".
 *
 * What it does:
 * 1. Takes the pointer where it concatenates the resultant void* arg
 * string.
 * 2. Check if the argument pointer from stack is a valid accessible
 * memory otherwise returns null if memory is invalid which is a FATAL
 * error. 
 * 
 * @param entry Pointer where string gets concatenated
 * @param ptr  Pointer to argument(void*) on Stack 
 * @param  name Name of the function
 *
 * @return TRUE or False
 */

int handle_unknown(char * entry,
				   void ** ptr,
				   const char * name)
{
	void * value;
	if (check_if_addr_valid(ptr,sizeof(void*)))
	{
		value = *ptr;
		snprintf(entry,ARG_STRING_SIZE,
				"UNKNOWN %s=%p,",name,value);
		return TRUE;
	}
	return FALSE;
}

/** @brief Get a string of all args and their values
 *
 * This function is to assemble all arguments of a function
 * their values and types together in a string.
 * Output: "Function NAME(TYPE NAME1=VALUE,....)"
 *
 * What it does:
 *
 * 1. Get the previous base pointer 
 * 2. get the list of arguments.
 * 3. Iterate over all the arguments and do the following:
 *		a). Find the type
 *		b). Call the handler
 *		c). Check the status from handler
 *		d). If TRUE, return the string
 *		e). If FALSE, FATAL error
 * 4. If the function index in the functions is -1,then the return
 * from the this function is "Function 0xAddress(...)" as no entry 
 * in the functions table.
 * 5. If argument list for a function is empty, then it returns the
 * string as Function NAME(void)
 *
 * @param base_ptr Base pointer of the frame
 * @param functions_index Index in the functions table
 *
 * @return Traceback entry for a function 
 * @return NULL in case of an error
 */


char * get_args_and_values_list(void * base_ptr, 
		int functions_index)
{
	int i=0,success= FALSE;
	void * ptr;
	/* get the previous frame base pointer */
	void * prev_frame_ptr = (void *) PREV_FRAME_PTR(base_ptr);
	/* Get the args list for functions with functions_index */
	const argsym_t * args_list = functions[functions_index].args;
	/* get the name of the function */
	const char * name = functions[functions_index].name;
	char * traceback_entry = (char * ) Malloc(MAX_TRACEBACK_ENTRY_SIZE);
	char * entry_temp;
	/* Get the return address wrt base_pointer */
    void * ret_addr = GET_RET_ADDR(base_ptr);

	/* Case when caller site address name cannot be found */
	if (functions_index == -1)
	{
		snprintf(traceback_entry,MAX_TRACEBACK_ENTRY_SIZE,
				"Function %p(...), in\n",ret_addr);
		return traceback_entry;
	} 
	snprintf(traceback_entry,MAX_TRACEBACK_ENTRY_SIZE,
			"Function %s(",name);

	/* Iterating over args list */
	for(; strlen(args_list[i].name) != 0 && i < ARGS_MAX_NUM; i++)
	{
		ptr = prev_frame_ptr + args_list[i].offset;
		argsym_t arg = args_list[i];
		entry_temp = traceback_entry + strlen(traceback_entry);
		switch(arg.type)
		{
			case TYPE_CHAR:
				success = handle_char(entry_temp,ptr,arg.name);
				break;
			case TYPE_INT:
				success = handle_int(entry_temp,ptr,arg.name);
				break;
			case TYPE_FLOAT:
				success = handle_float(entry_temp,ptr,arg.name);
				break;
			case TYPE_DOUBLE:
				success = handle_double(entry_temp,ptr
						,arg.name);
				break;
			case TYPE_STRING:
				success = handle_string(entry_temp,(void **)ptr
						,arg.name);
				break;
			case TYPE_STRING_ARRAY:
				success = handle_string_array(entry_temp,
						(void***)ptr,
						arg.name);
				break;
			case TYPE_VOIDSTAR:
				success = handle_voidstar(entry_temp,
						(unsigned int *)ptr
						,arg.name);
				break;
			case TYPE_UNKNOWN:
				success = handle_unknown(entry_temp,(void**)ptr
						,arg.name);
				break;
		}
	}
	/* If there are no args,then args are shown as void */ 
	if (strlen(args_list[0].name) == 0)
	{
		success = TRUE;
		entry_temp = traceback_entry + strlen(traceback_entry);
		snprintf(entry_temp,ARG_STRING_SIZE,"void,");
	}
	if (!success) 
	{
		free(traceback_entry);
		return NULL;
	} else 
	{
		entry_temp = traceback_entry + strlen(traceback_entry)-1;
		snprintf(entry_temp,ARG_STRING_SIZE,"), in\n");
		return traceback_entry;
	}
}

/** @brief Print traceback entries 
 *
 * This function writes the traceback entry to FILE stream
 *
 * @param output_fd File descriptor where to write
 * @param args_details String to be written
 *
 * @return True if successfull otherwise False
 */
 
int print_traceback_entry(int output_fd, char * args_details)
{
	/* EDIT: change 1 to fd */
	int result = write(output_fd,args_details,strlen(args_details));
	free(args_details);
	/*Check if write was successful */
	if (result != -1 )
		return TRUE;
	else 
		return FALSE;
}


/** @brief SIGSEGV signal handler
 *
 * This handler handles the SIGSEGV signal whenever an illegal
 * access to the memory is made. 
 *
 * What it does :
 * 1. It makes a long jump to the localtion where error was made
 * along with saving the signal context
 * 2. Reason for using siglongjump is to restore the signal context
 * after signal is handled and unmask/unblock all the signals including
 * SIGSEGV.
 *
 * @param signal Signal number it recieved
 * @param info Signal info 
 * @param arg Environment argument
 * 
 * @return void
 */


void segfault_handler(int signal, siginfo_t * info, void * arg)
{
	siglongjmp(buf,1);
}

/** @brief Traceback function (Traceback library)
 *
 * This function is responsible for collecting the stack trace 
 * and printing it to the FILE stream pointed by fp.
 *
 * What it does :
 * 1. Saves the old Signal set from the user program
 * 2. Unblocks the SIGSEGV signal for this traceback 
 * library function
 * 3. It installs the new SIGSEGV handler and saves the
 * previous handler 
 * 4. Evaluates the file descriptor from FILE stream
 * 5. Checks the base pointer to see if address is valid
 * 6. Checks if frame is valid
 * 7. If valid, it gets the return address of the caller site
 * 8. If caller site is valid, it extracts the functions index 
 * from the functions table
 * 9. Using that index, it extracts function name, args and their
 * values.
 * 10. It directs the string evaluated to FILE 
 * 11. It restores the previous signal handler for SIGSEGV it stored 
 * in step 2
 * 12. It restores the previous Signal set (from user program) it
 * stored in step 1
 *
 * @param fp FILE Stream
 * @return void
 * 
 */


void traceback(FILE *fp)
{
	/* Defining local variables */
	unsigned long * frame_base_ptr = (unsigned long * )get_ebp();
	void * caller_site_address = NULL;
	int is_frame_valid,is_ret_addr_valid,entry_in_functions;
	char * args_values_list = NULL;
	int result  = FALSE;
	int output_fd = fileno(fp);

	if( output_fd == -1)
	{
		fprintf(fp,"Not a valid stream to write");
		return;
	}
   
	/* 
	 * Defining the signal handler for handling SIGSEGV signal
	 * There are cases when an address needs to be validated and 
	 * Segmentation fault (SIGSEGV) can help in providing that info
	 */
	struct sigaction new_act;

	memset(&new_act,0,sizeof(new_act));
	/* Initialising the signal set */

	sigset_t new_set;

	/* Adding all signals to mask for blocking during handler
	 * except SIGSEGV */
	result = Sigfillset(&new_act.sa_mask);
	if (result == ERROR)
	{
		return ;
	}

	result = Sigdelset(&new_act.sa_mask,SIGSEGV);
	if (result == ERROR)
	{
		return ;
	}
	new_act.sa_flags = SA_SIGINFO;

	/*Saving previous SIGSEGV handler action */

	result = Sigaction(SIGSEGV,NULL,&old_act);
	if (result == ERROR)
	{
		return ;
	}

	/* Unblock SIGSEGV signal and save the previous old set*/
	result = Sigemptyset(&new_set);
	if (result == ERROR)
	{
		return ;
	}

	result = Sigaddset(&new_set,SIGSEGV);
	if (result == ERROR)
	{
		return ;
	}

	result = Sigprocmask(SIG_UNBLOCK,&new_set,&old_set);
	if (result == ERROR)
	{
		return ;
	}

	/* Binding the handler to the sigaction */
	new_act.sa_sigaction = segfault_handler;

	/* Overiding the Segmentation fault signal default behavior*/
	result = Sigaction(SIGSEGV,&new_act,NULL);
	if (result == ERROR)
	{
		return ;
	}

	/* Check if the frame is valid */
	while (TRUE) 
	{
		is_frame_valid = check_if_frame_valid(frame_base_ptr);
		if (is_frame_valid == TRUE)
		{
			/* 
			 * Get the return address to call site
			 * for this valid frame 
			 */
			caller_site_address = GET_RET_ADDR(
					frame_base_ptr);
			
			ENSURES(caller_site_address != NULL);
			
			is_ret_addr_valid = check_if_addr_valid(
					caller_site_address, 
					ADDR_LOC_32);

			if (is_ret_addr_valid) 
			{
				/*
				 * Get the entry with the help of function 
				 * return address
				 */
				entry_in_functions = get_func_index_by_ret_addr(
						caller_site_address);
				
				args_values_list = get_args_and_values_list(
						frame_base_ptr,
						entry_in_functions);

				/* Print traceback entry by entry */
				if (args_values_list != NULL)
				{
					result =print_traceback_entry(output_fd,
							args_values_list);
					if (result == FALSE)
						return;
				} else 
				{
					fprintf(fp,
					  "Fatal error :Invalid frame\n");
					break;
				}	
			}
			/* Get next frame */
			frame_base_ptr = PREV_FRAME_PTR(frame_base_ptr);
		} else if (is_frame_valid == FALSE)
		{ 
				fprintf(fp,"Fatal error :Invalid frame\n");
				break;
		} else if (is_frame_valid == STOP)
		{
				break;
		}
	}
	/* Restoring the old action for SIGSEGV */
	result = Sigaction(SIGSEGV,&old_act,NULL);
	if (result == ERROR)
	{
		return ;
	}
	/* Restoring the old set */
	result = Sigprocmask(SIG_SETMASK,&old_set,NULL);
	if (result == ERROR)
	{
		return ;
	}
}
