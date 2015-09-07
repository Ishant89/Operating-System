/** @file traceback.c
 *  @brief The traceback function
 *
 *  This file contains the traceback function for the traceback library
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @bug Unimplemented
 */

//#define DEBUG 1
#include "traceback_internal.h"
#include "get_ebp_info.h"
#include "user_functions_wrapper.h"
#include<stdlib.h>
#include<sys/mman.h>
#include<signal.h>
#include<errno.h>
#include<string.h>
#include<setjmp.h>
#include<ctype.h>

#define TRUE 1
#define FALSE 0
#define ERROR -1
#define ADDR_LOC_32 4
#define MAX_TRACEBACK_ENTRY_SIZE 1024

/* Macro to get frame pointer of the caller function (%ebp) */ 
#define PREV_FRAME_PTR(curr) (unsigned long *)(*((unsigned long *)curr))
#define GET_RET_ADDR(ebp) (void *)(*(((unsigned long *)(ebp)) + 1))

/* Buffer for setjmp and longjmp */
sigjmp_buf buf;

/*
 * mem_addr_valid : check if a particular memory address 
 * is valid 
 */


int check_if_addr_valid(void * addr,int iter)
{
        PRINTF("Entering the fuction: %s\n",__func__);
		REQUIRES(iter >= 1 && addr != NULL);
		int i = 0 ;
		for (; i < iter ; i++)
		{
			if (!sigsetjmp(buf,1))
			{
				char value = *(char *)(addr + i);
				value = value;
			} else 
			{
				return FALSE;
			}
		}
        PRINTF("Exiting the fuction: %s\n",__func__);
        return TRUE;
}

/*
 * @function check_if_frame_valid(void * base_ptr)
 * @brief Function to check if stack frame is valid 
 *
 * Following condition is tested to ensure if the frame is valid :
 *
 * 1. Base pointer (%ebp) should be less than address dereferenced by it
 *    (*(%ebp))
 * 
 * Error : 
 *    If frame is not valid, it returns False 
 *    If frame is valid, it returns True
 *
 *    If (%ebp) is not a valid address, then it returns -1
 * 
 */


int check_if_frame_valid(void * base_ptr)
{
        /* TC: Base ptr should never be null */
        ENSURES(base_ptr != NULL);
        
        int is_addr_valid = 0 ;
        unsigned long * curr_frame_ptr = (unsigned long *) base_ptr,
                      * prev_frame_ptr = NULL;

        /* Check if base pointer is a valid address */
        is_addr_valid = check_if_addr_valid(base_ptr,ADDR_LOC_32);

        if (is_addr_valid)
        {
                prev_frame_ptr = PREV_FRAME_PTR(curr_frame_ptr);
                if (prev_frame_ptr > curr_frame_ptr)
                {
                        PRINTF("A valid frame \n");
                        return TRUE;
                } else 
                {
                        PRINTF("Not a valid frame \n");
                        return FALSE;
                }
        } else 
        {
               PRINTF("Base pointer address is not valid\n");
               return ERROR;
        } 
}

/* @function get_caller_site_addr(void * base_ptr)
 * @brief Get the call site address 
 *
 * The return address is evaluated from the stack by incrementing the 
 * base_ptr by 4 bytes
 */

void * get_caller_site_addr(void * base_ptr)
{
        /* TC: base_ptr should never be NULL*/
        ENSURES(base_ptr != NULL);

        void * ret_addr = GET_RET_ADDR(base_ptr);

        REQUIRES(ret_addr != NULL);
        return ret_addr;
}


int get_func_index_by_ret_addr(void * ret_addr)
{
        /*TC: ret_addr cannot be zero*/

        ENSURES(ret_addr != NULL);
        int i = 0,prev_addr_diff = 0,index=-1,
            curr_addr_diff;

        for (;strlen(functions[i].name) != 0 && i < FUNCTS_MAX_NAME; i++)
        {
                curr_addr_diff = ret_addr - functions[i].addr;
                if (curr_addr_diff > 0  && curr_addr_diff <= 
						MAX_FUNCTION_SIZE_BYTES) 
				{
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
        REQUIRES(i >= 0 );
        return index;
}

int handle_char(void * ptr, argsym_t arg,int counter)
{
	if (check_if_addr_valid(ptr,sizeof(char)))
	{

	}
	return FALSE;
}

int handle_int(void * ptr, argsym_t arg,int counter)
{
	return FALSE;
}

int handle_float(void * ptr, argsym_t arg,int counter)
{
	return FALSE;
}

int handle_double(void * ptr, argsym_t arg,int counter)
{
	return FALSE;
}

int handle_string(void * ptr, argsym_t arg,int counter)
{
	return FALSE;
}

int handle_string_array(void * ptr, argsym_t arg,int counter)
{
	return FALSE;
}

int handle_voidstar(void * ptr, argsym_t arg,int counter)
{
	return FALSE;
}

int handle_unknown(void * ptr, argsym_t arg,int counter)
{
	return FALSE;
}



char * get_args_and_values_list(void * base_ptr, int functions_index)
{
        int i=0,counter=0,success= FALSE;
        void * ptr;
        void * prev_frame_ptr = (void *) PREV_FRAME_PTR(base_ptr);
        const argsym_t * args_list = functions[functions_index].args;
	const char * name = functions[functions_index].name;
	char * traceback_entry = (char * ) Malloc(MAX_TRACEBACK_ENTRY_SIZE);

	counter = snprintf(traceback_entry,MAX_TRACEBACK_ENTRY_SIZE,
			"Function %s(",name);
	counter = counter;
        for(; strlen(args_list[i].name) != 0 && i < ARGS_MAX_NUM; i++)
        {
                ptr = prev_frame_ptr + args_list[i].offset;
                argsym_t arg = args_list[i];
                switch(arg.type)
                {
			case TYPE_CHAR:
				success = handle_char(ptr,arg,counter);
				break;
			case TYPE_INT:
				success = handle_int(ptr,arg,counter);
				break;
			case TYPE_FLOAT:
				success = handle_float(ptr,arg,counter);
				break;
			case TYPE_DOUBLE:
				success = handle_double(ptr,arg,counter);
				break;
			case TYPE_STRING:
				success = handle_string(ptr,arg,counter);
				break;
			case TYPE_STRING_ARRAY:
				success = handle_string_array(ptr,arg,counter);
				break;
			case TYPE_VOIDSTAR:
				success = handle_voidstar(ptr,arg,counter);
				break;
			case TYPE_UNKNOWN:
				success = handle_unknown(ptr,arg,counter);
				break;
                }
        }
	if (!success) 
	{
		return NULL;
	} else 
	{
		return traceback_entry;
	}
}

void print_traceback_entry(const char * func_name, char * args_details)
{
}

void * get_next_frame(void * frame_base_ptr)
{
        /*TC: frame_base_ptr cannot be null */
        ENSURES(frame_base_ptr != NULL);
        void * prev_frame_ptr = (void*) PREV_FRAME_PTR(frame_base_ptr);
        
        /*TC: previous frame cannot be null*/
        REQUIRES(prev_frame_ptr != NULL);
        return prev_frame_ptr;
}

/*
 * @handler segfault_handler
 * @brief handling segfault conditions 
 *
 */


void segfault_handler(int signal, siginfo_t * info, void * arg)
{
        PRINTF("In the segfault handler \n");
		longjmp(buf,1);
}

/*
 * @function traceback(File * fp)
 */


void traceback(FILE *fp)
{
        /* EDIT:To be removed */
        PRINTF("Entering the fuction: %s\n",__func__);

        /* Defining local variables */
        unsigned long * frame_base_ptr = (unsigned long * )get_ebp();
        void * caller_site_address = NULL;
        int is_frame_valid,is_ret_addr_valid,entry_in_functions;
        const char * func_name = NULL;
        char * args_values_list = NULL;
       
        /* 
         * Defining the signal handler for handling SIGSEGV signal
         * There are cases when an address needs to be validated and 
         * Segmentation fault (SIGSEGV) can help in providing that info
         */
        struct sigaction new_act;

        memset(&new_act,0,sizeof(new_act));
        /* Initialising the signal set */

        Sigemptyset(&new_act.sa_mask);
        new_act.sa_flags = SA_SIGINFO;

        /* Binding the handler to the sigaction */
        new_act.sa_sigaction = segfault_handler;

        /* Overiding the Segmentation fault signal default behavior*/
        Sigaction(SIGSEGV,&new_act,NULL);


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
                        caller_site_address = get_caller_site_addr(
                                        frame_base_ptr);
                        
                        ENSURES(caller_site_address != NULL);
                        
                        is_ret_addr_valid = check_if_addr_valid(
                                        caller_site_address, ADDR_LOC_32);

                        if (is_ret_addr_valid) 
                        {
                                /*
                                 * Get the entry with the help of function 
                                 * return address
                                 */
                                entry_in_functions = get_func_index_by_ret_addr(
                                                caller_site_address);
                                
                                func_name = functions[entry_in_functions].name;
                                args_values_list = get_args_and_values_list(
                                                frame_base_ptr,
                                                entry_in_functions);

                                /* Print traceback entry by entry */
                                print_traceback_entry(
                                                func_name,
                                                args_values_list
                                                );
                        }
                        
                        /* Get next frame */
						printf("Base pointer is %p\n",frame_base_ptr);
                        frame_base_ptr = get_next_frame(frame_base_ptr);
                } else if (is_frame_valid == FALSE)
                { 
                        printf("Fatal error :\n");
                        /* EDIT: Not sure about exit value */
                        //exit(-1);
                        break;
                } else if (is_frame_valid == ERROR)
                {
                        break;
                }
        }
        PRINTF("Exiting the fuction: %s\n",__func__);
}
