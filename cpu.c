#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    FILE *fp = fopen(input_file, "r");
    // check if opening file was a success
    if (fp == NULL) {
      printf("Cannot open file\n");
      exit(1);
    }  
  }

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