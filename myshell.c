#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#define COMHISLEN 100  //Command history lentgh.
#define MAXBUF 1024    
#define MAXARGS 15     //The max number of args myshell can take.

extern char **environ;

//Structure to store command history
typedef struct comListNode {
    struct comListNode *next;
    char *command;
} comListNode; 

comListNode *comHead;
int comSize = 0;
int comFlag = 0;

//Structure to store dir stack
typedef struct dirListNode {
    struct dirListNode *next;
    char *dir;
} dirListNode;

dirListNode *topDir = NULL;

//Parse input into arguments
void parseArgs(char *buffer, char **args, int argsSize, int *nargs) {
    char *bufArgs[argsSize];
    char **cp;
    char *tbuf;
    int i, j;

    tbuf = buffer;
    bufArgs[0] = buffer;
    args[0] = buffer;

    for(cp = bufArgs; (*cp = strsep(&tbuf, " \n\t")) != NULL;) {
        if((**cp != '\0') && (++cp >= &bufArgs[argsSize]))
            break;
    }

    for(j = i = 0; bufArgs[i] != NULL; i++) {
        if(strlen(bufArgs[i]) > 0)
            args[j++] = bufArgs[i];
    }

    *nargs = j;
    args[j] = NULL;
}


int main(int argc, char **argv) {
    char buf[MAXBUF];    
    char prePath[MAXBUF] = {0}; //Used for cd - command.
   
    while(1) {       
        printf("$ ");
        fgets(buf, MAXBUF, stdin);
        
        if(strlen(buf) > 1) {  //Input must have string length more than 1
            buf[strlen(buf)-1] = '\0';

            char *args[MAXARGS];
            int nargs;
            char nbuf[MAXBUF];
            strncpy(nbuf, buf, MAXBUF);

            parseArgs(nbuf, args, MAXARGS, &nargs);
            if(nargs == 0) {
                continue;
            }

            //!n command
            if(args[0][0] == '!') {
                if(nargs != 1) {
                    printf("Usage: !n.\n");
                    continue;
                }

                int num = atoi(args[0] + 1);
                if(num <= 0 || num > comSize || num == COMHISLEN+1) {
                    printf("%s is not found.\n", args[0]);
                    continue;
                }

                int i = 1;
                comListNode *hisCom = comHead;
                while((hisCom->next != NULL) && (i < num)) {
                    hisCom = hisCom->next;
                    i++;
                }
                strncpy(buf, hisCom->command, strlen(hisCom->command)+1);
                printf("%s\n", buf);
            }

            //Add command into history
            comListNode *tmpNode1, *tmpNode2;
            tmpNode1 = (comListNode*)malloc(sizeof(comListNode));
            tmpNode1->next = NULL;
            tmpNode1->command = (char*)malloc(strlen(buf)+1);
            strncpy(tmpNode1->command, buf, strlen(buf)+1);

            if(comSize == 0) {
                comHead = tmpNode1;
            }
            else {
                tmpNode2 = comHead;
                while(tmpNode2->next) {
                    tmpNode2 = tmpNode2->next;
                }
                tmpNode2->next = tmpNode1;
            }

            if(comFlag == 0)
                comSize++;
            
            if(comSize > COMHISLEN) {  //Age out old history commands
                comFlag = 1;
                tmpNode2 = comHead;
                comHead = comHead->next;
                free(tmpNode2->command);
                free(tmpNode2);
            }

            parseArgs(buf, args, MAXARGS, &nargs);
            if(nargs == 0) 
                continue;
            
            //history command
            if(strncmp(args[0], "history", 7) == 0) {
                if(nargs != 1) {
                    printf("Usage: history.\n");
                    continue;
                }

                comListNode *tmp = comHead;
                int i = 1;

                while(tmp) {
                    printf("%d %s\n", i, tmp->command);
                    tmp = tmp->next;
                    i++;
                }

                continue;
            }

            //pwd command
            if(strncmp(args[0], "pwd", 3) == 0) {
                if(nargs != 1) {
                    printf("Usage: pwd.\n");
                    continue;
                }

                char pwdBuf[MAXBUF];

                if(getcwd(pwdBuf, MAXBUF) == NULL) {
                    perror("pwd failed.");
                    printf("strerror: %s.\n", strerror(errno));
                }
                else
                    printf("%s\n", pwdBuf);

                continue;
            }

            //cd command
            if(strncmp(args[0], "cd", 2) == 0) {
                if(nargs > 2) {
                    printf("Usage: cd [dir].\n");
                    continue;
                }

                char *path;
                if(nargs == 1) //cd without args
                    path = getenv("HOME");
                else
                    path = args[1];
                char tmpPath[MAXBUF];
                if(getcwd(tmpPath, MAXBUF) == NULL) {
                    perror("cd failed.");
                    printf("strerror: %s.\n", strerror(errno));
                }               
 
                if(*path == '-') {  //cd - command      
                    if(chdir(prePath) == 0) {
                        printf("%s\n", prePath);
                        strcpy(prePath, tmpPath);
                    }
                    else {
                        perror("cd failed.");
                        printf("strerror: %s.\n", strerror(errno));
                    }

                    continue;
                }

                if(chdir(path) == 0)
                    strcpy(prePath, tmpPath);            
                else {
                    perror("cd failed.");
                    printf("strerror: %s.\n", strerror(errno));
                }

                continue;
            }
            
            //pushd command.
            if(strncmp(buf, "pushd", 5) == 0) {
                if(nargs != 1) {
                    printf("Usage: pushd.\n");
                    continue;
                }

                char curDir[MAXBUF];
                if(getcwd(curDir, MAXBUF) == NULL) {
                    perror("getcwd failed.");
                    printf("strerror: %s.\n", strerror(errno));
                    continue;
                }

                char *tmpDir = (char*)malloc(strlen(curDir) + 1);
                strncpy(tmpDir, curDir, strlen(curDir)+1);
                dirListNode *tmpNode = (dirListNode*)malloc(sizeof(dirListNode));
                tmpNode->dir = tmpDir;

                if(topDir != NULL) {
                    tmpNode->next = topDir;
                    topDir = tmpNode;
                }
                else {
                    tmpNode->next = NULL;
                    topDir = tmpNode;
                }

                tmpNode = topDir;
                while(tmpNode != NULL) {
                    printf("%s ", tmpNode->dir);
                    tmpNode = tmpNode->next;
                }
                printf("\n");

                continue;
            } 

            //popd command
            if(strncmp(args[0], "popd", 4) == 0) {
                if(nargs != 1) {
                    printf("Usage: popd.\n");
                    continue;
                }

                if(topDir != NULL) {
                    if(chdir(topDir->dir) != 0) {
                        perror("chdir failed.");
                        printf("strerror: %s.\n", strerror(errno));
                        continue;
                    }

                    dirListNode *tmp = topDir;
                    topDir = topDir->next;
                    free(tmp->dir);
                    free(tmp);
                }

                if(topDir == NULL)
                    printf("Dir stack is empty.\n");
                else {
                    dirListNode *tmp = topDir;
                    while(tmp != NULL) {
                        printf("%s ", tmp->dir);
                        tmp = tmp->next;
                    }
                    printf("\n");
                }

                continue;
            }

            //dirs command
            if(strncmp(args[0], "dirs", 4) == 0) {
                if(nargs != 1) {
                    printf("Usage: dirs.\n");
                    continue;
                }

                if(topDir != NULL) {
                    dirListNode *tmp = topDir;
                    while(tmp != NULL) {
                        printf("%s ", tmp->dir);
                        tmp = tmp->next;
                    }
                    printf("\n");
                }
                else
                    printf("Dir stack is empty.\n");

                continue;
            }

            //exit command
            if(strncmp(args[0], "exit", 4) == 0) {
                comListNode *tmpHis1 = comHead;
                comListNode *tmpHis2;
                while(tmpHis1) {
                    tmpHis2 = tmpHis1;
                    tmpHis1 = tmpHis1->next;
                    free(tmpHis2->command);
                    free(tmpHis2);
                }

                dirListNode *tmpDir1 = topDir;
                dirListNode *tmpDir2;
                while(tmpDir1) {
                    tmpDir2 = tmpDir1;
                    tmpDir1 = tmpDir1->next;
                    free(tmpDir2->dir);
                    free(tmpDir2);
                }

                return 0;
            }

            //Execute command with absolute path.
            if(args[0][0] == '/') {
                int ret;
                pid_t pid;
                
                pid = fork();
                if(pid) {
                    pid = wait(&ret);
                }
                else {
                    if(execv(args[0], args) == -1) {
                        perror("Command executed failed.");
                        printf("strerror: %s.\n", strerror(errno));
                        exit(1);
                    }
                }
            }

            //Execute command with relative path.
            else {
                int i;
                int pathFlag = 0;

                for(i = 0; i < strlen(args[0]); i++)
                    if(args[0][i] == '/') {
                        pathFlag = 1;
                        break;
                    }
                
                //Command contains no '/', search command under system folders
                if(pathFlag == 0) {
                    int ret;
                    pid_t pid;

                    pid = fork();
                    if(pid) {
                        pid = wait(&ret);
                    }
                    else {
                        int i = 0;
                        while((environ[i] != NULL) && (strncmp(environ[i], "PATH", 4) != 0))
                            i++;
                        environ[i] = "PATH=/bin:/usr/bin:/sbin:/usr/sbin";

                        if(execvp(args[0], args) == -1) {
                            perror("Command executed failed.");
                            printf("strerror: %s.\n", strerror(errno));
                            exit(1);
                        }
                    }
                }
             
                //Command contains '/', search command under current dir.
                else {
                    if(args[0][0] != '.') {  //Command is under current directory
                        char currentPath[MAXBUF];
                        if(getcwd(currentPath, MAXBUF) == NULL) {
                            perror("getcwd failed.");
                            printf("strerror: %s.\n", strerror(errno));
                            continue;
                        }

                        strncat(currentPath, "/", 1);
                        strncat(currentPath, args[0], strlen(args[0]));
                    
                        int ret;
                        pid_t pid;

                        pid = fork();
                        if(pid) {
                            pid = wait(&ret);
                        }    
                        else {
                            if(execv(currentPath, args) == -1) {
                                perror("Command executed failed.");
                                printf("strerror: %s.\n", strerror(errno));
                                exit(1);
                            }
                        }
                    }
                    else {  //Command is under relative directory
                        char currentPath[MAXBUF];
                        if(getcwd(currentPath, MAXBUF) == NULL) {
                            perror("getcwd failed.");
                            printf("strerror: %s.\n", strerror(errno));
                            continue;
                        }

                        strncat(currentPath, args[0] + 1, strlen(args[0]+1));
                        
                        int ret;
                        pid_t pid;

                        pid = fork();
                        if(pid) {
                            pid = wait(&ret);
                        }    
                        else {
                            if(execv(currentPath, args) == -1) {
                                perror("Command executed failed.");
                                printf("strerror: %s.\n", strerror(errno));
                                exit(1);
                            }
                        }
                    }
                }
            }
        }    
    }
}
