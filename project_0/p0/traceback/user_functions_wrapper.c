#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
typedef void handler_t(int);

/* Error handling function */
void unix_error(char *msg) 
{
    fprintf(stderr, "msg: %s\n", msg);
    exit(0);
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
        unix_error("Malloc error");
    return p;
}

/* Wrapper for Sigemptyset */
void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0)
        unix_error("Sigemptyset error");
    return;
}

/* Wrapper for Sigaction */
void Sigaction(int signum, const struct sigaction *act,struct sigaction *oldact)
{
    if (sigaction(signum,act,oldact) < 0)
        unix_error("Sigaction error");
    return;
}
