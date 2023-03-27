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

// doubly linked list data structure
typedef struct Node {
  // int proc_id;
  int priority;
  int burst_number;
  char *cpu_IO;

  struct Node *next;
  struct Node *prev;
} Node;

struct Node *ready_head;
struct Node *ready_tail;

struct Node *io_head;
struct Node *io_tail;

struct Node *dq;

void en_q(int flag_q, int priority, int burst, char *cpu_IO) {
  // creates new Node
  struct Node *new_node = (Node *)malloc(sizeof(Node));

  // add data into the new node
  // new_node->proc_id = proc_id;
  new_node->priority = priority;
  new_node->burst_number = burst;
  new_node->cpu_IO = cpu_IO;
  new_node->next = NULL;
  new_node->prev = NULL;

  // checks if first node

  if (flag_q == 0) {
    if (ready_head == NULL) {
      ready_head = new_node;
      ready_tail = new_node;
    } else {
      ready_tail->next = new_node;
      new_node->prev = ready_tail;
      ready_tail = new_node;
    }
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
void swap_nodes(Node *a, Node *b) {
  Node *temp = a;

  a->priority = b->priority;
  b->priority = temp->priority;

  a->burst_number = b->burst_number;
  b->burst_number = temp->burst_number;

  a->cpu_IO = b->cpu_IO;
  b->cpu_IO = temp->cpu_IO;

  free(temp);
  return;
}

void sort_q(char *sched_algo) {
  if (strcmp(sched_algo, "PR") == 0) {
  }
}

// function to free all nodes in linked list
void free_nodes(Node *head) {
  Node *temp;

  while (head != NULL) {
    temp = head;
    head = head->next;
    free(temp);
  }
}

void *read_file(void *input_file) {
  printf("inside read file\n");
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
    // char subtext[10];
    // strncpy(subtext, &line[0], 5);
    char sleep_count[10];

    char * keyword = strtok(line, " ");
    printf("keyword %s\n", keyword);
    // char burst[10];
    // char cpu_IO[10];
    if ((strcmp("sleep", keyword)) == 0) {
      printf("need to sleep\n");
      // strncpy(sleep_count, &line[6], 8);
      // printf("%d", atoi(sleep_count));
      usleep(atoi(sleep_count) *
             1000); 
    } else {
      printf("not a sleep\n");
      // strncpy(priority, line[5], 6);
      // printf("priority %s\n", priority);
      // strncpy(burst, line[7], 8);
      // printf("burst %s\n", burst);
      // en_q(0, priority, burst, cpu_IO);
    }
  }

  fclose(fp);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

  // int integer_ms;
  char *input_file;

  char *sched_algo;
  sched_algo = argv[2];

  //     flag, procid, priority, burst, IO/CPU
  en_q(0, 20, 0, NULL);

  //free(ready_head);
  //free(ready_tail);

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

  if (strcmp(argv[1], "-alg") == 0) {
    pthread_t thread_input;

    pthread_create(&thread_input, NULL, read_file, (void *)input_file);
    pthread_join(thread_input, NULL);

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

