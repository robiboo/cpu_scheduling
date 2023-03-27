#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes
// ./cpu -alg FCFS -input input.txt
// ./cpu -alg RR -quantum 10 -input input.txt
// ./cpu -alg RR -quantum 30 -input input.txt

#define LINE_LEN 256

pthread_mutex_t zero = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t one = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t two = PTHREAD_MUTEX_INITIALIZER;

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

  struct Node *next;
  struct Node *prev;
} Node;

struct Node *ready_head;
struct Node *ready_tail;

struct Node *io_head;
struct Node *io_tail;

struct Node *dq;

void swap_nodes(Node *a, Node *b) {
  // swap two nodes

  int temp_priority = a->priority;
  int temp_burst = a->burst_number;
  int *temp_cpuIO = a->cpu_IO;

  a->priority = b->priority;
  b->priority = temp_priority;

  a->burst_number = b->burst_number;
  b->burst_number = temp_burst;

  a->cpu_IO = b->cpu_IO;
  b->cpu_IO = temp_cpuIO;

  return;
}

void en_q(int flag_q, int priority, int burst, int *cpu_IO, char* sched_alg) {
  // creates new Node
  struct Node *new_node = (Node *)malloc(sizeof(Node));

  // add data into the new node
  // new_node->proc_id = proc_id;
  new_node->priority = priority;
  new_node->burst_number = burst;
  new_node->cpu_IO = cpu_IO;
  new_node->next = NULL;
  new_node->prev = NULL;

  printf("enq cpu io %d\n", new_node->cpu_IO[0]);
  // checks if first node

  if (flag_q == 0) {
    if (ready_head == NULL) {
      ready_tail = new_node;
      ready_head = new_node;
    } else {
      ready_tail->next = new_node;
      new_node->prev = ready_tail;
      ready_tail = new_node;
      printf("===================== %s\n", sched_alg);
      printf("ready head next %d\n", ready_head->next->cpu_IO[0]);

      // PR sort based on priority
      
      if (strcmp(sched_alg, "PR") == 0){
        printf("PR CASE");
        Node * ptr = ready_tail;
        while (ptr->prev != NULL){
          if (ptr->priority > ptr->prev->priority ){
            printf("swapping current: %d prev: %d", ptr->priority, ptr->prev->priority);
            swap_nodes(ptr, ptr->prev);
            ptr = ptr->prev;
          } else{
            break;
          }
        }
    
      }
      //SJF sort based on CPU IO index 0
      else if (strcmp(sched_alg, "SJF") == 0) {
        Node * ptr = ready_tail;
        while (ptr != NULL){
          if (ptr->cpu_IO[0] < ptr->prev->cpu_IO[0] ){
            swap_nodes(ptr, ptr->prev);
            ptr = ptr->prev;
          } else{
            break;
          }
        }
        
   
      }
    }
    printf("ready head io %d\n", ready_head->priority);
    printf("ready head io %d\n", ready_head->burst_number);

    printf("ready head io %d\n", ready_head->cpu_IO[0]);
    printf("ready head io %d\n", ready_head->cpu_IO[1]);
    printf("ready head io %d\n", ready_head->cpu_IO[2]);

    printf("\nthe tails -------\n");
    printf("tail burst %d\n", ready_tail->burst_number);
    printf("tail prio %d\n", ready_tail->priority);
    printf("tail cpu %d\n", ready_tail->cpu_IO[0]);

    printf("tail ends -------\n\n");
    return;

  } else if (flag_q == 1) {
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

void de_q(int flag_q) {

  Node *head = NULL;
  if (flag_q == 0) {
    head = ready_head;
  } else if (flag_q == 1) {
    head = io_head;
  } else {
    printf("error: no node to dequeue");
  }

  // dequeue the head
  if (head != NULL) {
    dq = head;
    head = head->next;
    head->prev = NULL;
    // free(dq)
  }
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

void *read_file(void *args_param) {
  struct arg_struct *args = (struct arg_struct *)args_param;
  printf("%s\n", args -> arg1);
  printf("%s\n", args -> arg2);

  char* input_file = args->arg1;
  char* alg = args->arg2;
  FILE *fp = fopen((char *)input_file, "r");
  // check if opening file was a success

  if (fp == NULL) {
    printf("Cannot open file\n");
    pthread_exit(NULL);
  } else {
    printf("File successfully opened\n");
  }

  char line[LINE_LEN];

  while (fgets(line, LINE_LEN, fp) != NULL) {
    printf("%s", line);
    char *keyword = strtok(line, " ");
    printf("keyword %s\n", keyword);
    char *priority;
    char *burst;
    char *cpu_IO;
    if ((strcmp("stop", keyword)) == 0) {
      printf("end of input\n");
      break;
    } else if ((strcmp("sleep", keyword)) == 0) {
      printf("need to sleep\n");
      // printf("sleep count %d\n", atoi(strtok(NULL, " ")));
      usleep(atoi(strtok(NULL, " ")) * 1000);
    } else {
      // printf("not a sleep\n");
      priority = strtok(NULL, " ");
      printf("priority %s\n", priority);

      burst = strtok(NULL, " ");
      printf("burst %s\n", burst);
      
      cpu_IO = strtok(NULL, " ");
      printf("first cpu %d\n", atoi(cpu_IO));

      printf("array size ======= %d\n", atoi(burst));
      // int cpu_IO_arr[atoi(burst)];
      int *cpu_IO_arr;
      cpu_IO_arr = malloc(atoi(burst)*sizeof(*cpu_IO_arr));
      int i = 0;
      while (cpu_IO != NULL) {
        // printf("CPUIO ADD %d\n", atoi(cpu_IO));
        cpu_IO_arr[i] = atoi(cpu_IO);
        cpu_IO = strtok(NULL, " ");
        i++;
      }
      int loop;
      for (loop = 0; loop < atoi(burst); loop++) {
        printf("%d\n", cpu_IO_arr[loop]);
      }
      en_q(0, atoi(priority), atoi(burst), cpu_IO_arr, alg);
      
    }
  }
  fclose(fp);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

  // int integer_ms;
  char *input_file;

  char *sched_algo;
  sched_algo = argv[2]; // need to be fixed using strncpy
  
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
    }
    input_file = argv[4];
    exit(1);
  }
  
  arg_struct args;
  args.arg1 = input_file;
  args.arg2 = "PR";
  if (strcmp(argv[1], "-alg") == 0) {
    pthread_t thread_input;

    // pthread_create(&thread_input, NULL, read_file, (void *)input_file, "ehhehfje");
    pthread_create(&thread_input, NULL, read_file, (void*) &args);
    pthread_join(thread_input, NULL);

    printf("asdsadad %d\n", ready_tail->priority);
    printf("asdsadad %d\n", ready_tail->prev->priority);

    printf("asdsdaa %d\n", ready_tail->cpu_IO[0]);
    printf("asdsdaa %d\n", ready_tail->prev->cpu_IO[0]);

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
  }

  free_nodes(ready_head);

  return 0;
  // printf("Input File Name                  : %s\n", input_file)
  // printf("CPU Scheduling Alg               : %s", sched_algo)
  // if algorithm is RR, also print out the quantum
  // if (strcmp(sched_algo, "RR") == 0) {
  //		printf(" (%s)\n", quantum)
  // } else {
  //		printf("\n")
  // }
  // printf("Throughput                       : ")
  // printf("Avg. Turnaround Time             : ")
  // printf("Avg. Waiting Time in Ready Queue : ")
}

