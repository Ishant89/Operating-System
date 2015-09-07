/** @file user_functions_wrapper.h
 *  @brief wrapper functions definations for error handling
 *  
 *  This includes functions such as malloc,signal etc
 *
 *  @author Ishant Dawer (idawer)
 */

#ifndef __WRAPPER_FUNC_H
#define __WRAPPER_FUNC_H
#include "contracts.h"
#include<signal.h>
void unix_error(char* msg);
void *Malloc(size_t size);

typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

void Sigemptyset(sigset_t *set);

int Sigaction(int signum, const struct sigaction *act,struct sigaction *oldact);

#endif

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) ((void) 0)
#endif
