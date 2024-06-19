// File:	thread-worker.c

// List all group member's name: Tarini Divgi, Pruthviraj Patil
// username of iLab: td508
// iLab Server: rlab1

#include "thread-worker.h"


//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE

//global auxilliary variables & macros
#define MLFQ_RESET_THRESHOLD 150
#define MAX_THREAD 200
#define PRIORITY_LEVELS 4

static int thread_num = 0;
static int scheduler_initialized = 0;
static int main_initialized = 0;

double tot_turn_time = 0;
double tot_resp_time = 0;
long tot_comp_thrd = 0;

struct sigaction sa;
struct itimerval timer;




//global key variables and data structures
ucontext_t scheduler_ctx;
ucontext_t *main_ctx;
l_list *runqueue[PRIORITY_LEVELS+1];
tcb *running_thrd = NULL;
tcb *main_thrd;
node_t *running_node;
void *worker_exit_value[MAX_THREAD];
int worker_exit_status[MAX_THREAD];
 




/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {

       // - create Thread Control Block (TCB)
       // - create and initialize the context of this worker thread
       // - allocate space of stack for this thread to run
       // after everything is set, push this thread into run queue and 
       // - make it ready for the execution.

       // YOUR CODE HERE
       

       //initialize the scheduler context and other auxilliary variables
       if(scheduler_initialized == 0){
       	if(initialize_scheduler() != 0){
       		perror("Error initializing scheduler.");
       		exit(1);
       	}
       	
       }
       
       //initialize main context
       if(main_initialized == 0){
       	getcontext(main_ctx);
       	if(main_initialized == 0){
       		save_main_ctx();
       	}	
       }
       	
       
       //initializing the parameters of the tcb of the new thread
       tcb *thread1 = malloc(sizeof(tcb));
       thread1->tid = ++thread_num;
       *thread = thread1->tid;
       
       thread1->priority = 1;
       
       thread1->status = READY;
       
       thread1->elapsed = 0;
       
       //initialize context of the new thread
       if(getcontext(&thread1->ctx) < 0){
       	perror("getcontext");
       	exit(1);
       }
       
       void *stack = malloc(STACK_SIZE);
       
       thread1->ctx.uc_link=NULL;
       thread1->ctx.uc_stack.ss_sp = stack;
       thread1->ctx.uc_stack.ss_size = STACK_SIZE;
       thread1->ctx.uc_stack.ss_flags = 0;
       
       
       //making the context of the new thread
       if(!arg){
       	makecontext(&(thread1->ctx), (void *)function, 0);
       }
       else{
       	makecontext(&(thread1->ctx), (void *)function, 1, arg);
       }
         
       
       struct timeval tv;
       gettimeofday(&tv, NULL);
       thread1->arr_time = (double)(tv.tv_usec/1000) + (double)(tv.tv_sec*1000);
       
 
       
       //convert thread to node and add to the runqueue
       #ifndef MLFQ
	// Choose PSJF
	PSJFpushtoqueue(createnode(thread1),runqueue[1]);
		
	#else 
	// Choose MLFQ
	pushtoqueue(createnode(thread1),runqueue[1]);
	#endif
	
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	
	tot_cntx_switches += 1;
	swapcontext(&(running_node->thrd->ctx), &scheduler_ctx);
	
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
	
	//set exit status of the current thread to 1
	worker_exit_status[running_thrd->tid] = 1;
	
	//set the thread exit value to the value_ptr argument
	if(value_ptr){
		worker_exit_value[running_thrd->tid] = value_ptr;
	}
	else{
		worker_exit_value[running_thrd->tid] = NULL;
	}
	
	//calculate the total response time, total turnaround time, and total completed threads thus far.
	tot_resp_time += running_thrd->resp_time;
	struct timeval tv;
        gettimeofday(&tv, NULL);
        tot_turn_time += ((double)(tv.tv_usec/1000) + (double)(tv.tv_sec*1000) -running_thrd->arr_time);
        tot_comp_thrd += 1;
	
	//free memory associated with thread
	free(running_thrd->ctx.uc_stack.ss_sp);
	free(running_node->thrd);
	free(running_node);
	
	running_thrd = NULL;
	
	//calculate metrics
	avg_turn_time = tot_turn_time/tot_comp_thrd;
	avg_resp_time = tot_resp_time/tot_comp_thrd;	
	
	//switch to scheduler context
	tot_cntx_switches += 1;
	setcontext(&scheduler_ctx);	
	
	
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	while(!worker_exit_status[thread]){
	
	}
	
	if(value_ptr){
		*value_ptr = worker_exit_value[thread];
	}
	
	
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	
	mutex->flag = 0;
	mutex->mutex_queue = malloc(sizeof(l_list));
	mutex->mutex_queue->head = NULL;
	mutex->mutex_queue->tail = NULL;
	
	return 0;
};


/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        
        //use atomic test-and-set to check for lock
        if(__atomic_test_and_set(&(mutex->flag), 1) == 1){
        	running_node->thrd->status = BLOCKED;
        	pushtoqueue(running_node, mutex->mutex_queue);
        	running_thrd = NULL;
        	worker_yield();
        }
        
        //set the lock if it is not already set
        mutex->flag = 1;
        
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	
	//release the lock
	mutex->flag = 0;
	
	//push all the blocked threads in mutex_queue to runqueue
	while(mutex->mutex_queue->head){
		node_t *n = dequeue(mutex->mutex_queue);
		pushtoqueue(n,runqueue[n->thrd->priority]);
	}
	
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init
	
	free(mutex->mutex_queue);

	return 0;
};

/* scheduler */
static void schedule() {
	
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	 //if (SCHED == PSJF)
	 //else if (sched == MLFQ)

	// YOUR CODE HERE

	// - schedule policy
	#ifndef MLFQ
		// Choose PSJF
		sched_psjf();
		
	#else 
		// Choose MLFQ
		sched_mlfq();
	#endif


}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	
	//the loop runs only when there are active threads available
	while(runqueue[1]->head || running_node){
		
		//timeval object used to get time
		struct timeval tv;
		
		//push existing running thread to the runqueue
		if(running_thrd){
		running_thrd->status = READY;
		PSJFpushtoqueue(running_node,runqueue[1]);

		}
		
		//extract the next thread to be run
		running_node = PSJFdequeue(runqueue[1]);
		running_thrd = running_node->thrd;
		
		//set the response time for the thread
		if(!running_node->thrd->resp_time){
        		gettimeofday(&tv, NULL);
        		running_node->thrd->resp_time = (((double)(tv.tv_usec/1000) + (double)(tv.tv_sec*1000)) - running_node->thrd->arr_time);
		}
		
		//reset the timer
		timer.it_value.tv_usec = QUANTUM;
		timer.it_value.tv_sec = 0;
		setitimer(ITIMER_PROF, &timer, NULL);
		
		//schedule the thread to be run
		tot_cntx_switches += 1;
		swapcontext(&scheduler_ctx, &(running_thrd->ctx));
		
	
	}
	
	
};


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	struct timeval tv1;
	double current,previous;
	gettimeofday(&tv1, NULL);
	previous = ((double)(tv1.tv_usec)/1000 + (double)(tv1.tv_sec)*1000);
	
	//the loop runs only when there are active threads available
	while(runqueue[1]->head || running_node || runqueue[2]->head || runqueue[3]->head || runqueue[4]->head ){
		
		//push currently running thread to the runqueue based on its priority
		//set the status to READY
		if(running_thrd){
		running_thrd->status = READY;
		
		pushtoqueue(running_node, runqueue[running_node->thrd->priority]);
		}
		
		
		//Rule 5 of MLFQ - Move all the threads to highest priority after MLFQ_RESET_THRESHOLD
		gettimeofday(&tv1, NULL);
		current = ((double)(tv1.tv_usec)/1000 + (double)(tv1.tv_sec)*1000);
		if((current-previous) > MLFQ_RESET_THRESHOLD){
			previous = current;
			MLFQ_reset();
		}
		
		
		//extract the next thread to be scheduled
		if(runqueue[1]->head){
			running_node = dequeue(runqueue[1]);
		}
		else if(runqueue[2]->head){
			running_node = dequeue(runqueue[2]);
		}
		else if(runqueue[3]->head){
			running_node = dequeue(runqueue[3]);
		}
		else if(runqueue[4]->head){
			running_node = dequeue(runqueue[4]);
		}
		running_thrd = running_node->thrd;
		
		
		//set the time quantum based thread priority
		timer.it_value.tv_usec = QUANTUM * running_node->thrd->priority;
		timer.it_value.tv_sec = 0;
		setitimer(ITIMER_PROF, &timer, NULL);
		
		
		//set the response time for the thread
		if(!running_thrd->resp_time){
			struct timeval tv;
        		gettimeofday(&tv, NULL);
        		running_node->thrd->resp_time = (((double)(tv.tv_usec)/1000 + (double)(tv.tv_sec)*1000) - running_node->thrd->arr_time);
		}
		
		//schedule the thread to be run
		tot_cntx_switches += 1;
		swapcontext(&scheduler_ctx, &(running_thrd->ctx));
		

	}
	
};

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
};


// Feel free to add any other functions you need

// YOUR CODE HERE

//initialize scheduler context and other auxilliary variables
int initialize_scheduler(){

	//initialize the runqueue (priority 1)
	runqueue[1] = malloc(sizeof(struct linked_list));
	runqueue[1]->head = NULL;
	runqueue[1]->tail = NULL;

		
	#ifdef MLFQ 
	// Choose MLFQ
	for(int i=1; i<PRIORITY_LEVELS+1; i++){
		runqueue[i] = malloc(sizeof(struct linked_list));
		runqueue[i]->head = NULL;
		runqueue[i]->tail = NULL;
	
	}
	#endif

	
	//allocate main_ctx
	main_ctx = malloc(sizeof(ucontext_t));
	
	
	//register the signal
	sa.sa_handler = &timer_interrupt_handle;
	sigaction (SIGPROF, &sa, NULL);
	
	
	
	//create the scheduler context
	if(getcontext(&scheduler_ctx) < 0){
       	perror("getcontext_scheduler");
       	exit(1);
       }
       
       void *stack = malloc(STACK_SIZE);
       
       //set to scheduler context later
       scheduler_ctx.uc_link=NULL;
       scheduler_ctx.uc_stack.ss_sp = stack;
       scheduler_ctx.uc_stack.ss_size = STACK_SIZE;
       scheduler_ctx.uc_stack.ss_flags = 0;
       
 
       makecontext(&scheduler_ctx, (void *)&schedule, 0);
      
       
      	//intializting worker_exit_value array
      	for(int i =0;i< MAX_THREAD;i++){
      		worker_exit_value[i] = NULL;
      		worker_exit_status[i] = 0;
      	}
	
	
	scheduler_initialized = 1;
	
	return 0;
	
};


//create a node for each thread in the queue
node_t * createnode(tcb *tcb_block){
	node_t *n1 = malloc(sizeof(node_t));
	n1->thrd = tcb_block;
	n1->next = NULL;
	
	return n1;
}


//signal handler for timer interrupt
void timer_interrupt_handle(){
	
	#ifndef MLFQ
	running_thrd->elapsed += 1;
	
	#else
	if(running_thrd->priority < 4){
		running_thrd->priority += 1;
	}
	#endif
	
	tot_cntx_switches += 1;
	swapcontext(&(running_thrd->ctx), &scheduler_ctx);
	
	
};

//save context of main thread
void save_main_ctx(){
	
	//allocate space to main thread
	main_thrd = malloc(sizeof(tcb));
	main_thrd->tid = ++thread_num;
       
       main_thrd->priority = 1;
       
       main_thrd->status = READY;
       
       main_thrd->elapsed = 0;
       
       //initialize main context
       main_thrd->ctx = (ucontext_t) *main_ctx; 
       
       struct timeval tv;
       gettimeofday(&tv, NULL);
       main_thrd->arr_time = (double)(tv.tv_usec/1000) + (double)(tv.tv_sec*1000);
       
       //convert thread to node and add at the tail of runqueue
       pushtoqueue(createnode(main_thrd),runqueue[1]);
       
       main_initialized = 1;
       
       tot_cntx_switches += 1;
       setcontext(&scheduler_ctx);
};

//FIFO - push to queue (RR, MLFQ)
void pushtoqueue(node_t *n1, l_list *queue){
	if(!queue->head){
		queue->head = n1;	
	}
	else{
		queue->tail->next= n1;
	}
	n1->next = NULL;
	queue->tail = n1;
	
};

//FIFO dequeue
node_t * dequeue(l_list *queue){
	node_t *n1 = queue->head;
	queue->head = queue->head->next;
	n1->thrd->status = SCHEDULED;
	if(!queue->head){
		queue->tail = NULL;
	}
	return n1;
}

//Push to priority queue - for psjf
void PSJFpushtoqueue(node_t *n1, l_list *queue){
	if(!queue->head){
		queue->head = n1;
		n1->next = NULL;
		queue->tail = n1;	
	}
	else{
		node_t *curr = queue->head;
		node_t *prev = NULL;
		while(curr){
			if(curr->thrd->elapsed > n1->thrd->elapsed){
				if(!prev){
					queue->head = n1;
					n1->next = curr;
					return;
				}
				else{
					n1->next = curr;
					prev->next = n1;
					return;
				}
			}
			
			prev = curr;
			curr = curr->next;
		}
		prev->next = n1;
		n1->next = NULL;
		queue->tail = n1;
	}	
};

//dequeue from priority queue - for psjf
node_t * PSJFdequeue(l_list *queue){
	node_t *n1 = queue->head;
	queue->head = queue->head->next;
	n1->thrd->status = SCHEDULED;
	if(!queue->head){
		queue->tail = NULL;
	}
	return n1;
}

//shift threads from all runqueues to top priority runqueue
void MLFQ_reset()
{
	node_t *n1;
	while(runqueue[2]->head){
		n1 = dequeue(runqueue[2]);
		n1->thrd->priority = 1;
		pushtoqueue(n1,runqueue[1]);
	}
	while(runqueue[3]->head){
		n1 = dequeue(runqueue[3]);
		n1->thrd->priority = 1;
		pushtoqueue(n1,runqueue[1]);
	}
	while(runqueue[4]->head){
		n1 = dequeue(runqueue[4]);
		n1->thrd->priority = 1;
		pushtoqueue(n1,runqueue[1]);
	}
	
}

