// Assigment #1 - Real Time Operating Systems
// Steven Palma Morera . S4882385
// 2019-11-16


// Earlier Deadline First Algorithm
// The following script shows an example of how to schedule threads
// with the Earlier Deadline First algorithm for periodic tasks

// REFERENCES
// http://disi.unitn.it/~palopoli/courses/RTOS/use_sched_deadline.pdf
// https://elinux.org/images/d/de/Lelli.pdf
// https://www.kernel.org/doc/Documentation/scheduler/sched-deadline.txt
// http://retis.sssup.it/luca/TuToR/sched_dl-presentation.pdf
// https://stackoverflow.com/questions/42016035/how-use-sched-deadline-in-linux
// http://www.admin-magazine.com/Archive/2015/25/Optimizing-utilization-with-the-EDF-scheduler
// https://github.com/jlelli/sched-deadline-tests/blob/master/basic/periodic_yield.c
// https://www.youtube.com/watch?v=TDR-rgWopgM&feature=youtu.be&t=2187

// NOTES
// I tried the code using the standard nanosleep and next_arrival_time method, and with the sched_yield() function
// It seemed to me that the nanosleep() method worked better and produced a more logical output, that is, in the first run
// 1 finish first, 2 finish second and 3 finish third, and also at the end only the task 3 runs since it is the slowest in
// completing the 100 executions because of its period. But from the references that I read, It seems that the sched_yield()
// method is the right way to do it. So I decided to stay with sched_yield(), even if the output doesn't make much sense.

// Includes
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/types.h>
// Include for SCHED_DEADLINE
#include <sched.h>

//  Define the functions of each periodic task
void task1_code( );
void task2_code( );
void task3_code( );

// Define the functions for timing and synchronization of each task
void *task1( void *);
void *task2( void *);
void *task3( void *);

// New struct for SCHED_DEADLINE
// Define the functions for the new struct
int sched_setattr(pid_t pid,const struct sched_attr*attr,unsigned intflags);
int sched_getattr(pid_t pid,const struct sched_attr*attr,unsigned intsize,unsigned intflags);

// Define new schedule attributes structure
struct sched_attr {
  uint32_t size;
  uint32_t sched_policy;
  uint64_t sched_flags;
  int32_t sched_nice;
  uint32_t sched_priority;
  uint64_t sched_runtime;
  uint64_t sched_deadline;
  uint64_t sched_period;
};


// Preprocessor macros
//  Affects the computational time of each task
#define INNERLOOP 1000
#define OUTERLOOP 2000
//  Number of periodic, aperiodic and total tasks
#define NPERIODICTASKS 3
#define NAPERIODICTASKS 0 //  Only periodic tasks
#define NTASKS NPERIODICTASKS + NAPERIODICTASKS


// Global variables
long int periods[NTASKS];
long long int WCET[NTASKS] = {0,0,0};
pthread_t thread_id[NTASKS];
struct sched_attr attr[NTASKS];
// struct timespec next_arrival_time[NTASKS]; No longer needed with sched_yield

// Main
main() {

  // Set period of each task in nanoseconds
  periods[0]= 200000000;
  periods[1]= 400000000;
  periods[2]= 800000000;

  // Assign a maximum and minimum priority in the system
  struct sched_param priomax;
  priomax.sched_priority = sched_get_priority_max(SCHED_FIFO);
  struct sched_param priomin;
  priomin.sched_priority=sched_get_priority_min(SCHED_FIFO);

  // Set the maximum priority to the current thread temporarily
  if (getuid()==0){ // Checks if It is running as a superuser
    pthread_setschedparam(pthread_self(),SCHED_FIFO,&priomax);
  }
  else{
    return -1; // If not, the example ends as an error
  }

  // Compute the Worst Case Execution Time of each task
 	int i;
 	int j;
 	long long int WCET_actual[NTASKS];

  // Run each task 10 times and save the highest WCET of each task
  struct timespec time_1, time_2;
 	for (j=0; j<10; j++){
  	for (i =0; i < NTASKS; i++){
			clock_gettime(CLOCK_REALTIME, &time_1);
      if (i==0){
        task1_code();
      }
      else if (i==1){
        task2_code();
      }
      else{
        task3_code();
      }
      clock_gettime(CLOCK_REALTIME, &time_2);
      WCET_actual[i]= 1000000000*(time_2.tv_sec - time_1.tv_sec)+(time_2.tv_nsec-time_1.tv_nsec);
      if (WCET_actual[i]>WCET[i]){
        WCET[i]=WCET_actual[i];
      }
    }
  }

  // Show results for WCET of each task
	for (i=0; i<NTASKS; i++){
		printf("\nWorst Case Execution Time %d=%lld \n", i, WCET[i]);
    fflush(stdout);
	}

  // Compute U
  double U=0.0;
  for (i=0; i<NTASKS; i++){
    U= U+ WCET[i]/double(periods[i]);
  }

  // Compute Ulub
  //double Ulub = NPERIODICTASKS*(pow(2.0,(1.0/NPERIODICTASKS)) -1);
  double Ulub=1; // Since there are an harmonic relationship between the periods of each task

  // Check sufficient conditions for schedulablility
  if (U>Ulub){
    printf("\n U=%lf Ulub=%lf Non schedulable Task Set \n", U, Ulub);
    fflush(stdout);
    return(-1); // If it is not, the example ends as an error
  }
  else{
    printf("\n U=%lf Ulub=%lf Scheduable Task Set \n", U, Ulub);
  	fflush(stdout);
  }

  // Delay excecution for visualitization purposes
  sleep(5);

  // Set minimum priority to the current thread
  pthread_setschedparam(pthread_self(),SCHED_FIFO,&priomin);

  // Set the attributes of each task, including scheduling policy, period and WCET
  for (i=0; i<NPERIODICTASKS; i++){

    attr[i].size = sizeof(attr[i]);
    attr[i].sched_policy = SCHED_DEADLINE;
    attr[i].sched_runtime = WCET[i];
    attr[i].sched_period = periods[i];
    attr[i].sched_deadline = attr[i].sched_period;
  }

  // Declare a variable to store the return values of pthread_create
  int iret[NTASKS];

	clock_gettime(CLOCK_REALTIME, &time_1);

  // Set the next arrival time for each task - No longer needed with sched_yield

  // Create all threads
  iret[0] = pthread_create( &(thread_id[0]), NULL, task1, NULL);
  iret[1] = pthread_create( &(thread_id[1]), NULL, task2, NULL);
  iret[2] = pthread_create( &(thread_id[2]), NULL, task3, NULL);

  // Assign the attributes of each task to each thread just created
  for (i=0; i<NTASKS; i++){
    sched_setattr((thread_id[i]),&(attr[i]),0);
  }

  // Join all threads
  pthread_join( thread_id[0], NULL);
  pthread_join( thread_id[1], NULL);
  pthread_join( thread_id[2], NULL);

  return (0);
}


// Specific application of the tasks
void task1_code() {

  // Print the ID of the current tasks and shows when it starts
  printf(" 1[ "); fflush(stdout);

  // Random application for computational time
	int i,j;
	double k;
  for (i = 0; i < OUTERLOOP; i++){
    for (j = 0; j < INNERLOOP; j++){
			k = rand()*rand()%10;
    }
  }

  // Shows when it ends
  printf(" ]1 "); fflush(stdout);
}

void task2_code() {

  // Print the ID of the current tasks and shows when it starts
  printf(" 2[ "); fflush(stdout);

  // Random application for computational time
	int i,j;
	double k;
  for (i = 0; i < OUTERLOOP; i++){
    for (j = 0; j < INNERLOOP; j++){
			k = rand()*rand()%10;
    }
  }

  // Shows when it ends
  printf(" ]2 "); fflush(stdout);
}

void task3_code() {

  // Print the ID of the current tasks and shows when it starts
  printf(" 3[ "); fflush(stdout);

  // Random application for computational time
	int i,j;
	double k;
  for (i = 0; i < OUTERLOOP; i++){
    for (j = 0; j < INNERLOOP; j++){
			k = rand()*rand()%10;
    }
  }

  // Shows when it ends
  printf(" ]3 "); fflush(stdout);
}


// Thread code for synchronization
void *task1( void *ptr) {

  // Set thread affinity - No longer needed with SCHED_DEADLINE

  // Run the task 100 times
  int i=0;
  for (i=0; i<100; i++){
    task1_code();
    sched_yield();

    // Sleeps until the end of the period - No longer needed with sched_yield

    // Calculate the next deadline - No longer needed with sched_yield

  }
}

void *task2( void *ptr) {

  // Set thread affinity - No longer needed with SCHED_DEADLINE

  // Run the task 100 times
  int i=0;
  for (i=0; i<100; i++){
    task2_code();
    sched_yield();

    // Sleeps until the end of the period - No longer needed with sched_yield

    // Calculate the next deadline - No longer needed with sched_yield

  }
}

void *task3( void *ptr) {

  // Set thread affinity - No longer needed with SCHED_DEADLINE

  // Run the task 100 times
  int i=0;
  for (i=0; i<100; i++){
    task3_code();
    sched_yield();

    // Sleeps until the end of the period - No longer needed with sched_yield

    // Calculate the next deadline - No longer needed with sched_yield

  }
}


// Set and Get attributes for the new structure
int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags){
	return syscall(314, pid, attr, flags);
}

int sched_getattr(pid_t pid, const struct sched_attr *attr, unsigned int size, unsigned int flags){
 return syscall(315, pid, attr, size, flags);
}
