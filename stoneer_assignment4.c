/*
    Title:  CS 374 Assignment 4 SmallSh
    Author: Erik Stone (stoneer@oregonstate.edu)
    Date: 2/21/2026

    Major References: sample_parser.c provided in canvas: https://canvas.oregonstate.edu/courses/2031516/assignments/10318861
					  canvas process & monitoring example: https://canvas.oregonstate.edu/courses/2031516/pages/exploration-process-api-monitoring-child-processes?module_item_id=26231578
					  canvas executing a new program example: https://canvas.oregonstate.edu/courses/2031516/pages/exploration-process-api-executing-a-new-program?module_item_id=26231579


        
*/
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS		 512


struct commandLine
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool isBg;
};


struct commandLine *parse_input()
{
	char input[INPUT_LENGTH];
	struct commandLine *currCommand = (struct commandLine *) calloc(1, sizeof(struct commandLine));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	// Tokenize the input
	char *token = strtok(input, " \n");
	while(token){
		if(!strcmp(token,"<")){
			currCommand->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			currCommand->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			currCommand->isBg = true;
		} else{
			currCommand->argv[currCommand->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	return currCommand;
}

//function prototypes
void handleSIGINT(int);




int main()
{
	/*
	//Get limit of processes 
	//REFs: Google AI summary, prompt: "getrlimit rlim_cur example c linux"
	//		https://man7.org/linux/man-pages/man2/getrlimit.2.html
	int procLimit = 1000; //Default
	struct rlimit rlim;
    if (getrlimit(RLIMIT_NPROC, &rlim) == -1) {
        perror("getrlimit failure");
        return EXIT_FAILURE;
    }
    
    if (rlim.rlim_cur != RLIM_INFINITY) { 
		procLimit = (int) rlim.rlim_cur;
		
	}
	//create array to hold child processes.
	*/
	

	struct commandLine *currCommand;
	char *statusVar;
	statusVar = (char *) calloc((25),sizeof(char));
	sprintf(statusVar,"exit status 0");
	pid_t childPid = -1;
	int   childStatus;

	//printf("initial statusVar: %s",statusVar);
	//fflush(stdout);
	/*	
	//Handle SIGINT and SIGSTP
	struct sigaction SIGINTact = {0}, SIGSTPact = {0};
	//register handler
	SIGINTact.sa_handler = handleSIGINT;
	SIGINTact.sa_flags = 0;
	sigaction(SIGINT, &SIGINTact,NULL);
*/

	while (true)
	{
		currCommand = parse_input();

		// handle arguments
		if (currCommand->argc == 0)
		{
			// blank line
			printf("blank line\n");
			fflush(stdout);
		}
		else if (currCommand->argv[0][0] == '#')
		{
			printf("comment line\n");
			fflush(stdout);
		}
		// Handle the command types
		else if (!strcmp(currCommand->argv[0], "exit"))
		{
			printf("caught exit: %d:%s", currCommand->argc, currCommand->argv[0]);
			fflush(stdout);
		}

		//Handle cd : DONE
		else if (!strcmp(currCommand->argv[0], "cd"))
		{
			if(currCommand->argc == 2) {
				if(chdir(currCommand->argv[1]) == -1){
					sprintf(statusVar,"exit status 1 longer");
				}
				else {
					sprintf(statusVar,"exit status 0");
				}
			}
			else {  //REF: Google AI summary, prompt "cd to home directory linux c"
				const char *homeDir = getenv("HOME");
				if(chdir(homeDir) == -1){
					sprintf(statusVar,"exit status 1 longer");
				}
				else {
					sprintf(statusVar,"exit status 0");
				}
			}
				
		}

		//Handle Status: DONE
		else if (!strcmp(currCommand->argv[0], "status"))
		{
			printf("%s",statusVar);
			printf("\n");
			fflush(stdout);
		}
		else
		//Handle other command
		{
			/*
			for (int i = 0; i < currCommand->argc; i++)
			{
				printf("argNum: %d arg: %s ", i, currCommand->argv[i]);
				fflush(stdout);
				//execve(currCommand->argv[0], currCommand->argv,pathVar);
				
				
			}
			*/	
			
			childPid = fork();
			
			if(childPid == -1){
				perror("fork() failed!");
				sprintf(statusVar,"exit status 0");
			} else if(childPid == 0){
				// Child process
				int execRes;
				execRes = execvp(currCommand->argv[0], currCommand->argv);
				if( execRes == -1) {
					sprintf(statusVar,"exit status 1 program");
				}
				else if (execRes == 0){
					sprintf(statusVar,"exit status 0 program");
				}
				else {
					sprintf(statusVar,"exit status %d program",execRes);
				}
				//sleep(10);
				printf("child finished");
				fflush(stdout);
				
			} else{
				printf("Child's pid = %d\n", childPid);
				//Never executes after here, why?
				childPid = waitpid(childPid, &childStatus, 0);
				printf("waitpid returned value %d\n", childPid);
				
				if(WIFEXITED(childStatus)){
					printf(" In WIFEXITED");
					printf("Child %d exited normally with status %d\n", childPid, WEXITSTATUS(childStatus));
						//printf(statusVar,"exit status %d",WEXITSTATUS(childStatus));
						sprintf(statusVar,"exit status 0 WIFEXITED");
					} else{
					printf("Child %d exited abnormally due to signal %d\n", childPid, WTERMSIG(childStatus));
						sprintf(statusVar,"terminated by signal %d",WTERMSIG(childStatus));
					}
					
				fflush(stdout);
				
			}
			
			
			
		}

	}
	return EXIT_SUCCESS;
}

void handleSIGINT(int sigNum){
	char* msg = "Caught SIGINT, add function\n";
	write(STDOUT_FILENO, msg, strlen(msg));
}