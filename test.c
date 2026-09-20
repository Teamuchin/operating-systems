#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the maximum length of each string
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
    fprintf(stderr, "Memory allocation failed for queue\n");
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
    printf("Queue is full. Cannot enqueue \"%s\"\n", str);
    return false;
  }

  // Circular increment of rear
  queue->rear = (queue->rear + 1) % queue->capacity;

  // Allocate memory for the new string
  queue->data[queue->rear] = (char *)malloc((strlen(str) + 1) * sizeof(char));
  if (queue->data[queue->rear] == NULL) {
    fprintf(stderr, "Memory allocation failed for string\n");
    // Revert the rear index
    queue->rear = (queue->rear - 1 + queue->capacity) % queue->capacity;
    return false;
  }

  // Copy the string
  strcpy(queue->data[queue->rear], str);
  queue->size++;

  printf("Enqueued \"%s\" to the queue\n", str);
  return true;
}

// Function to remove a string from the queue (dequeue)
char *dequeue(StringQueue *queue) {
  if (isEmpty(queue)) {
    printf("Queue is empty. Cannot dequeue\n");
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
    printf("Queue is empty\n");
    return NULL;
  }

  return queue->data[queue->front];
}

// Function to display the queue contents
void displayQueue(StringQueue *queue) {
  if (isEmpty(queue)) {
    printf("Queue is empty\n");
    return;
  }

  printf("Queue elements: \n");
  int count = queue->size;
  int index = queue->rear;

  while (count > 0) {
    printf("  \"%s\"\n", queue->data[index]);
    // Fix the decrement with modulo for circular array
    index = (index - 1 + queue->capacity) % queue->capacity;
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
void queueHistAppend(StringQueue *queue, const char *str) {
  if (!enqueue(queue, str)) {
    dequeue(queue);
    enqueue(queue, str);
  }
}

// Main function to demonstrate the string queue operations
int main() {
  // Create a queue with capacity 5
  StringQueue *queue = createStringQueue(5);

  // Enqueue strings
  queueHistAppend(queue, "Hello");
  queueHistAppend(queue, "World");
  queueHistAppend(queue, "FIFO");
  queueHistAppend(queue, "Queue");
  queueHistAppend(queue, "1");
  queueHistAppend(queue, "2");
  queueHistAppend(queue, "3");
  queueHistAppend(queue, "4");

  // Display the queue
  displayQueue(queue);

  // Dequeue strings
  char *str1 = dequeue(queue);
  if (str1 != NULL) {
    printf("Dequeued: \"%s\"\n", str1);
    free(str1); // Don't forget to free the dequeued string
  }

  char *str2 = dequeue(queue);
  if (str2 != NULL) {
    printf("Dequeued: \"%s\"\n", str2);
    free(str2);
  }

  // Display the queue after dequeue
  displayQueue(queue);

  // Enqueue more strings
  enqueue(queue, "Data");
  enqueue(queue, "Structure");
  enqueue(queue, "in C"); // This should show that the queue is full

  // Display the queue after enqueue
  displayQueue(queue);

  // Display the front element
  char *front = peek(queue);
  if (front != NULL) {
    printf("Front element: \"%s\"\n", front);
  }

  // Free the allocated memory
  destroyQueue(queue);

  return 0;
}