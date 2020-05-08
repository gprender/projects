/* 
 * gshell.c - A barebones command line shell
 
 * Written by Graeme Prendergast, Spring 2019
 * This code is largely based on Stephen Brennan's tutorial:
 * https://brennan.io/2015/01/16/write-a-shell-in-c/ 
 */

#include <signal.h>
#include <string.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>


/* ----------------------------
 *      BUILT-IN COMMANDS
 * ---------------------------- */

// Change the current working directory
int gshell_cd(char** args) {

    if (args[1] == NULL) { 
        printf("Additional arguments required for cd!\n"); 
    }
    else {
        if (chdir(args[1]) != 0) { perror("Bad cd"); }
    }

    return 1;
}


// Print the current working directory
int gshell_pwd(char** args) {

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    printf("%s\n", cwd);

    return 1;
}


// Print out a set of helpful tips
int gshell_help(char** args) {

    printf("Welcome to gshell!\n\n");
    printf("Here are this shell's built-in commands:\n");
    printf("\tcd <dir> : Change the current working directory to <dir>\n");
    printf("\tpwd : Print the current working directory\n");
    printf("\thelp : You already know what this does, ya silly goose\n");
    printf("\texit : Exits the shell\n");
    printf("\tset <var> <value> : Set an environment variable <var> to <value>\n");
    printf("\tunset <var> : Deletes the binding for <var>\n");

    return 1;
}


// Exit the shell
int gshell_exit(char** args) { exit(0); }


// Set an environment variable
int gshell_set(char** args) {
    
	if (args[1] == NULL || args[2] == NULL) {
		printf("<var> and <value> arguments are required\n");
	}
	else {
		if (setenv(args[1], args[2], 0) != 0) { perror("Bad set"); }
	}
	
    return 1;
}


// Unset an environment variable
int gshell_unset(char** args) {
    
	if (args[1] == NULL) {
		printf("<var> argument is required\n");
	}
	else {
		if (unsetenv(args[1]) != 0) { perror("Bad unset"); }
	}
	
    return 1;
}


// Split a string into an array of strings by the given delimiter. Like Python!
char** str_split(char* str, char* delim, int buffer_size) {

    char** tokens = malloc(sizeof(char*) * buffer_size);
    char* t = NULL;
    int position = 0;

    if (!tokens || !str) { return NULL; }

    t = strtok(str, delim);
    while (t != NULL) {
        tokens[position] = t;
        position++;

        if (position >= buffer_size) { return NULL; }

        t = strtok(NULL, delim);
    }
    tokens[position] = NULL;

    return tokens;
}


/* Read a single line of user input from stdin, within the given size limit.
 * Note that if this size limit is exceeded, the function just returns NULL
 * signify an error, but gshell will continue running. */
char* read_stdin(int buffer_size) {

    char* buffer = malloc(sizeof(char) * buffer_size);
    int position = 0;
    int c = -1;

    if (!buffer) { return NULL; }

    // We read 1 char at a time here so we can detect ctrl+d
    while (1) {
        c = getchar();
		
		// Exit immediately if we read a ctrl+d (EOF)
		if (c == EOF) { 
			printf("\n");
			exit(0);
 		}

        else if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } 
        else { buffer[position] = c; }
        position++;

        if (position >= buffer_size) { return NULL; }
    }
}


// Run an external command, if it exists
int run_external(char** cmd) {
	
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) { // We're in the child process!

		/* Here, we reset the child's signal handler to default. This lets
		gshell retain its unique signal handling without affecting the child. */
		signal(SIGINT, SIG_DFL);

		if (execvp(cmd[0], cmd) == -1) {
			perror("Bad external command");
		}
		exit(1);
    }
    else if (pid < 0) {
        perror("A process fork went terribly wrong");
    }
    else { // We're in the parent process!
		waitpid(pid, &status, WUNTRACED);
    }
	
	return 1;
}


// Run a given built-in command, or pass it along if it's external
int run_command(char** cmd) {

    // Check if anything went wrong while parsing the command
    if (!cmd) { return 1; }
    char* base = cmd[0];
    if (!base) { return 1; }

    /* I know this isn't elegant, but it's so much simpler than dealing 
       with all the junk that goes along with function pointers. */
    if (strcmp(base, "cd") == 0) { return gshell_cd(cmd); }
    else if (strcmp(base, "pwd") == 0) { return gshell_pwd(cmd); }
    else if (strcmp(base, "help") == 0) { return gshell_help(cmd); }
    else if (strcmp(base, "exit") == 0) { return gshell_exit(cmd); }
    else if (strcmp(base, "set") == 0) { return gshell_set(cmd); }
    else if (strcmp(base, "unset") == 0) { return gshell_unset(cmd); }
    else { return run_external(cmd); }
}


// Wait for user input, and respond accordingly
void main_loop() {
    char* stdin_line = NULL;
    char** cmd = NULL;
    int status = 1; 

    do {
        printf("> ");
        stdin_line = read_stdin(512);
        cmd = str_split(stdin_line, " \t\r\n\a", 512);
        status = run_command(cmd);

        free(stdin_line);
        free(cmd);

    } while (status);
}


// Customize signal handling, and read+run .gshellrc
void gshell_init() {

	signal(SIGINT, SIG_IGN); // Ignore all ctrl+c signals

	char* home_path = getenv("HOME");
	char* rc_path = strcat(home_path, "/.gshellrc");

	FILE* rcfile = fopen(rc_path, "r");
	if (rcfile) {
		char** cmd = NULL;
        size_t buffer_size = 512;
		char* buffer = malloc(sizeof(char) * buffer_size);
		int line_length = -1;
		
        // Reading the whole line at once, since we don't care about ctrl+d here
		while ((line_length = getline(&buffer, &buffer_size, rcfile)) != -1 ) {
			cmd = str_split(buffer, " \t\r\n\a", line_length);

            printf("? ");
            for (int i=0; cmd[i]; i++) { printf("%s ", cmd[i]); }
            printf("\n");

			run_command(cmd);

			free(buffer);
			free(cmd);
		}
		if (buffer) { free(buffer); }
	}
	else { printf("Error opening .gshellrc; skipping init"); }
}


int main() {
	gshell_init();

    main_loop();

    return 0;
}
