#include "lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>

#define ARG_MAX sysconf(_SC_ARG_MAX)

void parse_args(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        if (opt == 'v') {
            printf("Shell Version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
            exit(0);
        }
    }
}

char *get_prompt(const char *env) {
    char *prompt = getenv(env);
    if (prompt == NULL) {
        prompt = "shell> ";
    }
    return strdup(prompt);
}

char **cmd_parse(const char *line) {
    char **cmd = malloc(ARG_MAX * sizeof(char *));
    if (!cmd) return NULL;

    char *token, *line_copy = strdup(line);
    int i = 0;

    token = strtok(line_copy, " ");
    while (token && i < ARG_MAX - 1) {
        cmd[i++] = strdup(token);
        token = strtok(NULL, " ");
    }
    cmd[i] = NULL;

    free(line_copy);
    return cmd;
}

void cmd_free(char **cmd) {
    if (!cmd) return;
    for (int i = 0; cmd[i] != NULL; i++) {
        free(cmd[i]);
    }
    free(cmd);
}

char *trim_white(char *line) {
    while (isspace((unsigned char)*line)) line++;
    if (*line == 0) return line;
    char *end = line + strlen(line) - 1;
    while (end > line && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return line;
}

int change_dir(char **args) {
    if (args[1] == NULL) {
        const char *home = getenv("HOME");
        if (!home) {
            struct passwd *pw = getpwuid(getuid());
            home = pw ? pw->pw_dir : NULL;
        }
        if (home && chdir(home) != 0) {
            perror("cd");
            return -1;
        }
    } else if (chdir(args[1]) != 0) {
        perror("cd");
        return -1;
    }
    return 0;
}

bool do_builtin(struct shell *sh, char **argv) {
    if (!argv[0]) return false;

    if (strcmp(argv[0], "exit") == 0) {
        sh_destroy(sh);
        exit(0);
    } else if (strcmp(argv[0], "cd") == 0) {
        return change_dir(argv) == 0;
    } else if (strcmp(argv[0], "history") == 0) {
        HIST_ENTRY **hist = history_list();
        if (hist) {
            for (int i = 0; hist[i]; i++) {
                printf("%d  %s\n", i + history_base, hist[i]->line);
            }
        }
        return true;
    }
    return false;
}

void sh_init(struct shell *sh) {
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_is_interactive = isatty(sh->shell_terminal);

    if (sh->shell_is_interactive) {
        while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp()))
            kill(-sh->shell_pgid, SIGTTIN);

        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        sh->shell_pgid = getpid();
        setpgid(sh->shell_pgid, sh->shell_pgid);
        tcsetpgrp(sh->shell_terminal, sh->shell_pgid);

        tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
    }

    sh->prompt = get_prompt("MY_PROMPT");
}

void sh_destroy(struct shell *sh) {
    free(sh->prompt);
}

void execute_command(char **cmd) {
    if (!cmd || !cmd[0]) return;

    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

        execvp(cmd[0], cmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
}
