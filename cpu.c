#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define LINE_LEN 256
pthread_mutex_t CPU = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t IO = PTHREAD_MUTEX_INITIALIZER;

sem_t mutex;
sem_t io_mutex;
sem_t cpu_mutex; 

// args for thread functions
typedef struct arg_struct {
  char *arg1;
  char *arg2;
  int quantum;
} arg_struct;

// doubly linked list data structure
typedef struct Node {
  int proc_id;
  int priority;
  int burst_number;
  int *cpu_IO;
  struct timeval start_time, end_time, wait_start; // turnaround time
  struct Node *next;
  struct Node *prev;
} Node;

struct Node *ready_head; // head of the cpu queue
struct Node *ready_tail; // tail of the cpu queue

struct Node *io_head; // head of the io queue
struct Node *io_tail; // tail of the io queue

long int turnaround_time = 0; 
long int num_proc = 0; // tracks number of processes
long int wait_total = 0; // tracks cpu queue wait times

int flag = 0; // to determine which queue to use | 0 for ready queue | 1 for IO queue
int stop_flag = 0;

// function to swap two nodes within a queue (linked list)
void swap_nodes(Node *a, Node *b) {
  // temp node to store node being swapped
  int temp_priority = a->priority;
  int temp_burst = a->burst_number;
  int *temp_cpuIO = a->cpu_IO;

  struct timeval temp_start_time = a->start_time;
  struct timeval temp_end_time = a->end_time;
  struct timeval temp_wait_start = a->wait_start;

  // swap all data in nodes
  int temp_id = a->proc_id;
  a->proc_id = b->proc_id;
  b->proc_id = temp_id;
  
  a->priority = b->priority;
  b->priority = temp_priority;

  a->burst_number = b->burst_number;
  b->burst_number = temp_burst;

  a->cpu_IO = b->cpu_IO;
  b->cpu_IO = temp_cpuIO;

  a->start_time = b->start_time;
  b->start_time = temp_start_time;

  a->end_time = b->end_time;
  b->end_time = temp_end_time;

  a->wait_start = b->wait_start;
  b->wait_start = temp_wait_start;

  return;
}

// function to free all nodes in linked list
void free_nodes(Node *head) {
  Node *temp;

  while (head != NULL) {
    temp = head;
    head = head->next;
    free(temp->cpu_IO);
    free(temp);
  }
}

// function to add process to a queue (linked list)
void en_q(int flag_q, int priority, int burst, int *cpu_IO, char *sched_alg,
           struct timeval start_time, int proc_id) {
  
  // creates new Node
  struct Node *new_node = (Node *)malloc(sizeof(Node));

  // add data into the new node
  new_node->priority = priority;
  new_node->burst_number = burst;
  new_node->cpu_IO = cpu_IO;
  new_node->next = NULL;
  new_node->prev = NULL;
  new_node->start_time = start_time;
  new_node->proc_id = proc_id;

  if (flag_q == 0) { // add to ready queue for cpu bursts
    gettimeofday(&new_node->wait_start, NULL); // start wait time
    if (ready_head == NULL) {
      ready_tail = new_node;
      ready_head = new_node;
    } else {
      ready_tail->next = new_node;
      new_node->prev = ready_tail;
      ready_tail = new_node;
      
      // PR sort based on priority
      if (strcmp(sched_alg, "PR") == 0) {
        Node *ptr = ready_tail;
   
        while (ptr->prev != NULL) {
          if (ptr->priority > ptr->prev->priority) {
            swap_nodes(ptr, ptr->prev);
            ptr = ptr->prev;
          } else {
            break;
          }
        }
        
      // SJF sort based on CPU IO index 0
      } else if (strcmp(sched_alg, "SJF") == 0) {
        Node *ptr = ready_tail;

        int cnt = 0;
        while (ptr != NULL) {
          if (ptr->cpu_IO[cnt] != -1) {
            if (ptr->cpu_IO[cnt] < ptr->prev->cpu_IO[cnt]) {
              swap_nodes(ptr, ptr->prev);
              ptr = ptr->prev;
            } else {
              break;
            }
          }
          cnt++;
        }
      }
    }
   
  } else if (flag_q == 1) { // add to io queue for io bursts
    if (io_head == NULL) {
      io_head = new_node;
      io_tail = new_node;
    } else {
      io_tail->next = new_node;
      new_node->prev = io_tail;
      io_tail = new_node;
    }
  } else {
    printf("Error: Wrong queue");
  }
  return;
}

// dequeued the head of the queue
struct Node *de_q(int flag) {
  
  struct Node *head = NULL;

  // check which queue to use
  if (flag == 0) {
    head = ready_head;
  } else if (flag == 1) {
    head = io_head;
  }

  // create a temporary node
  struct Node *temp = (Node *)malloc(sizeof(Node));
  temp->cpu_IO = head->cpu_IO;
  temp->burst_number = head->burst_number;
  temp->priority = head->priority;
  temp->cpu_IO = malloc(temp->burst_number * sizeof(*temp->cpu_IO));
  for (int x = 0; x < temp->burst_number; x++) {
    temp->cpu_IO[x] = head->cpu_IO[x];
  }

  temp->start_time = head->start_time;
  temp->wait_start = head->wait_start;
  temp->proc_id = head->proc_id;
  
  temp->next = NULL;
  temp->prev = NULL;

  // detach the old head
  if (flag == 0){
    if (head->next != NULL) {
      ready_head = ready_head->next;
      head->next = NULL;
      ready_head->prev = NULL;
    } else {
      ready_head = NULL;
    }
  } else if (flag == 1) {
    if (head->next != NULL) {
      io_head = io_head->next;
      head->next = NULL;
      io_head->prev = NULL;
    } else {
      io_head = NULL;
    }
  }
  
  free_nodes(head);
  return temp;
}

// function for input thread to read lines from input file
void *read_file(void *args_param) {
  struct arg_struct *args = (struct arg_struct *)args_param;

  char *input_file = args->arg1;
  char *alg = args->arg2;
  FILE *fp = fopen((char *)input_file, "r");
  
  // check if opening file was a success
  if (fp == NULL) {
    printf("ERROR: Cannot open file\n");
    pthread_exit(NULL);
  }

  char line[LINE_LEN];

  while (fgets(line, LINE_LEN, fp) != NULL) {
    if (strcmp("stop\n", line) == 0){
      stop_flag = 1;
      sem_post(&mutex);
      sem_post(&io_mutex);
      break;
    }
    char *keyword = strtok(line, " ");
    char *priority;
    char *burst;
    char *cpu_IO;
    
    if (strcmp("stop", keyword) == 0) {
      
      break;
      
    } else if (strcmp("sleep", keyword) == 0) {
      int sleep_count = atoi(strtok(NULL, " "));
      
      usleep(sleep_count * 1000);
      
    } else { // "proc"
      
      num_proc++;
      priority = strtok(NULL, " ");
      burst = strtok(NULL, " ");
      cpu_IO = strtok(NULL, " ");
      int *cpu_IO_arr;
      cpu_IO_arr = malloc(atoi(burst) * sizeof(*cpu_IO_arr));
      int i = 0;
      
      while (cpu_IO != NULL) {
        cpu_IO_arr[i] = atoi(cpu_IO);
        cpu_IO = strtok(NULL, " ");
        i++;
      }
    
      struct timeval temp_timer;
      gettimeofday(&temp_timer, NULL);
      pthread_mutex_lock(&CPU);
     
      en_q(0, atoi(priority), atoi(burst), cpu_IO_arr, alg, temp_timer, num_proc);

      pthread_mutex_unlock(&CPU);
      sem_post(&mutex);
      // send signal to cpu thread to begin
    }
  }
  fclose(fp);
  return NULL;
}

// function for CPU thread to get processes from ready queue and sleep for the next IO burst time
void *cpu_thread(void *args_param) {
  int completed = 0;
  struct arg_struct *args = (struct arg_struct *)args_param;
  int quantum = args->quantum;
  char *alg = args->arg2;
  int IO_CPU_FLAG = 1;
  struct Node *dq = NULL;
  
  sem_wait(&mutex);   
  if(stop_flag == 1){
  
    return NULL;
  }
  while (1) {
    if (ready_head == NULL && completed < num_proc){
      sem_wait(&cpu_mutex);
    }
    
    if (ready_head != NULL) {
      struct timeval temp_end;
      gettimeofday(&temp_end, NULL);

      wait_total = wait_total + (((temp_end.tv_usec - ready_head->wait_start.tv_usec) + 
              ((temp_end.tv_sec - ready_head->wait_start.tv_sec) * 1000000)) / 1000);
      pthread_mutex_lock(&CPU);
      dq = de_q(0);
      pthread_mutex_unlock(&CPU);

      struct timeval end_burst;
      int cnt = 0; // count number of bursts in process
      while (cnt < dq->burst_number) {
        
        if (dq->cpu_IO[cnt] != -1) {

          if (strcmp(alg, "RR") == 0) {
            if (dq->cpu_IO[cnt] > quantum){
              
              usleep(quantum * 1000);
              gettimeofday(&end_burst, NULL);
              dq->cpu_IO[cnt] = dq->cpu_IO[cnt] - quantum;
              IO_CPU_FLAG = 0;
            } else {
              usleep(dq->cpu_IO[cnt] * 1000);
              gettimeofday(&end_burst, NULL);
              IO_CPU_FLAG = 1;
              dq->cpu_IO[cnt] = -1;
            }         
          } else {

            // printf("sleeping for %d process %d index %d burst number %d\n", dq->cpu_IO[cnt], dq->proc_id, cnt, dq->burst_number);
   
            usleep(dq->cpu_IO[cnt] * 1000);
            gettimeofday(&end_burst, NULL);
            
            dq->cpu_IO[cnt] = -1;
          }
          break;
        }
        cnt++;
      }

      // if the process is completed
      if (cnt == dq->burst_number - 1) {
        gettimeofday(&dq->end_time, NULL);
        turnaround_time =
            turnaround_time +
            (((end_burst.tv_usec - dq->start_time.tv_usec) + 
            ((end_burst.tv_sec - dq->start_time.tv_sec) * 1000000))/ 1000);
        // printf("CPU PROCESS %d IS COMPLETE\n", dq->proc_id);
        
        completed++;
        if (completed == num_proc){
          free_nodes(dq);
          dq = NULL;
          break;
        }
      } else {
        // enqueueing into the I0 queue
        int temp_prio = dq->priority;
        int temp_bn = dq->burst_number;
        int *temp_cpu = malloc(temp_bn * sizeof(*temp_cpu));
        
        for (int x = 0; x < temp_bn; x++) {
          temp_cpu[x] = dq->cpu_IO[x];
        }
     
        int temp_proc_id = dq->proc_id;
        struct timeval temp_start_time = dq->start_time;
        pthread_mutex_lock(&IO);
        en_q(IO_CPU_FLAG, temp_prio, temp_bn, temp_cpu, alg, temp_start_time, temp_proc_id);
        pthread_mutex_unlock(&IO);
        sem_post(&io_mutex);
      }
      
      free_nodes(dq);
      dq = NULL;
    }

  }
  return NULL;
}

// function for IO thread to get processes from IO queue and sleep for the next IO burst time
void *io_thread(void *args_param) {
  
  int completed = 0;
  struct arg_struct *args = (struct arg_struct *)args_param;
  char *alg = args->arg2;
  struct Node *dq = NULL;
  
  sem_wait(&io_mutex);
  if (stop_flag == 1){
    return NULL;
  }
  while (1){
    if (io_head == NULL){
      sem_wait(&io_mutex);
    }
    if (io_head != NULL) {
      
      pthread_mutex_lock(&IO);
      dq = de_q(1);
      pthread_mutex_unlock(&IO);
  
      int cnt = 0;
      while (cnt < dq->burst_number) {
        if (dq->cpu_IO[cnt] != -1) {
          usleep(dq->cpu_IO[cnt] * 1000);
          dq->cpu_IO[cnt] = -1;
          break;
        }
        cnt++;
      }
      
      int temp_prio = dq->priority;
      int temp_bn = dq->burst_number;
      int *temp_cpu = malloc(temp_bn * sizeof(*temp_cpu));
      struct timeval temp_start_time = dq->start_time;
      for (int x = 0; x < temp_bn; x++) {
        temp_cpu[x] = dq->cpu_IO[x];
      }
    
      int temp_proc_id = dq->proc_id;
      
      pthread_mutex_lock(&CPU);
      en_q(0, temp_prio, temp_bn, temp_cpu, alg, temp_start_time, temp_proc_id);
      pthread_mutex_unlock(&CPU);
      sem_post(&cpu_mutex);
      if (cnt == dq->burst_number - 2) {
        completed++;
        if (completed == num_proc){
          free_nodes(dq);
          dq = NULL;
          break;
        }
      }
      free_nodes(dq);
      dq = NULL;
    }

  }
  return NULL;
}

int main(int argc, char *argv[]) {
  struct timeval run_start;
  gettimeofday(&run_start, NULL);
  char *input_file;

  char *sched_algo;
  sched_algo = argv[2];
  int quant = 0;

  // check for 6 args for RR
  if (strcmp(sched_algo, "RR") == 0) {
    if (argc != 7) {
      printf("ERROR: Wrong number of arguments entered\n");
      exit(1);
    }
    quant = atoi(argv[4]);
    input_file = argv[6];
  } else {
    if (argc != 5) {
      printf("ERROR: Wrong number of arguments entered\n");
      exit(1);
    }
    input_file = argv[4];
  }

  arg_struct args;
  args.arg1 = input_file;
  args.arg2 = sched_algo;
  args.quantum = quant;

  pthread_t thread_input;
  pthread_t thread_cpu;
  pthread_t thread_io;
  
  sem_init(&mutex, 0, 0);
  sem_init(&io_mutex, 0, 0);
  sem_init(&cpu_mutex, 0, 0);
  
  pthread_create(&thread_input, NULL, read_file, (void *)&args);
  pthread_create(&thread_cpu, NULL, cpu_thread, (void *)&args);
  pthread_create(&thread_io, NULL, io_thread, (void*)&args);

  pthread_join(thread_input, NULL);
  pthread_join(thread_cpu, NULL);
  pthread_join(thread_io, NULL);
  
  struct timeval run_end;
  gettimeofday(&run_end, NULL);

  float total_runtime = (((run_end.tv_usec - run_start.tv_usec) + 
              ((run_end.tv_sec - run_start.tv_sec) * 1000000)) / 1000);

  
  sem_destroy(&mutex);
  sem_destroy(&io_mutex);
  sem_destroy(&cpu_mutex);

  // free all nodes
  free_nodes(ready_head);
  free_nodes(io_head);

  // print out performance results after all threads are 
  if (stop_flag != 1){
    printf("Input File Name                  : %s\n", input_file);
    printf("CPU Scheduling Alg               : %s", sched_algo);
    // if algorithm is RR, also print out the quantum
    if (strcmp(sched_algo, "RR") == 0) {
      printf(" (%d)\n", quant);
    } else {
      printf("\n");
    }
    printf("Throughput                       : %0.03f ms\n", num_proc / total_runtime);
    printf("Avg. Turnaround Time             : %ld ms\n", turnaround_time / num_proc);
    printf("Avg. Waiting Time in Ready Queue : %ld ms\n", wait_total / num_proc);
  } else {
    printf("ERROR: No processes given\n");
  }
  
  return 0;
}
