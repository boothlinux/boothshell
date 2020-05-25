/***************************************************************************//**

Booth Shell v0.0.1
August 26th, 2019
Licensed under the GPL v2

*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  Function Declarations for builtin shell commands:
 */
int boothshell_cd(char **args);
int boothshell_help(char **args);
int boothshell_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "quit",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &boothshell_cd,
  &boothshell_help,
  &boothshell_exit
};

int boothshell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int boothshell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "Booth Shell: Use \"cd\" to change directories, such as \"cd /usr/bin\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("boothshell");
    }
  }
  return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int boothshell_help(char **args)
{
  int i;
  system ("tput clear");
  printf("\n");
  printf("Booth Shell\n");
  printf("\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("\n");
  printf("The following are built in:\n");
  printf("\n");

  for (i = 0; i < boothshell_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
  printf("\n");
  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int boothshell_exit(char **args)
{
  return 0;
}
int boothshell_quit(char **args)
{
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int boothshell_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("boothshell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("boothshell");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int boothshell_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < boothshell_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return boothshell_launch(args);
}

#define BOOTHSHELL_RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *boothshell_read_line(void)
{
  int bufsize = BOOTHSHELL_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "Booth Shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += BOOTHSHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "Booth Shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define BOOTHSHELL_TOK_BUFSIZE 64
#define BOOTHSHELL_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **boothshell_split_line(char *line)
{
  int bufsize = BOOTHSHELL_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "Booth Shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, BOOTHSHELL_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += BOOTHSHELL_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "Booth Shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, BOOTHSHELL_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void boothshell_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = boothshell_read_line();
    args = boothshell_split_line(line);
    status = boothshell_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Clear the screen and run command loop.
  system ("tput clear");
  printf("Welcome to Booth Shell.\n");
  printf("\n");
  printf("Booth Shell is a very simplistic and lightweight shell.\n");
  printf("Booth Shell does not contain many features you mat be used to.\n");
  printf("But it is very portable. Enjoy!\n");
  printf("\n");
  boothshell_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

