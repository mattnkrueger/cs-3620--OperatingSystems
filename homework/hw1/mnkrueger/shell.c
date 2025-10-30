/**
 * @file shell.c
 * @author Matt Krueger (mnkrueger@uiowa.edu)
 * @brief Homework 1 Task 1. Implementation of Fork + Exec + Wait pattern to simulate a shell on sample txt testing files.
 * @version 0.1
 * @date 2025-09-30
 * 
 * THIS FILE WAS MODIFIED FROM THE ORIGINAL 'shell.c' 
 * FROM THE Notion: "Homework 1 - Implementing a Shell with Linux Process API"
 * https://www.notion.so/pengjiang-hpc/Homework-1-Implementing-a-Shell-with-Linux-Process-API-14501d7f16804f95b423dbd9d82c24b6
 */
// ========================== 
// UNMODIFIED FROM HOMEWORK 1 
// ========================== 
#define _GNU_SOURCE //this is needed to be able to use execvpe 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <fcntl.h>


// ========================== 
// UNMODIFIED FROM HOMEWORK 1 
// ========================== 
typedef struct {
  char* binary_path;
  char* stdin;
  char* stdout;
  char* arguments;
  short wait;
} command;

// ========================== 
// UNMODIFIED FROM HOMEWORK 1 
// ========================== 
//function prototypes
void print_parsed_command(command);
short parse_command(command*, char*);

// ========================== 
// UNMODIFIED FROM HOMEWORK 1 
// ========================== 
// read a line from a file
short getlinedup(FILE* fp, char** value){
  char* line = NULL;
  size_t n = 0;
  //get one line
  int ret = getline(&line, &n, fp);

  if(ret == -1){
    //the file ended
    return 0;
  }
  //remove \n at the end
  line[strcspn(line, "\n")] = '\0';
  //duplicate the string and set value
  *value = strdup(line);
  free(line);

  return 1;
}

// ========================== 
// UNMODIFIED FROM HOMEWORK 1 
// ========================== 
//parse a command_file and set a corresponding command data structure
short parse_command(command* parsed_command, char* cmdfile){
  FILE* fp = fopen(cmdfile, "r");
  if(!fp){
    //the file does not exist
    return 0;
  }

  char* value;
  short ret;
  int intvalue;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  parsed_command->binary_path = value;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  parsed_command->stdin = value;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  parsed_command->stdout = value;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  parsed_command->arguments = value;

  ret = getlinedup(fp,&value);
  if(!ret){
    fclose(fp); return 0;
  }
  intvalue = atoi(value);
  if(intvalue != 0 && intvalue != 1){
    fclose(fp); return 0;
  }
  parsed_command->wait = (short)intvalue;

  return 1;
}

// ========================== 
// UNMODIFIED FROM HOMEWORK 1 
// ========================== 
//useful for debugging
void print_parsed_command(command parsed_command){
  printf("----------\n");
  printf("binary_path: %s\n", parsed_command.binary_path);
  printf("stdin: %s\n", parsed_command.stdin);
  printf("stdout: %s\n", parsed_command.stdout);
  printf("arguments: %s\n", parsed_command.arguments);
  printf("wait: %d\n", parsed_command.wait);
}

// ========================== 
// UNMODIFIED FROM HOMEWORK 1 
// ========================== 
void free_command(command cmd){
  free(cmd.binary_path);
  free(cmd.stdin);
  free(cmd.stdout);
  free(cmd.arguments);
}

// =================================================== 
//
//              START OF MODIFICATIONS 
//
// =================================================== 

/**
 * @brief command processor for Homework 1
 * 
 * @param parsed_command 
 * @param env 
 * @return pid_t 
 */
pid_t process_command(command parsed_command, char* env[]) {
  // TASK 1. USE FORK TO CREATE THE CHILD PROCESS
  // - forking gets the parent's resources: mem, files, etc
  // - ret 0=success, or 1=failure
  // - forks nondeterministic. if 0 -> child proc, else parent proc
  pid_t child_pid = fork();
  if (child_pid < 0) {
    perror("fork failed");
    exit(1);
  }

  // CHILD
  else if (child_pid == 0) {
    
    // TASK 2: REDIRECT STDIN AND/OR STDOUT
    // redirect the standard input
    if (strlen(parsed_command.stdin) > 0) {
      int fd_in = open(parsed_command.stdin, 0);

      if (fd_in < 0) {
        perror("open stdin failed"); exit(1); 
      }

      dup2(fd_in, 0); 
      close(fd_in);
    }

    // redirect the standard output
    if (strlen(parsed_command.stdout) > 0) {
      int fd_out = open(parsed_command.stdout, 1);

      if (fd_out < 0) {
        perror("open stdout failed"); exit(1); 
      }

      dup2(fd_out, 1); 
      close(fd_out);
    }

    // setup arguments 
    char* args[3];
    args[0] = parsed_command.binary_path;
    args[1] = (strlen(parsed_command.arguments) > 0) ? parsed_command.arguments : NULL;
    args[2] = NULL;

    // TASK 3. CALL EXECVE/VARIANTS 
    // execve replaces the current process image with a new program
    // - takes an argument list/vector, and an environment list
    // - the process id does not change but code, data, and heap do (as it runs new proc)
    // (using execvpe variant as it combines the path within its search... less arg passing)
    execvpe(parsed_command.binary_path, args, env);
    perror("execvpe failed");
    exit(1);
  } 
  
  // PARENT
  else {
    wait(NULL);   // forcing child's execution first
    printf("New child process started <%d>\n", child_pid);
    return child_pid;
  }
}

int main(int argc, char *argv[], char* env[]) {

  // parse each command
  for(int ncommand=1; ncommand<argc; ncommand++){
    command parsed_command;
    int ret = parse_command(&parsed_command, argv[ncommand]);
    if (!ret){
      printf("Command file <%s> is invalid\n", argv[ncommand]);
      continue;
    }

    pid_t children[argc];
    int child_count=0;

    // spawn child
    pid_t child_pid = process_command(parsed_command, env);
    children[child_count++] = child_pid;

    // TASK 4. call waitpid to wait for the child process to execute
    // waiting allows the parent to collect the child's (zombie processes), exit status
    if (parsed_command.wait == 1){
      int status;
      pid_t w = waitpid(child_pid, &status, 0);
      if (w > 0){
        printf("Child process <%d> terminated with exit code <%d>\n", w, WEXITSTATUS(status));
      }
    }

    free_command(parsed_command);
  }

  // TASK 5. wait until all launched commands are terminated
  int status;
  pid_t wpid;
  while ((wpid = wait(&status)) > 0){
    printf("Child process <%d> terminated with exit code <%d>\n", wpid, WEXITSTATUS(status));
  }

  return 0;
}