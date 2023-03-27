#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// ./cpu -alg FCFS -input input.txt
// ./cpu -alg RR -quantum 10 -input input.txt
// ./cpu -alg RR -quantum 30 -input input.txt

#define LINE_LEN 256

pthread_mutex_t zero = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t one = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t two = PTHREAD_MUTEX_INITIALIZER;

// doubly linked list data structure
typedef struct Node {
    int proc_id;
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

void en_q(int flag_q, int proc_id, int priority, int burst, char * cpu_IO){
    // creates new Node
    struct Node* new_node =(Node*)malloc(sizeof(Node));
  
    // add data into the new node
    new_node->proc_id = proc_id;
    new_node->priority = priority;
    new_node->burst_number = burst;
    new_node->cpu_IO = cpu_IO;
    new_node->next = NULL;
    new_node->prev = NULL;
  
  // checks if first node
  
    // into the ready queue
    if (flag_q == 0){
        if (ready_head == NULL){
            ready_head = new_node;
            ready_tail = new_node;
        }
        else{
            ready_tail->next = new_node;
            new_node->prev = ready_tail;
            ready_tail = new_node;
        } 
    }

    // into the IO queue
    else if (flag_q == 1){
        if (io_head == NULL){
            io_head = new_node;
            io_tail = new_node;
        }
        else{
            io_tail->next = new_node;
            new_node->prev = io_tail;
            io_tail = new_node;
        }
    }
    else{
        printf("Error: Not in queue ");
    }

    return;
}

void de_q(Node* head, Node* tail){
  
  // dequeue the head 
    if (head != NULL){
  	    dq = head;
        head = head->next;
        head->prev = NULL;
        // free(dq)
    }
    return;
}
  
void *read_file(void *input_file){
    printf("inside read file\n");
    FILE *fp = fopen((char *)input_file, "r");
    // check if opening file was a success
    if (fp == NULL) {
        printf("Cannot open file\n");
        pthread_exit(NULL);
    } 
    else {
        printf("File successfully opened\n");
    }
  
    char line[LINE_LEN];
  
    while (fgets(line, LINE_LEN, fp) != NULL) {
        printf("%s", line);
        char subtext[10];
        strncpy(subtext, &line[0], 5);
        char sleep_count[10];
        if ((strcmp("sleep", subtext)) == 0) {
            printf("need to sleep\n");
            strncpy(sleep_count, &line[6], 8);
            printf("%d", atoi(sleep_count));
            usleep(atoi(sleep_count) * 1000); // usleep is in micro, * 1000 to get miliseconds
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
  
    en_q(0, 10, 20, 30, NULL);
  
    printf("hello print\n");
    printf("%d\n", ready_head->priority);
    //free(ready_head);
    //free(ready_tail);

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

