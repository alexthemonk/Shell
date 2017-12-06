#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#include "SignalHandlers.h"

void chldHandler(int sig){
    int status;
    /* pid_t pid;*/
    
    /*pid = */waitpid(-1, &status, WNOHANG);
}
