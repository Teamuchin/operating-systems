#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// fifoQueueInitialization
#define MAX_STRING_LENGTH 100
// Define the FIFO queue structure for strings
typedef struct {
  char **data;  // Array of string pointers
  int capacity; // Maximum capacity of the queue
  int front;    // Index of the front element
  int rear;     // Index of the rear element
  int size;     // Current number of elements in the queue
} StringQueue;

// Function to create a new string queue with given capacity
StringQueue *createStringQueue(int capacity) {
  StringQueue *queue = (StringQueue *)malloc(sizeof(StringQueue));
  if (queue == NULL) {
    return NULL;
  }

  // Allocate memory for the array of string pointers
  queue->data = (char **)malloc(capacity * sizeof(char *));
  if (queue->data == NULL) {
    fprintf(stderr, "Memory allocation failed for queue data array\n");
    free(queue);
    return NULL;
  }

  // Initialize each string pointer to NULL
  for (int i = 0; i < capacity; i++) {
    queue->data[i] = NULL;
  }

  queue->capacity = capacity;
  queue->front = 0;
  queue->rear = -1;
  queue->size = 0;

  return queue;
}

// Function to check if queue is full
bool isFull(StringQueue *queue) { return (queue->size == queue->capacity); }

// Function to check if queue is empty
bool isEmpty(StringQueue *queue) { return (queue->size == 0); }

// Function to add a string to the queue (enqueue)
bool enqueue(StringQueue *queue, const char *str) {
  if (isFull(queue)) {
    return false;
  }

  // Circular increment of rear
  queue->rear = (queue->rear + 1) % queue->capacity;

  // Allocate memory for the new string
  queue->data[queue->rear] = (char *)malloc((strlen(str) + 1) * sizeof(char));
  if (queue->data[queue->rear] == NULL) {
    // Revert the rear index
    queue->rear = (queue->rear - 1 + queue->capacity) % queue->capacity;
    return false;
  }

  // Copy the string
  strcpy(queue->data[queue->rear], str);
  queue->size++;
  return true;
}

// Function to remove a string from the queue (dequeue)
char *dequeue(StringQueue *queue) {
  if (isEmpty(queue)) {
    return NULL;
  }

  char *str = queue->data[queue->front];
  queue->data[queue->front] = NULL; // Set to NULL after dequeuing
  queue->front = (queue->front + 1) % queue->capacity;
  queue->size--;

  return str;
}

// Function to get the front string without removing it
char *peek(StringQueue *queue) {
  if (isEmpty(queue)) {
    return NULL;
  }

  return queue->data[queue->front];
}

// Function to display the queue contents
void displayQueue(StringQueue *queue) {
  if (isEmpty(queue)) {
    return;
  }
  int count = queue->size;
  int index = queue->front;
  while (count > 0) {
    printf("[%d] %s\n", (queue->size - count + 1), queue->data[index]);
    // Fix the decrement with modulo for circular array
    index = (index + 1 + queue->capacity) % queue->capacity;
    count--;
  }
}

// Function to free the memory allocated for the queue
void destroyQueue(StringQueue *queue) {
  if (queue) {
    if (queue->data) {
      // Free each string
      for (int i = 0; i < queue->capacity; i++) {
        if (queue->data[i] != NULL) {
          free(queue->data[i]);
        }
      }
      free(queue->data);
    }
    free(queue);
  }
}
// Automatic append function to handle historyqueue
void queueHistAppend(StringQueue *queue, const char *str) {
  if (!enqueue(queue, str)) {
    char *old = dequeue(queue);
    if (old != NULL) {
      free(old);
    }
    enqueue(queue, str);
  }
}

// fifoQueue end
//
//
//
//
//
//
// cd command integration
int changedir(char *path) {
  if (chdir(path) == 0) {
    setenv("PWD", path, 1);
    return 1;
  } else {
    char cwd[255];
    getcwd(cwd, 255);
    char subpath[255];
    strcpy(subpath, cwd);
    strcat(subpath, path);
    if (chdir(subpath) == 0) {
      setenv("PWD", subpath, 1);
      return 0;
      // relative path feature
    } else {
      printf("cd Command Fail!\n");
      return 1;
    }
  }
}
// cwd function
int currentWorkDir() {
  char cwd[255];
  getcwd(cwd, 255);
  printf("%s\n", cwd);
  return 0;
}
int inputShowHistory(StringQueue *history) {
  displayQueue(history);
  return 0;
}
// Exit function
void shellExit() { exit(0); }

// Executes non-implemented functions
int execOther(char **args) {
  if (args[0] == NULL) {
    return 0;
  }

  pid_t pid;
  int status;

  pid = fork();

  if (pid < 0) {
    return -1;
  } else if (pid == 0) {
    // Create fork and change childs process to given function from args
    if (execvp(args[0], args) == -1) {
      exit(1);
    }
  } else {
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      // Return the actual exit code from the child process for logical and
      // operator
      return WEXITSTATUS(status);
    } else {
      // Child terminated abnormally
      return 1;
    }
  }

  return 0;
}

int main() {
  int pipefd[2];
  pid_t forkingid;
  char shellinput[100] = "";
  const char *defaulthomedir = getenv("HOME");
  const char *pwddir = getenv("PWD");
  bool background = false;
  bool forking = false;
  bool logicanding = false;
  int separatorloc = 0;
  StringQueue *inputHistory = createStringQueue(10);

  // Main loop
  while (1) {
    background = false;
    forking = false;
    logicanding = false;
    printf("shell322>");
    if (fgets(shellinput, sizeof(shellinput), stdin) == NULL) {
      // Handle EOF or read error
      if (feof(stdin)) {
        printf("\nExiting shell.\n");
        break;
      }
      perror("fgets error");
      continue;
    }
    shellinput[strcspn(shellinput, "\n")] = 0;
    if (strcmp(shellinput, "exit") == 0) {
      shellExit();
    }
    // Command appended to history before command execution
    queueHistAppend(inputHistory, shellinput);
    // Creating copy of whole input because tokenizing voids actual input
    // array(storing copy for further possible use)
    char inputcpy[100];
    strcpy(inputcpy, shellinput);

    // Tokenizing string for later use

    char *tokenarray[10] = {NULL};
    int arg_count = 0;
    char *token = strtok(shellinput, " ");

    while (token != NULL && arg_count < 9) {
      tokenarray[arg_count++] = token;
      token = strtok(NULL, " ");
    }
    // Last element is null because of execvp
    tokenarray[arg_count] = NULL;

    // Skip empty commands
    if (arg_count == 0) {
      continue;
    }
    // Checking for special arguments("|","&&","&")
    if (arg_count > 0 && strcmp(tokenarray[arg_count - 1], "&") == 0) {
      background = true;
      tokenarray[--arg_count] = NULL;
    }

    for (int i = 0; tokenarray[i] != NULL; i++) {
      if (strcmp(tokenarray[i], "|") == 0) {
        forking = true;
        separatorloc = i;
      } else if (strcmp(tokenarray[i], "&&") == 0) {
        logicanding = true;
        separatorloc = i;
      }
    }
    // All function check complete
    // Default check condition
    if (forking == false && logicanding == false && background == false) {
      if (strcmp(tokenarray[0], "cd") == 0) {
        if (tokenarray[1] == NULL) {
          changedir(getenv("HOME"));
        } else {
          changedir(tokenarray[1]);
        }
      } else if (strcmp(tokenarray[0], "pwd") == 0) {
        currentWorkDir();
      } else if (strcmp(tokenarray[0], "history") == 0) {
        inputShowHistory(inputHistory);
      } else {
        execOther(tokenarray);
      } /*




       Forking condition*/
    } else if (forking == true && logicanding == false && background == false) {
      if (pipe(pipefd) == -1) {
        perror("pipe error");
        continue;
      }

      // Prepare command arrays for both sides of the pipe
      char *cmd1[10] = {NULL};
      char *cmd2[10] = {NULL};

      // Copy first command (before the pipe symbol)
      for (int i = 0; i < separatorloc; i++) {
        cmd1[i] = tokenarray[i];
      }
      cmd1[separatorloc] = NULL;

      // Copy second command (after the pipe symbol)
      int j = 0;
      for (int i = separatorloc + 1; i < arg_count; i++) {
        cmd2[j++] = tokenarray[i];
      }
      cmd2[j] = NULL;

      // Create child process (will execute the second command)
      pid_t pid = fork();

      if (pid < 0) {
        perror("fork error");
        close(pipefd[0]);
        close(pipefd[1]);
        continue;
      } else if (pid == 0) {
        // Child process: input comes from pipe
        close(pipefd[1]); // Close write end as child only reads

        // Redirect stdin to pipe read end
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
          perror("dup2 error");
          exit(EXIT_FAILURE);
        }
        close(pipefd[0]); // Close original descriptor after duplication

        // Execute second command based on its type
        if (strcmp(cmd2[0], "cd") == 0) {
          if (cmd2[1] == NULL) {
            changedir(getenv("HOME"));
          } else {
            changedir(cmd2[1]);
          }
          exit(EXIT_SUCCESS);
        } else if (strcmp(cmd2[0], "pwd") == 0) {
          currentWorkDir();
          exit(EXIT_SUCCESS);
        } else if (strcmp(cmd2[0], "history") == 0) {
          inputShowHistory(inputHistory);
          exit(0);
        } else {
          execvp(cmd2[0], cmd2);
          perror("execvp failed for second command");
          exit(EXIT_FAILURE);
        }
      } else {
        // Parent process: execute first command with output to pipe
        close(pipefd[0]); // Close read end as parent only writes

        // Save the original stdout for restoration later
        int saved_stdout = dup(STDOUT_FILENO);

        // Redirect stdout to pipe write end
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
          perror("dup2 error");
          close(pipefd[1]);
          dup2(saved_stdout, STDOUT_FILENO);
          close(saved_stdout);
          continue;
        }
        close(pipefd[1]); // Close original descriptor after duplication

        // Execute first command based on its type
        int result = 0;
        if (strcmp(cmd1[0], "cd") == 0) {
          if (cmd1[1] == NULL) {
            result = changedir(getenv("HOME"));
          } else {
            result = changedir(cmd1[1]);
          }
        } else if (strcmp(cmd1[0], "pwd") == 0) {
          currentWorkDir();
        } else if (strcmp(cmd1[0], "history") == 0) {
          inputShowHistory(inputHistory);
        } else {
          execOther(cmd1);
        }

        // Restore original stdout
        fflush(stdout);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);

        // Wait for the child process (second command) to complete
        waitpid(pid, NULL, 0);
      } /*




       Logical-and condition*/
    } else if (forking == false && logicanding == true && background == false) {
      bool returnfail = false;
      // Prepare command arrays for both sides of the pipe
      char *cmd1[10] = {NULL};
      char *cmd2[10] = {NULL};

      // Copy first command (before the pipe symbol)
      for (int i = 0; i < separatorloc; i++) {
        cmd1[i] = tokenarray[i];
      }
      cmd1[separatorloc] = NULL;

      // Copy second command (after the pipe symbol)
      int j = 0;
      for (int i = separatorloc + 1; i < arg_count; i++) {
        cmd2[j++] = tokenarray[i];
      }
      cmd2[j] = NULL;
      // Any of error return values of the functions will result of returnfail
      // becoming true so second command will not execute
      if (strcmp(cmd1[0], "cd") == 0) {
        if (cmd1[1] == NULL) {
          if (changedir(getenv("HOME")) != 0) {
            returnfail = true;
          }
        } else {
          if (changedir(cmd1[1]) != 0) {
            returnfail = true;
          }
        }
      } else if (strcmp(cmd1[0], "pwd") == 0) {
        if (currentWorkDir() != 0) {
          returnfail = true;
        }
      } else if (strcmp(cmd1[0], "history") == 0) {
        if (inputShowHistory(inputHistory) != 0) {
          returnfail = true;
        }
      } else {
        if (execOther(cmd1) != 0) {
          returnfail = true;
        }
      }
      // Second command part
      if (!returnfail) {
        if (strcmp(cmd2[0], "cd") == 0) {
          if (cmd2[1] == NULL) {
            changedir(getenv("HOME"));
          } else {
            changedir(cmd2[1]);
          }
        } else if (strcmp(cmd2[0], "pwd") == 0) {
          currentWorkDir();
        } else if (strcmp(cmd2[0], "history") == 0) {
          inputShowHistory(inputHistory);
        } else {
          execOther(cmd2);
        }
      } /*







       Background processing*/
    } else if (forking == false && logicanding == false && background == true) {
      pid_t bgfork = fork();
      if (bgfork == 0 /*child*/) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
          // Redirect child stdin and stdout to null so it does not affect our
          // current console while executing
          dup2(devnull, STDOUT_FILENO);
          dup2(devnull, STDERR_FILENO);
          close(devnull);
        }
        if (strcmp(tokenarray[0], "cd") == 0) {
          if (tokenarray[1] == NULL) {
            changedir(getenv("HOME"));
          } else {
            changedir(tokenarray[1]);
          }
        } else if (strcmp(tokenarray[0], "pwd") == 0) {
          currentWorkDir();
        } else if (strcmp(tokenarray[0], "history") == 0) {
          inputShowHistory(inputHistory);
        } else {
          execOther(tokenarray);
        }
        // We do not wait for child here so shell can continue while process
        // executes on background
      } else {
        printf("Process with id [%d] running at background\n", bgfork);
      }
    }
  }

  return 0;
}
