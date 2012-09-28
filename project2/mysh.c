#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void flush(void){
	char c;
	while((c = getchar()) != '\n' && c != EOF)
		/* discard */ ;
}

void stripEndNewLine(char *usrInput) {
	int i = 0;
	
	// this loop helps to get last valid char entered
	while(usrInput[i] != '\0') {
		i++; 
	}
	
	if(usrInput[i-1] == '\r' || usrInput[i-1] == '\n') {
		usrInput[i-1] = '\0'; // replace carriage ret and newline with null
	}
}

void printError() {
	char error_message[30] = "An error has occured\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
}

// my own exit function
void myExit(int status) {
	exit(status);
}

// my own cd function
int cd(char *args) {
	int retStatus;
	if(args != NULL) {
		// 0 successful, -1 error
		retStatus = chdir(args);
	} else {
		// cd to home dir
		args = getenv("HOME"); 
		retStatus = chdir(args);
	}
	return retStatus;
} 

// my own pwd function
// exit 0 on success, -1 if fails
int pwd(void) {
	char cwdBuf[1024];
	char *pwdOut;
	
	pwdOut = getcwd(cwdBuf, sizeof(cwdBuf));
	
	if(pwdOut == NULL) { // if getcwd fail
		return -1;
	} else {
		printf("%s\n", pwdOut);
		return 0;
	}
	return 0;
}

int main(int argc, char **argv) {

	char *batchFile = "no/such/file";
	
	if(argc == 2 ) { // batch mode
	
		batchFile = argv[1];
		printf("Batchfiles : %s\n", batchFile);	
		
	} else if(argc == 1) {
	
		// interactive mode
		printf("mysh> ");
		
		char usrInput[512];  
		char *exitStr = "exit\n";
		char *newLine = "\n";
		char *cmdCd = "cd";
		char *cmdPwd = "pwd";
		char *cmdExit = "exit";
		char *token[512]; // very bad i know!
		
		while((fgets(usrInput, sizeof(usrInput), stdin) != NULL) && (strcmp(usrInput, exitStr) != 0)) {
			
			if(strlen(usrInput) == 1) { // empty command (carriage ret.)? continue!
				printf("mysh> "); // dont forget prompt!
				continue;
			}
			
			stripEndNewLine(usrInput);
			char *cmdArg = strtok(usrInput, " ");
			
			if(cmdArg == NULL) { // user enter space only?
				printf("mysh> "); // dont forget prompt!
				continue;
			}
			
			int c = 0;
			
			while(cmdArg != NULL) {
				
				if(strcmp(cmdArg, newLine) != 0) {
					token[c] = strdup(cmdArg);
				} else {
					cmdArg = strtok(NULL, " "); // get next token
					continue; 
				}

				cmdArg = strtok(NULL, " ");
				c++; // increase counter
			}
			
			token[c] = NULL; // terminate with NULL!
			
			int i = 0;
			while(token[i] != NULL) {
				printf("token[%d] : %s\n", i, token[i]);
				i++;
			}
			
			// check whether the cmd is built in or not
			if(strcmp(token[0], cmdCd) == 0) { // cd?
				int st = cd(token[1]);
				
				if(st == -1) {
					printError();
				}
			} else if (strcmp(token[0], cmdPwd) == 0) { // pwd?
				int st = pwd();
				
				if(st == -1) {
					printError();
				}
			} else if (strcmp(token[0], cmdExit) == 0) { // exit?
				myExit(0);
			} else { // not built in, use child
				
				pid_t childpid = fork();
				
				if(childpid >= 0) { // succeed
					if(childpid == 0) { // child
						execvp(token[0], token);
						printError();

					} else { // parent
						// wait for child
						if(waitpid(childpid, NULL, 0) != childpid) {
							printError();
						}
					}
				} else {
					printError();
				}
			}
			
			flush(); // use my own flush!			
			printf("mysh> ");
		
		}
		
		printf("exit it is then..\n");
		myExit(0);

	} else {
		// ERROR invalid # of arguments
		printError();
	}
	
	return 0;
}
