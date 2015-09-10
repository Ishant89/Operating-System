/** @file user_functions_wrapper.c
 *  @brief Wrappers for important Syscalls
 *
 *  This contains wrappers definitions for 
 *  syscalls such as Malloc, sigfillset etc
 *
 *  @author Ishant Dawer (idawer)
 *  @bug No bugs known
 */

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#define TRUE 1
#define FALSE 0


typedef void handler_t(int);

/** @brief  Error handling function
 *
 * This function prints the error assciated with
 * system call 
 *
 * @param msg Error message
 * @return void
 */

void unix_error(char *msg) 
{
    fprintf(stderr, "msg: %s\n", msg);
}

/** @brief Signal Wrapper 
 *
 * Wrapper for signal syscall
 *
 * @param signum Signal number
 * @param handler Handler pointer
 * @return handler_t
 *
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        unix_error("Signal error");
    return (old_action.sa_handler);
}

/** @brief Malloc Wrapper 
 *
 * Wrapper for malloc syscall
 *
 * @param size Size to be allocated 
 * @return handler_t
 *
 */
void *Malloc(size_t size) 
{
    void *p;

    if ((p  = malloc(size)) == NULL)
	{
        unix_error("Malloc error");
		return NULL;
	}
    return p;
}

/** @brief sigemptyset Wrapper 
 *
 * Wrapper for sigemptyset syscall
 *
 * @param set Signal set 
 * @return handler_t
 *
 */
int Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0) 
	{
        unix_error("Sigemptyset error");
		return 0;
	}
	return 1;
}

/** @brief sigaction Wrapper 
 *
 * Wrapper for sigaction syscall
 *
 * @param signum Signal number
 * @param act New action
 * @param oldact Old action 
 * @return True or False
 *
 */
int Sigaction(int signum, const struct sigaction *act,struct sigaction *oldact)
{
    if (sigaction(signum,act,oldact) < 0)
    {
        unix_error("Sigemptyset error");
		return 0;
	}
	return 1;
}

/** @brief Sigprocmask Wrapper 
 *
 * Wrapper for sigprocmask syscall
 *
 * @param how Signal action 
 * @param set new signal set
 * @param oldset Old Signal set
 * @return True or False
 *
 */
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	    if (sigprocmask(how, set, oldset) < 0)
		{
			unix_error("Sigemptyset error");
			return 0;
		}
		return 1;
}


/** @brief Sigfillset Wrapper 
 *
 * Wrapper for sigfillset syscall
 *
 * @param set Signal set
 * @return True or False
 *
 */

int Sigfillset(sigset_t *set)
{ 
	    if (sigfillset(set) < 0)
		{
			unix_error("Sigemptyset error");
			return 0;
		}
		return 1;
}

/** @brief Sigaddset Wrapper 
 *
 * Wrapper for sigaddset syscall
 *
 * @param set Signal set
 * @param signum Signal Number
 * @return True or False
 *
 */
int Sigaddset(sigset_t *set, int signum)
{
	    if (sigaddset(set, signum) < 0)
		{
			unix_error("Sigemptyset error");
			return 0;
		}
		return 1;
}

/** @brief Sigdelset Wrapper 
 *
 * Wrapper for sigdelset syscall
 *
 * @param set Signal set
 * @param signum Signal Number
 * @return True or False
 *
 */

int Sigdelset(sigset_t *set, int signum)
{
	    if (sigdelset(set, signum) < 0)
		{
			unix_error("Sigemptyset error");
			return 0;
		}
		return 1;
}

/** @brief Sigdelset Wrapper 
 *
 * Wrapper for sigdelset syscall
 *
 * @param set Signal set
 * @param signum Signal Number
 * @return True or False
 *
 */
int Sigismember(const sigset_t *set, int signum)
{
	    int rc;
		if ((rc = sigismember(set, signum)) < 0)
			unix_error("Sigismember error");
		return rc;
}

