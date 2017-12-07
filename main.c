#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#include "SignalHandlers.h"

#define MAX_LINE 256
#define MAX_ARGS 25

int piping(char *line);
int historyCommand(int background);
char* historyExclaimation(char *line);
int checkin(char* commandPtr);
int checkout(char* commandPtr);
char* getFileIn(int option, char* commandPtr);
char* getFileOut(int option, char* commandPtr);
void debug();

int main(int argc , char * argv []) {
    char *temp;
    char *clean;
    int stdin_c, stdout_c;
    struct sigaction action;
    int background;
    char line[MAX_LINE];
    int status;
    /* pid_t pid;*/
    FILE* history;

    /* define sigchld*/
    memset (&action , '\0', sizeof(struct sigaction));
    action.sa_handler = chldHandler;
    action.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &action, NULL);

    /* creating a history file or just write to the existed history */
    history = fopen("command_history", "a+");
    do{
        /* reset background, 0 for foreground, 1 for background*/
        background = 0;
        
        /* first ensure everything has been set to default*/
        printf("\nCommand me Sir -> ");
        memset(line, '\0', MAX_LINE);

        /* reading the command input*/
        fgets(line , MAX_LINE - 1, stdin);

        /* check if no command has been inputed */
        if(strcmp(line, "\n") == 0){
            continue;
        }
        
        /* taking off the \n char at the end */
        line[strlen(line)-1] = '\0';
        /* clean up the extra spaces at the end */
        while(line[strlen(line)-1] == ' '){
            line[strlen(line)-1] = '\0';
        }
        /* clean up extra spaces at the beginning */
        temp = malloc((strlen(line) + 1) * sizeof(char));
        clean = temp;
        strcpy(temp, line);
        while(*temp == ' '){
            temp++;
        }
        strcpy(line, temp);
        free(clean);
        
        if(line[0] == '!'){
            /* using ! to execute the command from history */
            temp = historyExclaimation(line);
            memset(line, '\0', MAX_LINE);
            strcpy(line, temp);
            free(temp);
            printf("%s\n", line);
        }
        
        /* dealing with other arbitrary commands */
        if(strcmp(line, "history") == 0){
            /* storing the command into the history file */
            fputs(line, history);
            fputs("\n", history);
            fflush(history);
            /* deal with history command */
            historyCommand(background);
            continue;
        }
        
        /* storing the command into the history file */
        fputs(line, history);
        fputs("\n", history);
        fflush(history);
        
        /* default operations */
        if(strcmp(line, "quit") == 0){
            break;
        }

        /* dealing with background commands */
        if(line[strlen(line)-1] == '&'){
            background = 1;
            line[strlen(line)-1] = '\0';
        }
        /* clean up the extra spaces at the end */
        while(line[strlen(line)-1] == ' '){
            line[strlen(line)-1] = '\0';
        }
            
        /* back up stdin and stdout for the parent */
        stdin_c = dup(0);
        stdout_c = dup(1);
        piping(line);
        /* restore the stding and stdout for the parent process */
        dup2(stdin_c, 0);
        dup2(stdout_c, 1);
        
        if(background == 0){
        /* dealing with foreground command*/
            /* pid = */wait(&status);
        }
        /* else dealing with background command*/
        
    }while(1);
    
    fclose(history);

    return 0;
}


int piping(char *line){
    /* taking the line of argument and tokenize with character '|' */
    /* creating processes using commands from the token */
    /* change the stdin and stdout of each process recursively */
    
    /* initialize the pipe integer array, 0 for stdin 1 for stdout*/
    int pipeline[2];
    char *commandPtr;
    char *stringPtr;
    char *sPtr;
    char *commandArgs[MAX_ARGS];
    int i, j;
    char *fnIn;
    char *fnOut;
    char *temp;
    
    int writeToFile, readFromFile;
    
    pipe(pipeline);
    
    /* clean up the array*/
    for(i = 0; i < MAX_ARGS; i++){
        commandArgs[i] = NULL;
    }
    
    /* tokenize with '|' to get the first command*/
    commandPtr = strtok(line, "|");
    /* store the rest of the string for recursion*/
    stringPtr = strtok(NULL, "");
    
    /* check for alternative file output */
    writeToFile = checkout(commandPtr);
    readFromFile = checkin(commandPtr);
    
    commandPtr = strtok(commandPtr, "<>");
    
    temp = strtok(NULL, "");
    if(readFromFile != 0){
        fnIn = getFileIn(readFromFile, temp);
    }
    if(writeToFile != 0){
        fnOut = getFileOut(writeToFile, temp);
    }
    
    /* printf("%s %s\n", commandPtr, stringPtr);
    printf("%d %s, %d %s\n", readFromFile, fnIn, writeToFile, fnOut); */
    
    /* tokenize the first argument*/
    sPtr = strtok(commandPtr, " ");
    commandArgs[0] = malloc((strlen(sPtr) + 1) * sizeof(char));
    strcpy(commandArgs[0], sPtr);
    i = 1;
    while(sPtr != NULL){
        /* printf("%s \n", sPtr);*/
        sPtr = strtok(NULL, " ");
        if(sPtr != NULL){
            commandArgs[i] = malloc((strlen(sPtr) + 1) * sizeof(char));
            strcpy(commandArgs[i], sPtr);
        }
        i ++;
    }
    
    /* base case*/
    if(stringPtr == NULL){
        if(writeToFile == 1){
            freopen(fnOut, "w+", stdout);
            // dup2(output, 1);
        }
        else if(writeToFile == 2){
            freopen(fnOut, "a+", stdout);
            // dup2(output, 1);
        }
        if(readFromFile == 1){
            freopen(fnIn, "r", stdin);
            // dup2(output, 1);
        }
        else if(readFromFile == 2){
            freopen(fnIn, "r", stdin);
            // dup2(output, 1);
        }
        if(fork() == 0){
            /* child performing the execution of command*/
            execvp(commandArgs[0], commandArgs);
            perror("Invalid Command");
            /* memory clean up*/
            for(j = 0; j < MAX_ARGS; j ++){
                free(commandArgs[j]);
            }
            exit(2);
        }
        /* memory clean up*/
        for(j = 0; j < MAX_ARGS; j ++){
            free(commandArgs[j]);
        }
        return 0;
    }
    /* recursive case*/
    else{
        if(fork() == 0){
            /* start connecting the pipe for child and parent*/
            /* redirect stdout of child process to the pipe*/
            dup2(pipeline[1], 1);
            
            /* child performing the execution of command*/
            execvp(commandArgs[0], commandArgs);
            perror("Invalid Command");
            /* memory clean up*/
            for(j = 0; j < MAX_ARGS; j ++){
                free(commandArgs[j]);
            }
            exit(2);
        }
        
        /* redirect stdin of parent process to the pipe*/
        dup2(pipeline[0], 0);
        close(pipeline[1]);
        close(pipeline[0]);
    
        /* memory clean up*/
        for(j = 0; j < MAX_ARGS; j ++){
            free(commandArgs[j]);
        }
        piping(stringPtr);
        wait(NULL);
    
        return 0;
    }
}


int historyCommand(int background){
    char * command[4];
    int status;
    
    command[0] = "cat";
    command[1] = "command_history";
    command[2] = "-n";
    command[3] = NULL;
    
    if(fork() == 0){
        execvp(command[0], command);
        perror("Invalid Command");
        exit(2);
    }
    if(background == 0){
        /* dealing with foreground command*/
        /* pid = */wait(&status);
    }
    return 0;
}


char* historyExclaimation(char *line){
    int i, l, k;
    char* ret;
    FILE* history;
    size_t len;
    
    len = 0;
    
    ret = malloc((strlen(line) + 1) * sizeof(char));
    strcpy(ret, line);
    /* get the second element of the line, which is the number */
    for(i = 0; i < strlen(ret)-1; i++){
        ret[i] = ret[i+1];
    }
    ret[strlen(ret)-1] = '\0';
    /* taking off the spaces before the number */
    while(*ret == ' '){
        ret++;
    }
    
    l = atoi(ret);
    ret[0] = '\0';
    history = fopen("command_history", "r");
    for(i = 0; i < l; i++){
        getline(&ret, &len, history);
        k = strlen(ret);
    }
    fclose(history);
    /* taking off the \n char at the end */
    ret[k-1] = '\0';
    return ret;
}


int checkin(char* commandPtr){
    int readFromFile = 0; /* 0 for nothing, 1 for < with w+, 2 for << with a+ */
    if(strstr(commandPtr, "<")){
        readFromFile = 1; 
        if(strstr(commandPtr, "<<")){
            readFromFile = 2;
        }
    }
    return readFromFile;
}


int checkout(char* commandPtr){
    int writeToFile = 0; /* 0 for nothing, 1 for > with w+, 2 for >> with a+ */
    if(strstr(commandPtr, ">")){
        writeToFile = 1; 
        if(strstr(commandPtr, ">>")){
            writeToFile = 2;
        }
    }
    return writeToFile;
}


char* getFileIn(int option, char* commandPtr){
    char *fn;
    
    fn = strtok(commandPtr, "<");
    if(option == 2){
        fn = strtok(fn, "<");
    }
    while((*fn) == ' '){
        fn++;
    }
    return fn;
}


char* getFileOut(int option, char* commandPtr){
    char *fn;
    
    fn = strtok(commandPtr, ">");
    if(option == 2){
        fn = strtok(fn, ">");
    }
    while((*fn) == ' '){
        fn++;
    }
    return fn;
}


void debug(){
    printf("Random stuffs\n");
}