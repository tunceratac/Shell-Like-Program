#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 100
#define MAX_ARGS 10
#define HISTORY_COUNT 10

char history[HISTORY_COUNT][MAX_CMD_LEN];
int history_count = 0;

void add_history(char* cmd) {
    strncpy(history[history_count % HISTORY_COUNT], cmd, MAX_CMD_LEN);
    history_count++;
}

void print_history() {
    printf("Command history:\n");
    int start = history_count >= 10 ? history_count - 10 : 0;
    for (int i = start; i < history_count; ++i) {
        printf("%d %s\n", i + 1, history[i % HISTORY_COUNT]);
    }
}

void exec_cmd(char* args[], int bg) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
    } else if (pid == 0) {
        execvp(args[0], args);
        perror("Exec failed");
        exit(1);
    } else {
        if (!bg) {
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

    while (token != NULL && i < MAX_ARGS) {
        args[i] = token;
        token = strtok(NULL, " ");
        ++i;
    }
    args[i] = NULL;
    if (args[i - 1][strlen(args[i - 1]) - 1] == '&') {
        bg = 1;
        args[i - 1][strlen(args[i - 1]) - 1] = 0;
    }

    if (strcmp(args[0], "cd") == 0) {
        char* dir = args[1] ? args[1] : getenv("HOME");
        if (chdir(dir) != 0) {
            perror("cd failed");
        }
        return;
    }

    if (strcmp(args[0], "dir") == 0) {
        char cwd[MAX_CMD_LEN];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
        } else {
            printf("Current working directory: %s\n", cwd);
        }
        return;
    }

    if (strcmp(args[0], "history") == 0) {
        print_history();
        return;
    }

    if (strcmp(args[0], "bye") == 0) {
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
        parse_cmd(cmd);
    }
    return 0;
}
