#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Maximum length of a command
#define MAX_CMD_LEN 100
// Maximum number of arguments
#define MAX_ARGS 10
// Number of recent commands to remember
#define HISTORY_COUNT 10

// Array to store command history
char history[HISTORY_COUNT][MAX_CMD_LEN];
// Counter for the command history
int history_count = 0;

void add_history(char* cmd) {
    // Add command to history 
    strncpy(history[history_count % HISTORY_COUNT], cmd, MAX_CMD_LEN);
    // Increment history count
    history_count++;
}

void print_history() {
    printf("Command history:\n");
    // Determine where to start printing from
    int start = history_count >= 10 ? history_count - 10 : 0;
    // Print out the command history
    for (int i = start; i < history_count; ++i) {
        printf("%d %s\n", i + 1, history[i % HISTORY_COUNT]);
    }
}

void exec_cmd(char* args[], int bg) {
    // Fork the process to execute the command 
    pid_t pid = fork();
    if (pid < 0) {
        // Print error if fork failed
        perror("Fork failed");
    } else if (pid == 0) {
        // Execute the command in child process
        execvp(args[0], args);
        // Print error if exec failed
        perror("Exec failed");
        exit(1);
    } else {
        if (!bg) {
            // Wait for the child process to finish if it's a foreground process
            waitpid(pid, NULL, 0);
        }
    }
}

void parse_cmd(char* cmd) {
    add_history(cmd);
    char* args[MAX_ARGS + 1];
    char* token = strtok(cmd, " ");
    int i = 0;
    int bg = 0;

    // Parse command into argument
    while (token != NULL && i < MAX_ARGS) {
        args[i] = token;
        token = strtok(NULL, " ");
        ++i;
    }
    args[i] = NULL;
    if (args[i - 1][strlen(args[i - 1]) - 1] == '&') {
        // Set bg to 1 if the command should be run in the background
        bg = 1;
        args[i - 1][strlen(args[i - 1]) - 1] = 0;
    }

    if (strcmp(args[0], "cd") == 0) {
        // Get directory for cd command
        char* dir = args[1] ? args[1] : getenv("HOME");
        if (chdir(dir) != 0) {
            // Print error if cd command failed
            perror("cd failed");
        }
        return;
    }

    if (strcmp(args[0], "dir") == 0) {
        char cwd[MAX_CMD_LEN];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            // Print error if getcwd failed
            perror("getcwd() error");
        } else {
            // Print current working directory
            printf("Current working directory: %s\n", cwd);
        }
        return;
    }

    if (strcmp(args[0], "history") == 0) {
        // Print command history 
        print_history();
        return;
    }

    if (strcmp(args[0], "bye") == 0) {
        // Exit the shell
        exit(0);
    }

    exec_cmd(args, bg);
}

int main() {
    char cmd[MAX_CMD_LEN];
    while (1) {
        printf("myshell>");
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            break;
        }
        cmd[strlen(cmd) - 1] = 0; 
        // Parse and execute the command
        parse_cmd(cmd);
    }
    return 0;
}
