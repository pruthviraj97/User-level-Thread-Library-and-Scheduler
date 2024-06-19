// File:	worker_t.h

// List all group member's name: Tarini Divgi, Pruthviraj Patil
// username of iLab: pp865
// iLab Server: rlab1

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

// YOUR CODE HERE
#define READY 0
#define SCHEDULED 1
#define BLOCKED 2

#define STACK_SIZE SIGSTKSZ

#define QUANTUM 10000

typedef uint worker_t;



typedef struct TCB {
	/* add important states in a thread control block */
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...

	// YOUR CODE HERE
	worker_t tid;
	ucontext_t ctx;
	int status;
	int priority;
	int elapsed;
	double arr_time;
	double resp_time;

	
} tcb; 

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

typedef struct node {
	tcb *thrd;
	struct node *next;
} node_t;

typedef struct linked_list{
	node_t *head;
	node_t *tail;
}l_list;

/* mutex struct definition */
typedef struct worker_mutex_t {
	// YOUR CODE HERE
	int flag;
	l_list *mutex_queue;
} worker_mutex_t;


/* Function Declarations: */

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);

//extra additions

void pushtoqueue(node_t *n1, l_list *queue);
node_t * dequeue(l_list *queue);
node_t * createnode(tcb *tcb_block);
void timer_interrupt_handle();
int initialize_scheduler();
void save_main_ctx();
node_t * PSJFdequeue(l_list *queue);
void PSJFpushtoqueue(node_t *n1, l_list *queue);
static void sched_psjf();
static void sched_mlfq();
void MLFQ_reset();

/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#endif

#endif
