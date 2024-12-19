#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

char *command_generator(const char *text, int state);
char **command_completion(const char *text, int start, int end);

char *commands[] = {
    "cd",
    "ls",
    "pwd",
    "cat",
    "exit",
    NULL
};

void change_directory(char *input) {
    char *token = strtok(NULL, " \n");
    if (token != NULL) {
        if (access(token, F_OK) != -1) {
            if (chdir(token) != 0) {
                perror("chdir");
            }
        } else {
            fprintf(stderr, "cd: no such file or directory: %s\n", token);
        }
    } else {
        fprintf(stderr, "cd: missing argument\n");
    }
}

void list_directory() {
    DIR *d;
    struct dirent *dir;
    struct stat file_stat;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (stat(dir->d_name, &file_stat) == 0) {
                if (S_ISDIR(file_stat.st_mode)) {
                    printf("\033[0;34m%s\033[0m\n", dir->d_name); // Blue for directories
                } else if (file_stat.st_mode & S_IXUSR) {
                    printf("\033[0;32m%s\033[0m\n", dir->d_name); // Green for executables
                } else {
                    printf("%s\n", dir->d_name); // Default color for other files
                }
            } else {
                perror("stat");
            }
        }
        closedir(d);
    } else {
        perror("opendir");
    }
}

void print_current_directory() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
}

void print_file_contents(char *input) {
    char *token = strtok(NULL, " \n");
    if (token != NULL) {
        FILE *file = fopen(token, "r");
        if (file != NULL) {
            char line[1024];
            while (fgets(line, sizeof(line), file) != NULL) {
                printf("%s", line);
            }
            fclose(file);
            printf("\n");
        } else {
            perror("fopen");
        }
    } else {
        fprintf(stderr, "cat: missing argument\n");
    }
}

void print_file_contents_with_syntax_highlighting(char *input) {
    char *token = strtok(NULL, " \n");
    if (token != NULL) {
        FILE *file = fopen(token, "r");
        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            char *file_contents = malloc(file_size + 1);
            fread(file_contents, 1, file_size, file);
            file_contents[file_size] = '\0';
            fclose(file);

            char command[1024];
            snprintf(command, sizeof(command), "pygmentize -l c -f terminal256 -O style=monokai");
            FILE *pipe = popen(command, "w");
            if (pipe != NULL) {
                fwrite(file_contents, 1, file_size, pipe);
                pclose(pipe);
            } else {
                printf("%s", file_contents);
            }
            free(file_contents);
        } else {
            perror("fopen");
        }
    } else {
        fprintf(stderr, "cat: missing argument\n");
    }
}

void clear() {
    printf("\033[H\033[J");
}

void print_history() {
    HIST_ENTRY **history = history_list();
    if (history) {
        for (int i = 0; history[i]; i++) {
            printf("%d: %s\n", i + 1, history[i]->line);
        }
    }
}

int loop() {
    rl_attempted_completion_function = command_completion;
    while(1) {
        char *input = readline("t-shell> ");
        if (input == NULL) {
            break;
        }
        if (strlen(input) > 0) {
            add_history(input);
        }
        char *command = strtok(input, " \n");
        if (command != NULL) {
            if (strcmp(command, "cd") == 0) {
                change_directory(input);
            } else if(strcmp(command, "ls") == 0) {
                list_directory();
            } else if(strcmp(command, "pwd") == 0) {
                print_current_directory();
            } else if(strcmp(command, "cat") == 0) {
                print_file_contents_with_syntax_highlighting(input);
            } else if(strcmp(command, "history") == 0) {
                print_history();
            } else if(strcmp(command, "clear") == 0) {
                clear();
            } else if (strcmp(command, "exit") == 0) {
                break;
            } else {
                pid_t pid = fork();
                if(pid == 0) {
                    char *args[1024];
                    int i = 0;
                    args[i++] = command;
                    char *arg;
                    while((arg = strtok(NULL, " \n")) != NULL) {
                        args[i++] = arg;
                    }
                    args[i] = NULL;
                    execvp(command, args);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                } else if(pid > 0) {
                    int status;
                    waitpid(pid, &status, 0);
                } else {
                    perror("fork");
                }
            }
        }
        free(input);
    }
    return 0;
}

char **command_completion(const char *text, int start, int end) {
    char **matches = NULL;

    if (start == 0) {
        matches = rl_completion_matches(text, command_generator);
    } else {
        matches = rl_completion_matches(text, rl_filename_completion_function);
    }

    return matches;
}

char *command_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = commands[list_index])) {
        list_index++;
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    loop();
    return 0;
}