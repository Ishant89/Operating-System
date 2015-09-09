#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#define TRUE 1
#define FALSE 0


typedef void handler_t(int);

/* Error handling function */
void unix_error(char *msg) 
{
    fprintf(stderr, "msg: %s\n", msg);
}

/* Wrapper for signal function */
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

/*Wrapper for Malloc function */
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

/* Wrapper for Sigemptyset */
int Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0) 
	{
        unix_error("Sigemptyset error");
		return 0;
	}
	return 1;
}

/* Wrapper for Sigaction */
int Sigaction(int signum, const struct sigaction *act,struct sigaction *oldact)
{
    if (sigaction(signum,act,oldact) < 0)
    {
        unix_error("Sigemptyset error");
		return 0;
	}
	return 1;
}

/* Wrapper for Sigprocmask */
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	    if (sigprocmask(how, set, oldset) < 0)
		{
			unix_error("Sigemptyset error");
			return 0;
		}
		return 1;
}



/* Wrapper for Sigfillset */
int Sigfillset(sigset_t *set)
{ 
	    if (sigfillset(set) < 0)
		{
			unix_error("Sigemptyset error");
			return 0;
		}
		return 1;
}


/* Wrapper for Sigaddset */
int Sigaddset(sigset_t *set, int signum)
{
	    if (sigaddset(set, signum) < 0)
		{
			unix_error("Sigemptyset error");
			return 0;
		}
		return 1;
}


/* Wrapper for Sigdelset */
int Sigdelset(sigset_t *set, int signum)
{
	    if (sigdelset(set, signum) < 0)
		{
			unix_error("Sigemptyset error");
			return 0;
		}
		return 1;
}


/* Wrapper for Sigismember */
int Sigismember(const sigset_t *set, int signum)
{
	    int rc;
		if ((rc = sigismember(set, signum)) < 0)
			unix_error("Sigismember error");
		return rc;
}

