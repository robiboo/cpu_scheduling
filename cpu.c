#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes
// ./cpu -alg FCFS -input input.txt
// ./cpu -alg RR -quantum 10 -input input.txt
// ./cpu -alg RR -quantum 30 -input input.txt

#define LINE_LEN 256

pthread_mutex_t CPU = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t IO = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t input = PTHREAD_MUTEX_INITIALIZER;
sem_t mutex;
sem_t io_mutex;
sem_t cpu_mutex;

typedef struct arg_struct {
  char *arg1;
  char *arg2;
} arg_struct;

// doubly linked list data structure
typedef struct Node {
  // int proc_id;
  int priority;
  int burst_number;
  int *cpu_IO;
  struct timeval start_time, end_time; // turnaround time
  int burst_tracker;                   // tracks the cpu burst times
  struct Node *next;
  struct Node *prev;
} Node;

struct Node *ready_head;
struct Node *ready_tail;

struct Node *io_head;
struct Node *io_tail;

int turnaround_time = 0;
int num_proc = 0;

int flag = 0;

// function to swap two nodes within a ready queue (linked list)
void swap_nodes(Node *a, Node *b) {
  // swap two nodes
  
  int temp_priority = a->priority;
  int temp_burst = a->burst_number;
  int *temp_cpuIO = a->cpu_IO;
  int temp_burst_tracker = a->burst_tracker;
  struct timeval temp_start_time = a->start_time;
  struct timeval temp_end_time = a->end_time;

  a->priority = b->priority;
  b->priority = temp_priority;

  a->burst_number = b->burst_number;
  b->burst_number = temp_burst;

  a->cpu_IO = b->cpu_IO;
  b->cpu_IO = temp_cpuIO;

  a->burst_tracker = b->burst_tracker;
  b->burst_tracker = temp_burst_tracker;

  a->start_time = b->start_time;
  b->start_time = temp_start_time;

  a->end_time = b->end_time;
  b->end_time = temp_end_time;

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
          int burst_tracker, struct timeval start_time) {
  // creates new Node
  struct Node *new_node = (Node *)malloc(sizeof(Node));

  // add data into the new node
  // new_node->proc_id = proc_id;
  new_node->priority = priority;
  new_node->burst_number = burst;
  new_node->cpu_IO = cpu_IO;
  new_node->next = NULL;
  new_node->prev = NULL;
  new_node->burst_tracker = burst_tracker;
  new_node->start_time = start_time;

  // printf("enq cpu io %d\n", new_node->cpu_IO[0]);
  // checks if first node

  if (flag_q == 0) { // add to ready queue for cpu bursts
    if (ready_head == NULL) {
      ready_tail = new_node;
      ready_head = new_node;
    } else {
      ready_tail->next = new_node;
      new_node->prev = ready_tail;
      ready_tail = new_node;
      // printf("===================== %s\n", sched_alg);
      // printf("ready head next %d\n", ready_head->next->cpu_IO[0]);
      
      // PR sort based on priority
      if (strcmp(sched_alg, "PR") == 0) {
        printf("PR CASE");
        Node *ptr = ready_tail;
        while (ptr->prev != NULL) {
          if (ptr->priority > ptr->prev->priority) {
            printf("swapping current: %d prev: %d", ptr->priority,
                   ptr->prev->priority);
            swap_nodes(ptr, ptr->prev);
            ptr = ptr->prev;
          } else {
            break;
          }
        }

      }
      // SJF sort based on CPU IO index 0
      else if (strcmp(sched_alg, "SJF") == 0) {
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
    
    // printf("ready head priority %d\n", ready_head->priority);
    // printf("ready head burst number %d\n", ready_head->burst_number);

    // printf("ready head CPU_io %d\n", ready_head->cpu_IO[0]);
    // printf("ready head CPU_io %d\n", ready_head->cpu_IO[1]);
    // printf("ready head CPU_io %d\n", ready_head->cpu_IO[2]);

    // printf("\nthe tails -------\n");
    // printf("tail burst %d\n", ready_tail->burst_number);
    // printf("tail prio %d\n", ready_tail->priority);
    // printf("tail cpu %d\n", ready_tail->cpu_IO[0]);

    // printf("tail ends -------\n\n");
   
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
  // new_node = NULL;
  return;
}

struct Node *de_q(int flag) {
  // READY QUEUE
  // struct Node *temp = NULL;
  struct Node *head = NULL;

  if (flag == 0) {
    head = ready_head;
  } else if (flag == 1) {
    head = io_head;
  }

  struct Node *temp = (Node *)malloc(sizeof(Node));
  temp->cpu_IO = head->cpu_IO;
  temp->burst_number = head->burst_number;
  temp->priority = head->priority;
  temp->cpu_IO = malloc(temp->burst_number * sizeof(*temp->cpu_IO));
  for (int x = 0; x < temp->burst_number; x++) {
    temp->cpu_IO[x] = head->cpu_IO[x];
  }
  temp->burst_tracker = head->burst_tracker;
  temp->start_time = head->start_time;
  temp->end_time = head->end_time;

  temp->next = NULL;
  temp->prev = NULL;

  if (flag == 0){
    if (head->next != NULL) {
      // head = ready_head;
      ready_head = ready_head->next;
      // temp = ready_head->prev;
      head->next = NULL;
      ready_head->prev = NULL;
    } else {
      ready_head = NULL;
    }
  } else if (flag == 1) {
    if (head->next != NULL) {
      // head = io_head;
      io_head = io_head->next;
      // temp = ready_head->prev;
      head->next = NULL;
      io_head->prev = NULL;
    } else {
      io_head = NULL;
    }
  }
  
  free_nodes(head);


  return temp;
}

void *read_file(void *args_param) {
  // printf("thread 1\n");
  struct arg_struct *args = (struct arg_struct *)args_param;


  char *input_file = args->arg1;
  char *alg = args->arg2;
  FILE *fp = fopen((char *)input_file, "r");
  // check if opening file was a success

  if (fp == NULL) {
    // printf("Cannot open file\n");
    pthread_exit(NULL);
  } else {
    // printf("File successfully opened\n");
  }

  char line[LINE_LEN];

  while (fgets(line, LINE_LEN, fp) != NULL) {
    // printf("%s", line);
    char *keyword = strtok(line, " ");
    // printf("keyword %s\n", keyword);
    char *priority;
    char *burst;
    char *cpu_IO;
    if ((strcmp("stop", keyword)) == 0) {
      // printf("end of input\n");
      break;
    } else if ((strcmp("sleep", keyword)) == 0) {
      // printf("need to sleep\n");
      // printf("sleep count %d\n", atoi(strtok(NULL, " ")));
      usleep(atoi(strtok(NULL, " ")) * 1000);

    } else {
      // printf("not a sleep\n");
      num_proc++;

      priority = strtok(NULL, " ");
      // printf("priority %s\n", priority);

      burst = strtok(NULL, " ");
      // printf("burst %s\n", burst);

      cpu_IO = strtok(NULL, " ");
      // printf("first cpu %d\n", atoi(cpu_IO));

      // printf("array size ======= %d\n", atoi(burst));
      // int cpu_IO_arr[atoi(burst)];
      int *cpu_IO_arr;
      cpu_IO_arr = malloc(atoi(burst) * sizeof(*cpu_IO_arr));
      int i = 0;
      while (cpu_IO != NULL) {
        // printf("CPUIO ADD %d\n", atoi(cpu_IO));
        cpu_IO_arr[i] = atoi(cpu_IO);
        cpu_IO = strtok(NULL, " ");
        i++;
      }
      // int loop;
      // for (loop = 0; loop < atoi(burst); loop++) {
      // printf("%d\n", cpu_IO_arr[loop]);
      // }
      pthread_mutex_lock(&CPU);
      struct timeval temp_timer;
      gettimeofday(&temp_timer, NULL);
      en_q(0, atoi(priority), atoi(burst), cpu_IO_arr, alg, 0, temp_timer);
      
      // printf("++++++++++++++++++++\n");
      pthread_mutex_unlock(&CPU);
      sem_post(&mutex);
      // send signal to cpu thread to begin
    }
  }
  fclose(fp);
  // pthread_exit(NULL);
  return NULL;
}

void *cpu_thread(void *args_param) {
  printf("thread 2\n");
  int completed = 0;
  struct arg_struct *args = (struct arg_struct *)args_param;
  // printf("schedule algo%s\n", args->arg2);

  char *alg = args->arg2;

  // int count = 1000000;
  struct Node *dq = NULL;
  sem_wait(&mutex);
  while (1) {

    if (ready_head == NULL && completed < num_proc){
      // printf("the io queue head %d\n", io_head->priority);
      // printf("WAITING FOR IO\n");
      sem_wait(&cpu_mutex);
      // printf("DONE WAITING FOR IO\n");
    }
    
    if (ready_head != NULL) {
      // printf("NEWLY ACQUIRED NODE %d\n", ready_head->priority);
      pthread_mutex_lock(&CPU);
      dq = de_q(0);
      pthread_mutex_unlock(&CPU);
   
      int cnt = 0;// count number of bursts in process
      while (cnt < dq->burst_number) {
        
        if (dq->cpu_IO[cnt] != -1) {

          // printf("inside the count if condition\n");
          struct timeval start_burst;
          struct timeval end_burst;
          // printf("sleeping for %d process %d index %d\n", dq->cpu_IO[cnt], dq->priority, cnt);
          // printf("sleeping for %d\n", dq->burst_number);
          gettimeofday(&start_burst, NULL);
          usleep(dq->cpu_IO[cnt] * 1000);
          gettimeofday(&end_burst, NULL);
          dq->burst_tracker = 
              dq->burst_tracker +
              (((end_burst.tv_usec - start_burst.tv_usec) + ((end_burst.tv_sec - start_burst.tv_sec) * 1000000)) / 1000);
          // printf("burst tracker ======== %d\n", dq->burst_tracker);
          dq->cpu_IO[cnt] = -1;
          break;
        }
        cnt++;
      }

      // if the process is completed
      if (cnt == dq->burst_number - 1) {
        gettimeofday(&dq->end_time, NULL);
        turnaround_time =
            turnaround_time +
            (((dq->end_time.tv_usec - dq->start_time.tv_usec) + ((dq->end_time.tv_sec - dq->start_time.tv_sec) * 1000000))/ 1000);
        // printf("start time %ld\n", dq->start_time.tv_sec);
        // printf("turnaround time *** %d\n", turnaround_time);
        // printf("CPU PROCESS %d IS COMPLETE\n", dq->priority);
        
        completed++;
        // printf("CPU PROCESS CPU COMPLETE %d\n",completed);
        if (completed == num_proc){
          // printf("ALL CPU PROCESSES DONE\n");
          // free_nodes(dq);
          free_nodes(dq);
          dq = NULL;
          break;
        }
      } else {
        // enqueueing into the I0 queue
        // pthread_mutex_lock(&IO);
        int temp_prio = dq->priority;
        int temp_bn = dq->burst_number;
        int *temp_cpu = malloc(temp_bn * sizeof(*temp_cpu));
        
        for (int x = 0; x < temp_bn; x++) {
          temp_cpu[x] = dq->cpu_IO[x];
        }
        int temp_bt = dq->burst_tracker;

        pthread_mutex_lock(&IO);
        // en_q(1, dq->priority, dq->burst_number, dq->cpu_IO, alg, dq->burst_tracker); 
        // printf("Putting something in the IO Q %d\n", dq->priority);
        struct timeval temp_start_time = dq->start_time;
        en_q(1, temp_prio, temp_bn, temp_cpu, alg, temp_bt, temp_start_time);
        
        pthread_mutex_unlock(&IO);
        
        sem_post(&io_mutex);
       
       
      }
      
      free_nodes(dq);
      dq = NULL;
    }

    // if (ready_head == NULL && dq == NULL ) {
    //   printf("ALL NULLS FOUND\n");
    //   // free_nodes(dq);
    //   break;
    // }
  }
  // pthread_exit(NULL);
  return NULL;
}

void *io_thread(void *args_param) {
  // printf("thread 3\n");
  int completed = 0;
  struct arg_struct *args = (struct arg_struct *)args_param;
  char *alg = args->arg2;
  struct Node *dq = NULL;
  if (io_head == NULL){
    // printf("STUCK WAITING ^^^^^^^^^^^^^^^\n");
    sem_wait(&io_mutex);
  }
  
  while (1){
    // printf("in while 1 **************\n");
    if (io_head == NULL){
      // printf("STUCK WAITING ##################\n");
      sem_wait(&io_mutex);
    }
    // printf("WHAT'S IN THE IO Q ?%d\n", io_head->priority);
    if (io_head != NULL) {
      // printf("in the io while loop\n");
      
      pthread_mutex_lock(&IO);
      dq = de_q(1);
      pthread_mutex_unlock(&IO);
  
      int cnt = 0;
      while (cnt < dq->burst_number) {
        if (dq->cpu_IO[cnt] != -1) {
          // printf("IO sleeping for %d process %d index %d\n", dq->cpu_IO[cnt], dq->priority, cnt);
          // pthread_mutex_lock(&IO);
          dq->cpu_IO[cnt] = -1;
          // pthread_mutex_unlock(&IO);
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
      int temp_bt = dq->burst_tracker;
          
      pthread_mutex_lock(&CPU);
      // printf("BACK IN THE CPU Q %d\n", dq->priority);
      en_q(0, temp_prio, temp_bn, temp_cpu, alg, temp_bt, temp_start_time);
      pthread_mutex_unlock(&CPU);
      if (ready_head == NULL){
        sem_post(&cpu_mutex);
      }
      if (cnt == dq->burst_number - 2) {
        completed++;
        // printf("NUMBER OF DONE PROCESSES IO %d\n", completed);
        // printf("NUMBER of processes %d\n", num_proc);
        if (completed == num_proc){
          printf("ALL IO PROCESSES DONE\n");
          // free_nodes(dq);
          free_nodes(dq);
          dq = NULL;
          break;
        }
      }
      free_nodes(dq);
      dq = NULL;
    }

  }
  // pthread_exit(NULL);
  return NULL;
}

int main(int argc, char *argv[]) {

  // int integer_ms;
  char *input_file;

  char *sched_algo;
  sched_algo = argv[2];

  // check for 6 args for RR
  if (strcmp(sched_algo, "RR") == 0) {
    if (argc != 7) {
      printf("wrong number of arguments entered\n");
      exit(1);
    }
    // integer_ms = atoi(argv[4]);
    input_file = argv[6];
  } else {
    if (argc != 5) {
      printf("wrong number of arguments entered\n");
      exit(1);
    }
    input_file = argv[4];
  }

  arg_struct args;
  args.arg1 = input_file;
  args.arg2 = sched_algo;

  if (strcmp(sched_algo, "FCFS") == 0) {
    printf("FCFS\n");
  }

  if (strcmp(sched_algo, "SJF") == 0) {
    printf("SJF\n");
  }

  if (strcmp(sched_algo, "PR") == 0) {
    printf("PR\n");
  }

  if (strcmp(sched_algo, "RR") == 0) {
    if (strcmp(argv[4], "0") == 0) {
      printf("Error: The quantum value give is zero");
      exit(1);
    }
    printf("RR\n");
  }

  if (strcmp(argv[1], "-alg") == 0) {
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
    sem_destroy(&mutex);
    sem_destroy(&io_mutex);
    sem_destroy(&cpu_mutex);
  }
  

  free_nodes(ready_head);
  free_nodes(io_head);

  printf("Input File Name                  : %s\n", input_file);
  printf("CPU Scheduling Alg               : %s", sched_algo);
  // if algorithm is RR, also print out the quantum
  if (strcmp(sched_algo, "RR") == 0) {
    printf(" (%d)\n", 0);
  } else {
    printf("\n");
  }
  printf("Throughput                       : %d ms\n", 0);
  printf("Avg. Turnaround Time             : %d ms\n", turnaround_time / num_proc);
  printf("Avg. Waiting Time in Ready Queue : %d ms\n", 0);

  return 0;
}
