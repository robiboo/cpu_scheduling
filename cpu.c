#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// ./cpu -alg FCFS -input input.txt
// ./cpu -alg RR -quantum 10 -input input.txt
// ./cpu -alg RR -quantum 30 -input input.txt

#define LINE_LEN 256

pthread_mutex_t zero = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t one = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t two = PTHREAD_MUTEX_INITIALIZER;

// doubly linked list data structure
typedef struct Node {
  int data;
  int priority;
  int burst_number;
  char * cpu_IO;
 
  struct Node* next;
  struct Node* prev;
} Node;

struct Node* ready_head;
struct Node* ready_tail;

struct Node* io_head;
struct Node* io_tail;

struct Node* dq;

static void en_q(Node* head,Node* tail, int data, int burst){
  struct Node* new_node = (Node*)malloc(sizeof());
  new_node->data = data;
  new_node->burst = burst;
  new_data->next = NULL;
  new_data->prev = NULL;
  
  if (head == NULL){
    head = new_node;
    tail = new_node;
  }
  else{
    tail->next = new_node;
    new_node-> = tail;
    tail = new_node;
  } 
  return;
}

static void de_q(Node* head, Node* tail){
  if (head != NULL){
  	dq = head;
    head = head->next;
    head->prev = NULL;
    // free(dq)
  return;
}

static void sort_q(Node* head, Node* tail){
  return 0;
}
  
static void *read_file(void *input_file){
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
  
  while (fgets(line, LINE_LEN, fp) != NULL){
    printf("%s", line);
//     if the thread encounters sleep, then thread sleeps before continuing to read file
		if (line[0] == "sleep") {
      printf("need to sleep");
    }
  }
    
  fclose(fp);
  pthread_exit(NULL);
}


int main(int argc, char *argv[]){

  // int integer_ms;
  char *input_file;

  char *sched_algo;
  sched_algo = argv[2];
  

  // check for 6 args for RR
  if (strcmp(sched_algo, "RR") == 0){
    if (argc != 7) {
      printf("wrong number of arguments entered\n");
      exit(1);
    }
    // integer_ms = atoi(argv[4]);
    input_file = argv[6];
  }
  else{
    if (argc != 5) {
      printf("wrong number of arguments entered\n");
      exit(1);
    }
    input_file = argv[4];
  }
  
  if (strcmp(argv[1], "-alg") == 0){
    printf("i am running\n");
    pthread_t thread_input;
    
    pthread_create(&thread_input, NULL, read_file, (void *)input_file);
    pthread_join(thread_input, NULL);
    
    if (strcmp(sched_algo, "FCFS") == 0){
      return 0;
    }

    if (strcmp(sched_algo, "SJF") == 0){
      return 0;
    }

    if (strcmp(sched_algo, "PR") == 0){
      return 0;
    }

    if (strcmp(sched_algo, "RR") == 0){
      if (strcmp(argv[4], "0") == 0){
        printf("Error: The quantum value give is zero");
        exit(1);
      }
      return 0;
    }
  }
  
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
