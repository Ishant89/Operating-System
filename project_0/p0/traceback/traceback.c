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
#include<unistd.h>

#define TRUE 1
#define FALSE 0
#define ERROR -1
#define ADDR_LOC_32 4
#define MAX_TRACEBACK_ENTRY_SIZE 1024
#define ARG_STRING_SIZE 256
#define MAX_STRING_SIZE 29
#define MAX_STRING_WO_DOTS 25

/* Macro to get frame pointer of the caller function (%ebp) */ 
#define PREV_FRAME_PTR(curr) (unsigned long *)(*((unsigned long *)curr))
#define GET_RET_ADDR(ebp) (void *)(*(((unsigned long *)(ebp)) + 1))

/* Buffer for setjmp and longjmp */
sigjmp_buf buf;

/* Sigset to save the previous state of signal set */
sigset_t old_set;

/* Sigaction to save the old action of SIGSEGV handler */

struct sigaction old_act;

/*
 * mem_addr_valid : check if a particular memory address 
 * is valid 
 */


int check_if_addr_valid(void * addr,int iter)
{
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
			sigset_t temp_set = old_set;
			Sigdelset(&temp_set,SIGSEGV);
			Sigprocmask(SIG_SETMASK,&temp_set,NULL);
			return FALSE;
		}
	}
        return TRUE;
}

/*
 */

int is_protected_mem_writable(void * base_ptr)
{
	void ** prev_frame_ptr = (void *)PREV_FRAME_PTR(base_ptr);
	char value_restore ;
	if (!sigsetjmp(buf,1))
	{
		value_restore = *(char*)prev_frame_ptr;
		memset(*prev_frame_ptr,0,sizeof(char));
	} else 
	{
		sigset_t temp_set = old_set;
		Sigdelset(&temp_set,SIGSEGV);
		Sigprocmask(SIG_SETMASK,&temp_set,NULL);
		return TRUE;
	}
	memset(*prev_frame_ptr,value_restore,sizeof(char));
	return FALSE;
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
			return TRUE;
		} else 
		{
			if (is_protected_mem_writable(base_ptr))
					return ERROR;
			return FALSE;
		}
        } else 
        {
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
                curr_addr_diff = (unsigned int)ret_addr - 
				    (unsigned int)functions[i].addr;
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
				if (isprint(ch))
				{
					if (i < MAX_STRING_WO_DOTS)
						output_str[i]=ch;
				} else 
				{
					error = TRUE;
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
		if ( i > MAX_STRING_WO_DOTS -1 )
			for (i = MAX_STRING_WO_DOTS; i < MAX_STRING_SIZE-1;i++)
			{
				output_str[i]='.';
			}
		output_str[i] = '\0';
		snprintf(temp_str,ARG_STRING_SIZE,"\"%s\"",output_str);
		return temp_str;
	}
}


int handle_string(char * entry,void ** ptr,const char * name)
{
	void * value;
	char * output_str;
	if (check_if_addr_valid(ptr,sizeof(char *)))
	{
		value =  *ptr;
		output_str = verify_string_conditions((char *)value);
		snprintf(entry,ARG_STRING_SIZE,
				"char *%s=%s,",name,output_str);
		free(output_str);
		return TRUE;
	}
	return FALSE;
}

int handle_string_array(char * entry,void *** ptr,const char * name)
{
	void ** value;
	int i = 0,error = 0 ;
	char * string_entry;
	char * iterator;
	snprintf(entry,ARG_STRING_SIZE,"char **%s={",name);
	if (check_if_addr_valid(ptr,sizeof(char **)))
	{
		value = * ptr;
		while (TRUE)
		{
			int result = check_if_addr_valid(value + i,
					sizeof(char*)) && value[i] != NULL;
			if (result) 
			{
				string_entry = verify_string_conditions(
						(char*)value[i]); 
				iterator = entry + strlen(entry);
				if ( i < 3 ) 
				{
					if (string_entry != NULL)
					{
					    snprintf(iterator,
						 ARG_STRING_SIZE,
						 "%s,",
						 string_entry);
					} 

				} else 
				{
					snprintf(iterator,ARG_STRING_SIZE,
							"...");
					break;
				}
				i++;
			} else 
			{

				snprintf(entry,ARG_STRING_SIZE,
						"char **%s=%p",name,value);
				return TRUE;
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

int handle_voidstar(char * entry,unsigned int * ptr,const char * name)
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

int handle_unknown(char * entry,void ** ptr,const char * name)
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



char * get_args_and_values_list(void * base_ptr, 
				int functions_index)
{
        int i=0,success= FALSE;
        void * ptr;
        void * prev_frame_ptr = (void *) PREV_FRAME_PTR(base_ptr);
        const argsym_t * args_list = functions[functions_index].args;
	const char * name = functions[functions_index].name;
	char * traceback_entry = (char * ) Malloc(MAX_TRACEBACK_ENTRY_SIZE);
	char * entry_temp;
        void * ret_addr = GET_RET_ADDR(base_ptr);

	if (functions_index == -1)
	{
		snprintf(traceback_entry,MAX_TRACEBACK_ENTRY_SIZE,
				"Function %p(...), in\n",ret_addr);
		return traceback_entry;
	} 
    	snprintf(traceback_entry,MAX_TRACEBACK_ENTRY_SIZE,
				"Function %s(",name);

	PRINTF("Arg len name is %s\n",args_list[i].name);
        for(; strlen(args_list[i].name) != 0 && i < ARGS_MAX_NUM; i++)
        {
                ptr = prev_frame_ptr + args_list[i].offset;
                argsym_t arg = args_list[i];
		entry_temp = traceback_entry + strlen(traceback_entry);
		PRINTF("Arg type is %d\n",arg.type);
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

	
	if ( strlen(args_list[0].name) == 0)
	{
		PRINTF("Handling it \n");
		success = TRUE;
		entry_temp = traceback_entry + strlen(traceback_entry);
		snprintf(entry_temp,ARG_STRING_SIZE,"void,");
	}
	if (!success) 
	{
		return NULL;
	} else 
	{
		entry_temp = traceback_entry + strlen(traceback_entry)-1;
		snprintf(entry_temp,ARG_STRING_SIZE,"), in\n");
		return traceback_entry;
	}
}

void print_traceback_entry(FILE * fp, char * args_details)
{
	/* EDIT: change 1 to fd */
	int output_fd = fileno(fp);
	write(output_fd,args_details,strlen(args_details));
	free(args_details);
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
	longjmp(buf,1);
}

/*
 * @function traceback(File * fp)
 */


void traceback(FILE *fp)
{
        /* EDIT:To be removed */

        /* Defining local variables */
        unsigned long * frame_base_ptr = (unsigned long * )get_ebp();
        void * caller_site_address = NULL;
        int is_frame_valid,is_ret_addr_valid,entry_in_functions;
        char * args_values_list = NULL;
       
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
        Sigfillset(&new_act.sa_mask);
	Sigdelset(&new_act.sa_mask,SIGSEGV);
        new_act.sa_flags = SA_SIGINFO;

	/*Saving previous SIGSEGV handler action */

	Sigaction(SIGSEGV,NULL,&old_act);

	/* Unblock SIGSEGV signal and save the previous old set*/
	sigemptyset(&new_set);
	sigaddset(&new_set,SIGSEGV);
	Sigprocmask(SIG_UNBLOCK,&new_set,&old_set);

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
                                
                                args_values_list = get_args_and_values_list(
                                                frame_base_ptr,
                                                entry_in_functions);

                                /* Print traceback entry by entry */
				if (args_values_list != NULL)
				{
				    print_traceback_entry(
                                                fp,
                                                args_values_list
                                                );
    				} else 
				{
				    printf("Fatal error2 :\n");
				    break;
				}
                        }
                        
                        /* Get next frame */
                        frame_base_ptr = get_next_frame(frame_base_ptr);
                } else if (is_frame_valid == FALSE)
                { 
                        printf("Fatal error1 :\n");
                        /* EDIT: Not sure about exit value */
                        //exit(-1);
                        break;
                } else if (is_frame_valid == ERROR)
                {
                        break;
                }
        }

	/* Restoring the old action for SIGSEGV */
	Sigaction(SIGSEGV,&old_act,NULL);
}

