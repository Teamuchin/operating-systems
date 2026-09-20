#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// task structure
typedef struct {
  unsigned long producerid;
  int taskid;
} Task;

Task *createTask(int taskid, unsigned long producerid) {
  Task *task = (Task *)malloc(sizeof(Task));
  task->taskid = taskid;
  task->producerid = producerid;
  return task;
}

void setprodid(Task *task, unsigned long id) { task->producerid = id; }
void settaskid(Task *task, int id) { task->taskid = id; }
unsigned long getprodid(Task *task) { return task->producerid; }
int gettaskid(Task *task) { return task->taskid; }

// round queue definiton
typedef struct {
  Task *array;  // Array to store queue elements
  int capacity; // Maximum capacity of the queue
  int front;    // Index of the front element
  int rear;     // Index of the rear element
  int size;     // Current size of the queue
} CircularQueue;

/**
 * Creates a new circular queue with the given capacity
 */
CircularQueue *createQueue(int capacity) {
  CircularQueue *queue = (CircularQueue *)malloc(sizeof(CircularQueue));
  if (!queue) {
    return NULL;
  }

  queue->array = (Task *)malloc(capacity * sizeof(Task));
  if (!queue->array) {
    free(queue);
    return NULL;
  }

  queue->capacity = capacity;
  queue->front = -1;
  queue->rear = -1;
  queue->size = 0;

  return queue;
}

/**
 * Checks if the queue is empty
 */
bool isEmpty(CircularQueue *queue) { return (queue->size == 0); }

/**
 * Checks if the queue is full
 */
bool isFull(CircularQueue *queue) { return (queue->size == queue->capacity); }

/**
 * Returns the current size of the queue
 */
int getSize(CircularQueue *queue) { return queue->size; }

/**
 * Adds an element to the queue. If the queue is full,
 * it will overwrite the oldest element.
 */
void enqueue(CircularQueue *queue, Task item) {
  if (isEmpty(queue)) {
    // If queue is empty, set front and rear to 0
    queue->front = 0;
    queue->rear = 0;
  } else {
    // Move rear to the next position, wrapping around if necessary
    queue->rear = (queue->rear + 1) % queue->capacity;

    // If queue is full, move front as well (overwriting the oldest element)
    if (isFull(queue)) {
      queue->front = (queue->front + 1) % queue->capacity;
    }
  }

  // Add the new item
  queue->array[queue->rear] = item;

  // Update size, making sure not to exceed capacity
  if (queue->size < queue->capacity) {
    queue->size++;
  }
}

/**
 * Removes and returns the front element from the queue.
 */
Task dequeue(CircularQueue *queue) {
  // Get the front item
  Task item = queue->array[queue->front];

  // If this is the last item, reset front and rear
  if (queue->front == queue->rear) {
    queue->front = -1;
    queue->rear = -1;
  } else {
    // Move front to the next position
    queue->front = (queue->front + 1) % queue->capacity;
  }

  queue->size--;
  return item;
}

/**
 * Returns the front element without removing it.
 */
Task peek(CircularQueue *queue) { return queue->array[queue->front]; }

/**
 * Frees all memory allocated for the queue
 */
void destroyQueue(CircularQueue *queue) {
  if (queue) {
    if (queue->array) {
      free(queue->array);
    }
    free(queue);
  }
}

// Thread args structure
typedef struct {
  CircularQueue *low_queue;
  CircularQueue *med_queue;
  CircularQueue *high_queue;
  int *task_counter;
  int *total_tasks;
  pthread_t *prodid;

  // Synchronization objects
  pthread_mutex_t *low_mutex, *med_mutex, *high_mutex;
  sem_t *low_tasks, *med_tasks, *high_tasks;
  sem_t *low_slots, *med_slots, *high_slots;

  // For tracking completion
  bool *producers_done;
  int *tasks_consumed;

  // Added task count mutex for consumers
  pthread_mutex_t *task_count_mutex;
} ThreadArgs;

ThreadArgs *createArg(CircularQueue *lq, CircularQueue *mq, CircularQueue *hq,
                      int *taskcount, int *total_tasks, pthread_t *prodid) {
  ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
  if (!args) {
    return NULL;
  }

  // Initialize all fields to NULL/0 first
  memset(args, 0, sizeof(ThreadArgs));

  // Set the provided values
  args->low_queue = lq;
  args->med_queue = mq;
  args->high_queue = hq;
  args->task_counter = taskcount;
  args->total_tasks = total_tasks;
  args->prodid = prodid;
  args->producers_done = NULL;   // Will be set later for consumers
  args->tasks_consumed = NULL;   // Will be set later for consumers
  args->task_count_mutex = NULL; // Will be set later

  return args;
}

// producer threads
void *producers(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  int total = *(args->total_tasks);
  int *count = args->task_counter;
  CircularQueue *lq = args->low_queue;
  CircularQueue *mq = args->med_queue;
  CircularQueue *hq = args->high_queue;

  // Initialize random seed for this thread
  unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)pthread_self();

  while (1) {
    // Atomically check and increment counter
    pthread_mutex_lock(args->task_count_mutex);
    if (*count >= total) {
      pthread_mutex_unlock(args->task_count_mutex);
      break;
    }
    (*count)++;
    int current_task = *count;
    pthread_mutex_unlock(args->task_count_mutex);

    int priority = rand_r(&seed) % 3; // Thread-safe random number generation

    // Select queue and semaphores based on priority
    CircularQueue *queue;
    pthread_mutex_t *mutex;
    sem_t *slots_sem, *tasks_sem;

    switch (priority) {
    case 0: // High priority
      queue = hq;
      mutex = args->high_mutex;
      slots_sem = args->high_slots;
      tasks_sem = args->high_tasks;
      break;
    case 1: // Medium priority
      queue = mq;
      mutex = args->med_mutex;
      slots_sem = args->med_slots;
      tasks_sem = args->med_tasks;
      break;
    case 2: // Low priority
      queue = lq;
      mutex = args->low_mutex;
      slots_sem = args->low_slots;
      tasks_sem = args->low_tasks;
      break;
    }

    // Wait for an available slot
    sem_wait(slots_sem);
    pthread_mutex_lock(mutex);

    // Create and enqueue task
    Task task = {pthread_self(), current_task};
    enqueue(queue, task);

    // Log the operation
    printf("Producer %lu: Enqueued task %d with priority %d\n", pthread_self(),
           task.taskid, priority);

    pthread_mutex_unlock(mutex);
    sem_post(tasks_sem); // Signal that a task is available

    usleep(10000); // Small delay between tasks (10ms)
  }

  free(args);
  return NULL;
}

// consumer threads
void *consumers(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  CircularQueue *lq = args->low_queue;
  CircularQueue *mq = args->med_queue;
  CircularQueue *hq = args->high_queue;
  bool got_task = false;

  while (1) {
    got_task = false;

    // Try high priority first
    if (sem_trywait(args->high_tasks) == 0) {
      pthread_mutex_lock(args->high_mutex);
      Task task = dequeue(hq);
      pthread_mutex_unlock(args->high_mutex);
      sem_post(args->high_slots);

      printf("Consumer %lu: Consumed high priority task %d from producer %lu\n",
             pthread_self(), task.taskid, task.producerid);

      pthread_mutex_lock(args->task_count_mutex);
      (*(args->tasks_consumed))++;
      pthread_mutex_unlock(args->task_count_mutex);

      got_task = true;
    }
    // Try medium priority
    else if (sem_trywait(args->med_tasks) == 0) {
      pthread_mutex_lock(args->med_mutex);
      Task task = dequeue(mq);
      pthread_mutex_unlock(args->med_mutex);
      sem_post(args->med_slots);

      printf(
          "Consumer %lu: Consumed medium priority task %d from producer %lu\n",
          pthread_self(), task.taskid, task.producerid);

      pthread_mutex_lock(args->task_count_mutex);
      (*(args->tasks_consumed))++;
      pthread_mutex_unlock(args->task_count_mutex);

      got_task = true;
    }
    // Try low priority
    else if (sem_trywait(args->low_tasks) == 0) {
      pthread_mutex_lock(args->low_mutex);
      Task task = dequeue(lq);
      pthread_mutex_unlock(args->low_mutex);
      sem_post(args->low_slots);

      printf("Consumer %lu: Consumed low priority task %d from producer %lu\n",
             pthread_self(), task.taskid, task.producerid);

      pthread_mutex_lock(args->task_count_mutex);
      (*(args->tasks_consumed))++;
      pthread_mutex_unlock(args->task_count_mutex);

      got_task = true;
    }

    // Check if we should exit
    if (!got_task) {
      if (*(args->producers_done)) {
        // Check if any tasks remain in any queue
        pthread_mutex_lock(args->high_mutex);
        pthread_mutex_lock(args->med_mutex);
        pthread_mutex_lock(args->low_mutex);

        bool all_empty = isEmpty(hq) && isEmpty(mq) && isEmpty(lq);

        pthread_mutex_unlock(args->low_mutex);
        pthread_mutex_unlock(args->med_mutex);
        pthread_mutex_unlock(args->high_mutex);

        if (all_empty) {
          break;
        }
      }
      usleep(1000); // Short sleep if no task found (1ms)
    } else {
      usleep(50000); // Process time for task (50ms)
    }
  }

  free(args);
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 5) {
    printf("Usage: %s <producers> <consumers> <queue_size> <tasks>\n", argv[0]);
    return 1;
  }

  bool producers_done = false;
  int tasks_consumed = 0;
  pthread_mutex_t task_count_mutex;
  pthread_mutex_init(&task_count_mutex, NULL);

  CircularQueue *lowPrioTasks = createQueue(atoi(argv[3]));
  CircularQueue *medPrioTasks = createQueue(atoi(argv[3]));
  CircularQueue *highPrioTasks = createQueue(atoi(argv[3]));

  if (!lowPrioTasks || !medPrioTasks || !highPrioTasks) {
    printf("Failed to create queues\n");
    return 1;
  }

  int prodcount = atoi(argv[1]);
  int conscount = atoi(argv[2]);
  int taskcount = atoi(argv[4]);
  int taskct = 0;

  pthread_mutex_t low_mutex, med_mutex, high_mutex;
  sem_t low_tasks, med_tasks, high_tasks;
  sem_t low_slots, med_slots, high_slots;

  pthread_t producerlist[prodcount];
  pthread_t conslist[conscount];

  // Initialize mutexes
  pthread_mutex_init(&low_mutex, NULL);
  pthread_mutex_init(&med_mutex, NULL);
  pthread_mutex_init(&high_mutex, NULL);

  // Initialize semaphores
  int queue_size = atoi(argv[3]);
  sem_init(&low_tasks, 0, 0);
  sem_init(&med_tasks, 0, 0);
  sem_init(&high_tasks, 0, 0);
  sem_init(&low_slots, 0, queue_size);
  sem_init(&med_slots, 0, queue_size);
  sem_init(&high_slots, 0, queue_size);

  // Create both producer and consumer threads
  for (int i = 0; i < prodcount; i++) {
    ThreadArgs *producer_args =
        createArg(lowPrioTasks, medPrioTasks, highPrioTasks, &taskct,
                  &taskcount, &producerlist[i]);

    producer_args->low_mutex = &low_mutex;
    producer_args->med_mutex = &med_mutex;
    producer_args->high_mutex = &high_mutex;
    producer_args->low_tasks = &low_tasks;
    producer_args->med_tasks = &med_tasks;
    producer_args->high_tasks = &high_tasks;
    producer_args->low_slots = &low_slots;
    producer_args->med_slots = &med_slots;
    producer_args->high_slots = &high_slots;
    producer_args->task_count_mutex = &task_count_mutex;
    producer_args->producers_done = &producers_done;

    pthread_create(&producerlist[i], NULL, producers, producer_args);
  }

  // Create consumer threads immediately
  for (int i = 0; i < conscount; i++) {
    ThreadArgs *consumer_args =
        createArg(lowPrioTasks, medPrioTasks, highPrioTasks, &taskct,
                  &taskcount, &conslist[i]);

    consumer_args->low_mutex = &low_mutex;
    consumer_args->med_mutex = &med_mutex;
    consumer_args->high_mutex = &high_mutex;
    consumer_args->low_tasks = &low_tasks;
    consumer_args->med_tasks = &med_tasks;
    consumer_args->high_tasks = &high_tasks;
    consumer_args->low_slots = &low_slots;
    consumer_args->med_slots = &med_slots;
    consumer_args->high_slots = &high_slots;
    consumer_args->producers_done = &producers_done;
    consumer_args->tasks_consumed = &tasks_consumed;
    consumer_args->task_count_mutex = &task_count_mutex;

    pthread_create(&conslist[i], NULL, consumers, consumer_args);
  }

  // Wait for all producers to finish
  for (int i = 0; i < prodcount; i++) {
    pthread_join(producerlist[i], NULL);
  }

  // Set producers_done flag to true
  producers_done = true;

  // Wait for all consumers to finish
  for (int i = 0; i < conscount; i++) {
    pthread_join(conslist[i], NULL);
  }

  // Cleanup
  destroyQueue(lowPrioTasks);
  destroyQueue(medPrioTasks);
  destroyQueue(highPrioTasks);

  // Destroy mutexes
  pthread_mutex_destroy(&low_mutex);
  pthread_mutex_destroy(&med_mutex);
  pthread_mutex_destroy(&high_mutex);
  pthread_mutex_destroy(&task_count_mutex);

  // Destroy semaphores
  sem_destroy(&low_tasks);
  sem_destroy(&med_tasks);
  sem_destroy(&high_tasks);
  sem_destroy(&low_slots);
  sem_destroy(&med_slots);
  sem_destroy(&high_slots);

  printf("All tasks completed: %d tasks consumed\n", tasks_consumed);

  return 0;
}