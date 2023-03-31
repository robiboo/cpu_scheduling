# cpu_scheduling
## OS Project 2
## Team:
  - Emily Lim
  - Vivian Duong
  - Robert Panerio

### How you share data between parts of the program
  - We used global variables to share data between parts of the program such as:
       - Struct "Node" representing processes
       - The Ready and IO Queue (as struct Node)
       - Mutex locks
       - Semaphores
       - integer variables to keep track of the turnaround time, number of process, and total wait time
     
### Your approach to synchronization issues
  - Threads will run at the same time, but since the CPU and the IP processing threads need processes to be in the queue before they begin, we use semaphores to force a thread to wait. Once the input thread has
put something in the cpu queue, then it will sempost and the cpu thread can start. The same goes for the IO thread receiving signals from the CPU thread.

### How you switch between scheduling algorithms
  - We passed various parameters, including information of the process and scheduling algorithm used, into the en_q function. Within the function, there are if statements to check which algorithm is being used, which will determine how the process node gets sorted into the ready queue.
  - There is also a check for the RR algorithm in the cpu_thread function, so that the first node chosen sleeps for only up to the quantum amount.

### How you generate data for the required measurements
  - Throughput: total processes / total runtime. To get runtime, we get the time at the top of main and the time when all threads have stopped.
  - Average turnaround time: turnaround times of all processes / number of processes. Turnaround times were calculated by getting the time where each thread was first enqueued from the input file and subtracting that from the time where the last cpu burst had completed for that same thread.
  - Average waiting time in ready queue: total waiting time / number of processes. Waiting times were calculate by getting the time where a process has been enqueud into the CPU queue and subtracting that from the time right before the process is dequeud to "run". This time was added to a total waiting time variable. 

### The purposes of any threads beyond the three required ones
  - No threads are created/used beyond the required minimum number of threads.