
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h> // includes fork(), execvp()
#include <string.h>
#include <ctype.h>
#include <sys/types.h> // include pid_t için kullanılması lazım ama neden aktif görünmüyor ?
#include <sys/wait.h> // includes wait()
#include <stdlib.h> // exit()
#include <sys/shm.h>

#define MAX_LINE        100   /* 100 chars per line, per command */
int hist_index =0;// keeps a track of the number of commands executed
char hist[10][100];

// function declaration

void updateHistory(char *);

void printHistory();

int printCurrentWorkingDirectory();

//driver function
int main() {




    while (1) {
        char command[MAX_LINE];
        printf("myshell> ");
        char buffer[100];
        fgets(buffer, 100, stdin);

        char *str = strtok(buffer, "\n");
        if (strlen(buffer)==1){
            continue;
        }
        strcpy(command, str);


        char *args[10] = {NULL};
        char *token;
        bool waitFlag = false;
        int i = 0, j = 0;
        char *pipedArgs[10]= {NULL};


        token = strtok(command, " ");
        int pipeFlag = 1;

        while (token != NULL) {
            if (strcmp(token, "&")==0){
                token = NULL ;
                waitFlag = true;
                break;
            }
            if (pipeFlag == 1 && strcmp(token, "|") == 0) {
                pipeFlag = 2;
                i = -1;
            } else {
                if (pipeFlag == 1) {
                    args[i] = token;
                } else {
                    pipedArgs[i] = token;
                }
            }

            token = strtok(NULL, " ");
            i++;
        }


        if (strcmp(command, "bye") == 0) {
            updateHistory(command);
            exit(0);
        } else if (strcmp(command, "history") == 0) {
            updateHistory(command);
            printHistory();
        } else if (strcmp(command, "dir") == 0) {
            updateHistory(command);
            printCurrentWorkingDirectory();
        } else if (strcmp(args[0], "cd") == 0) {
            updateHistory(command);
            if (args[1] == NULL) {
                char *home = getenv("HOME");
                int flag = chdir(home);
                if (flag < 0)
                    printf("directory couldn't be changed\n");
            } else {
                int result = 0;
                result = chdir(args[1]);
                if (result == 0) {
                    printf("directory changed\n");
                    setenv("PWD", args[1], 1);
                    getenv("PWD");
                } else {
                    perror("Couldn't change directory ");
                }
            }

        } else {
            if (pipeFlag == 1) {
                // Forking a child
                pid_t pid = fork();

                if (pid == -1) {
                    printf("Failed forking child.\n");
                    continue;
                } else if (pid == 0) {
                    //printf("child %s %s \n", args[0], args[1]);
                    if (execvp(args[0], args) != 0) {
                        printf("Could not execute command.\n");
                        continue;
                    }
                    exit(0);
                } else {
                    // waiting for child to terminate
                    if (waitFlag == false ) {wait(NULL);}
                }
            } else {

                // 0 is read end, 1 is written end
                int pipeFd[2];
                pid_t p1, p2;

                if (pipe(pipeFd) < 0) {
                    printf("\nPipe could not be initialized\n");
                    continue;
                }
                p1 = fork();
                if (p1 < 0) {
                    printf("\nCould not fork\n");
                    continue;
                }

                if (p1 == 0) {
                    // Child 1 executing
                    // It only needs to write at to write end
                    close(pipeFd[0]);
                    dup2(pipeFd[1], STDOUT_FILENO);
                    close(pipeFd[1]);

                    if (execvp(args[0], args) < 0) {
                        printf("Could not execute command before pipe.\n");
                    }
                    exit(0);
                } else {
                    // Parent executing
                    p2 = fork();

                    if (p2 < 0) {
                        printf("\nCould not fork\n");
                        continue;
                    }

                    // Child 2 executing
                    // It only needs to read at the read end
                    if (p2 == 0) {
                        close(pipeFd[1]);
                        dup2(pipeFd[0], STDIN_FILENO);
                        close(pipeFd[0]);
                        if (execvp(pipedArgs[0], pipedArgs) < 0) {
                            printf("Could not execute command after pipe\n");

                        }
                        exit(0);
                    } else {
                        // parent executing, waiting for two children
                        close(pipeFd[0]);
                        close(pipeFd[1]);
                        wait(NULL);
                        wait(NULL);
                    }
                }
            }

        }
    }

    exit(0);
}

// function implementation

void updateHistory(char *temp) {
    int remove_index = 1;
    if (hist_index<10){

        strcpy(hist[hist_index], temp);
        hist_index++;

    }
    else {

        for (remove_index = 1 ; remove_index<=10; remove_index++){
            strcpy(hist[remove_index-1],hist[remove_index]);
        }
        strcpy(hist[9], temp);
    }

}

void printHistory() {
    int x;
    for(x =0; x<hist_index;x++){
        printf("%s %d %s %s\n", "[" ,x+1 ,"]" , hist[x]);

    }

}

int printCurrentWorkingDirectory() {
    char *cwd;
    char buff[MAX_LINE];

    cwd = getcwd(buff, MAX_LINE);
    if (cwd != NULL) {
        printf( "%s\n", cwd);
    }

    return EXIT_SUCCESS;
}


