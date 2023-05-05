#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens* tokens);
int cmd_help(struct tokens* tokens);
<<<<<<< HEAD
int cmd_cwd(struct tokens* tokens);
int cmd_cd(struct tokens* tokens);
int cmd_exec(struct tokens* tokens[], int num_tokens);


=======
>>>>>>> a833c955957b6de8b6eccb11437b6b1efa956b3e

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens* tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t* fun;
  char* cmd;
  char* doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
<<<<<<< HEAD
    {cmd_cwd, "cwd", "print the current working directory"},
    {cmd_cd, "cd", "change the current working directory"},
    {cmd_exec, "exec", "execute a program"},
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens* token){
=======
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens* tokens) {
>>>>>>> a833c955957b6de8b6eccb11437b6b1efa956b3e
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens* tokens) { exit(0); }

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
<<<<<<< HEAD
  
  return -1;
}


/* Get current working directory */
int cmd_cwd(unused struct tokens* tokens) {
  char* cwd[1024];
  getcwd(cwd, 1024);
  printf("%s", cwd);
  printf("\n");
  return 1;
}

/* Change current working directory */
int cmd_cd(struct tokens* tokens) {
  char* path = tokens_get_token(tokens, 1);
  if (path == NULL) {
    printf("cd: missing argument\n");
    return 1;
  }
  if (chdir(path) == -1) {
    printf("cd: %s: No such file or directory\n", path);
  }
  return 1;
}


char* get_path(struct tokens* token){
  char *path = getenv("PATH");
  char *path_copy = strdup(path);
  char *token_path = strtok(path_copy, ":");
  char *file_name = tokens_get_token(token, 0);

  while(token_path != NULL){
    char *file_path = malloc(strlen(token_path) + strlen(file_name) + 2);
    strcpy(file_path, token_path);
    strcat(file_path, "/");
    strcat(file_path, file_name);
    if(access(file_path, F_OK) != -1){
      free(path_copy);
      return file_path;
    }
    free(file_path);
    token_path = strtok(NULL, ":");
  }
  free(path_copy);
  return NULL;
}

char* get_path_final(struct tokens* tokens){
    char *path = malloc(strlen(tokens_get_token(tokens, 0)) + 2);
    if (tokens_get_token(tokens,0)[0] == '/') {
      path = tokens_get_token(tokens, 0);
    } else {
      path = get_path(tokens);
    }
    if (path == NULL) {
      printf("command not found \n");
      exit(0);
    }
    return path;
}


int cmd_exec(struct tokens* tokens_list[],int num_tokens){
  pid_t pid;


  if ((pid = fork()) < 0) {
    printf("fork error");
  } else if (pid == 0) {
    // Set Child Process Group
    signal(SIGINT, SIG_DFL);
    if (!tcgetpgrp(STDIN_FILENO) == getpgrp()) {
      signal(SIGTTOU, SIG_IGN);
      tcsetpgrp(STDIN_FILENO, getpgrp());
    }

    for (int j = 0; j < num_tokens; j++){

      bool pipe_out = j < num_tokens - 1;
      bool pipe_in = j > 0;

      struct tokens* tokens = tokens_list[j];
      
      /* Identify the correct path */
      char *path = get_path_final(tokens);
      /* End Path Identification */


      /* Identify the correct arguments and redirection */
      bool need_stdin = false;
      bool need_stdout = false;
      char *file_in = NULL;
      char *stdout_to_parent;
      char* argv[1024];
      int i;
      for (i = 0; i < tokens_get_length(tokens); i++) {
        if (strcmp(tokens_get_token(tokens, i), "<") == 0) {
          need_stdin = true;
          file_in = tokens_get_token(tokens, i + 1);
          i++;
        }
        else if (strcmp(tokens_get_token(tokens, i), ">") == 0) {
          need_stdout = true;
          stdout_to_parent = tokens_get_token(tokens, i + 1);
          i++;
        } else {
          argv[i] = tokens_get_token(tokens, i);
        }  
      }
      argv[i] = NULL;

      if (need_stdin) {
        int fd_in = open(file_in, O_RDONLY);
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
      }
    
      if (need_stdout) {
        int fd_stdin_out = open(stdout_to_parent, O_WRONLY | O_CREAT | O_TRUNC, 00700);
        dup2(fd_stdin_out, STDOUT_FILENO);
        close(fd_stdin_out);
      }
      /* End Argument and Redirection Identification */



      /* Support for the pipe */
      if (pipe_out){
        int pipefd[2];
        pipe(pipefd);
        pid_t p = fork();
        if (p == 0){
          dup2(pipefd[1], STDOUT_FILENO);
          close(pipefd[0]);
          close(pipefd[1]);
          execv(path, argv);
          exit(0);
        } else {
          dup2(pipefd[0], STDIN_FILENO);
          close(pipefd[0]);
          close(pipefd[1]);
          wait(NULL);
        }
      }
      if (!pipe_out) {
          if (execv(path, argv) < 0) {
          printf("execv error \n");
        }
      }
    }
    
    exit(0);

  } else {
    signal(SIGINT, SIG_IGN);

    wait(NULL);
    signal(SIGINT, SIG_DFL);
  }
  
  return 1;
}





=======
  return -1;
}

>>>>>>> a833c955957b6de8b6eccb11437b6b1efa956b3e
/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
<<<<<<< HEAD
    /* If the shell is not currently in the foregroundexi, we must pause the shell until it becomes a
=======
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
>>>>>>> a833c955957b6de8b6eccb11437b6b1efa956b3e
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

<<<<<<< HEAD




=======
>>>>>>> a833c955957b6de8b6eccb11437b6b1efa956b3e
int main(unused int argc, unused char* argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
<<<<<<< HEAD
    struct tokens *tokens = tokenize(line);
    int fundex = lookup(tokens_get_token(tokens, 0));
    if (fundex >= 0) {
        cmd_table[fundex].fun(tokens);
        tokens_destroy(tokens);
    } else {
      int multiple_commands = 1;
      for(int i = 0; i < strlen(line); i++){
        if(line[i] == '|'){
          multiple_commands++;
        }
      }
      struct tokens *tokens[multiple_commands + 1];
      char *token = strtok(line, "|");
      int i = 0;
      while (token != NULL) {
        tokens[i] = tokenize(token);
        token = strtok(NULL, "|");
        i++;
      }
      tokens[i] = NULL;
      cmd_exec(tokens, multiple_commands);

      for (int i = 0; i < multiple_commands; i++) {
        tokens_destroy(tokens[i]);
      }
    }
    


=======
    struct tokens* tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      /* REPLACE this to run commands as programs. */
      fprintf(stdout, "This shell doesn't know how to run programs.\n");
    }
>>>>>>> a833c955957b6de8b6eccb11437b6b1efa956b3e

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);
<<<<<<< HEAD
    
=======

    /* Clean up memory */
    tokens_destroy(tokens);
>>>>>>> a833c955957b6de8b6eccb11437b6b1efa956b3e
  }

  return 0;
}
