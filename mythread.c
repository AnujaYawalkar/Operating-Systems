#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "futex.h"
#include "mythread.h"
#include "myqueue.h"
#include <sys/syscall.h>

#define CLONE_VM        0x00000100      /* set if VM shared between processes */
#define CLONE_FS        0x00000200      /* set if fs info shared between processes */
#define CLONE_FILES     0x00000400      /* set if open files shared between processes */
#define CLONE_SIGHAND   0x00000800      /* set if signal handlers shared */

int setup = 0;

struct wrapper_args
{
	void *(*function_name) (void *);
	void *function_arguments;
	mythread_t *tcb;
};


void * idle_thread_func(void *a)
{
	int x;
	struct mythread_t * idle_thread;
	idle_thread=(struct mythread_t *) malloc(sizeof(mythread_t));
	while(1)	//Idle thread always keeps yielding to other threads
	{ 
	    printf("Dei about to yield!\n"); 
		x=mythread_yield(); 
	}
}

int wrapper_func(void * arg)
{
	printf("Reached rapper yo!\n");

	struct wrapper_args *my_wrapper_args;
	my_wrapper_args = (struct wrapper_args*) arg; //casting void * received to wrapper type

	void * (*user_function)(void *) = *(my_wrapper_args->function_name);
	(*(user_function))(((void *)my_wrapper_args->function_arguments)); //function call to user start thread function

	mythread_exit(0);
	return 0;
}


void mythread_exit(void *retval)
{
	node * current = info->head;
	mythread_t thread_to_exit = current->thread;

	//self state = 2;
	thread_to_exit->state=2;

	//mythread_t thread_to_exit = mythread_self();
	//printf("In exit thread function...ID is %d\n",((int)(thread_to_exit->tid)));
	//node * current = (node *)findInQueue(thread_to_exit->tid);

	//evy1 with state==-1 in joinQ is to be made ready
	mythread_queue_t * deque = &(thread_to_exit->joinq);
	while(deque != NULL)
	{
		mythread_t dequeThread = ((mythread_t)(deque->thread));
		if(dequeThread->state==-1) dequeThread->state=0;	//in join queue, all suspended are made ready prior to exit
		deque=deque->right;
	}
	node * next = (node *)nextInQueue(current);
	if(next==NULL)
	{
		printf("Next guy null, no more\n");
	}
	else
	{
		//delete self from queue
		deleteNode(current);
	}	

	//next guy's futex up
	mythread_t next_thread = next->thread;
	futex_up(&(next_thread->futex));
	
	//self.futex down
	futex_down(&(thread_to_exit->futex));
}

mythread_t mythread_self(void)
{
	mythread_t currThread;
	pid_t thread_id;
	thread_id = (pid_t)syscall(SYS_gettid);

	node * currThreadNode =findInQueue(thread_id);
	currThread = currThreadNode->thread;
	printf("Thread id in mythread_self is %d\n",(int)thread_id);
	return currThread;
}


int mythread_yield()
{
	
	node * current = info->head;
	mythread_t current_thread = current->thread;
	current_thread->state = 0;	//Self ready
	node * next = nextInQueue(current);
//I love my life as is period

/*	if(next==NULL)	//a deadlock has happened or sumthing
	{

	}
*/
	mythread_t next_thread = next->thread;
	next_thread->state = 1;
	//next futex up
	futex_up(&(next_thread->futex));

	//self futex down
	futex_down(&(current_thread->futex));
	return 0;
}

int mythread_join(mythread_t target_thread, void **status)
{
	if(target_thread->state == 2) //it is already finished, do nothing
	{
		return -1; //to indicate that could not join
	}
	else
	{
		mythread_t currThread = mythread_self();
		node * current = findInQueue(currThread->tid);
		mythread_queue_t * t1 = (mythread_queue_t *)malloc(sizeof(mythread_queue_t));
		t1->thread = currThread;
		t1->right = NULL;

		//set self state = -1 i.e. suspend
		currThread->state = -1;

		//put self in joinQ of target
		mythread_queue_t * temp = &(target_thread->joinq);
		if(temp == NULL)
		{
			target_thread->joinq = *t1;
		}
		else
		{
			mythread_queue_t * next = &(target_thread->joinq);
			while(next->right != NULL)
			{
				next = next->right;
			}
			next->right = t1;
		}

		//find next ready in queue, set its state = 1 and its futex up
		node * nextReady = nextInQueue(current);
		mythread_t nextReady_thread = (mythread_t)(nextReady->thread);
		nextReady_thread->state = 1;
		//futex up
		futex_up(&(nextReady_thread->futex));

		//self futex down
		futex_down(&(currThread->futex));

	}
	return 0;
}

int mythread_create(mythread_t *new_thread_ID,
		    mythread_attr_t *attr,
		    void * (*start_func)(void *),
		    void *arg)
{
	mythread_t new_thread;
	new_thread = (mythread_t) malloc(sizeof(mythread_t)); //assigning space to the thread structure
	//printf("%p\n",&new_thread );
	new_thread = *new_thread_ID; //points to incoming thread
	//printf("%p\n",&new_thread );

	//printf("new thread id = %p\n\n", &new_thread_ID);

	//new_thread->attr = attr;
	//if(new_thread==NULL) return 0;	
	//new_thread->start_func = start_func;
	//new_thread->arg = arg;

	void * stack = malloc(SIGSTKSZ); //modifications needed in case optional value has come
	
	mythread_attr_t * attr1 = (mythread_attr_t *)malloc(sizeof(mythread_attr_t));
	attr1->stacksize = SIGSTKSZ;//((int)(*(stack));
	attr1->stackbase = (void *)(stack + SIGSTKSZ - sizeof(sigset_t));
	new_thread->attr = *attr1;
	//new_thread->attr->stackbase = (void *)(stack + SIGSTKSZ - sizeof(sigset_t));
	//new_thread->joinq = NULL;
	new_thread->start_func = start_func;
	new_thread->arg = arg;
	new_thread->state = 0;
	new_thread->returnValue = NULL;
	futex_init(&(new_thread->futex),-1);

	struct wrapper_args *send_wrapper_args;
	send_wrapper_args = (struct wrapper_args *)malloc(sizeof(struct wrapper_args));
	if(send_wrapper_args==NULL) return 0;
	send_wrapper_args->function_name = start_func;
	send_wrapper_args->function_arguments = arg;
	send_wrapper_args->tcb = (mythread_t *)new_thread;
	void * wrapper_pointer = (void *)send_wrapper_args;

	if(setup == 0) //first thread -->> so we need to make a queue and insert main thread and idle thread
	{
		myqueue_init();
		//create main thread
		//Next lines are for creation of main_thread & adding to queue
		mythread_t main_thread;
		main_thread = (mythread_t) malloc(sizeof(mythread_t)); //assigning space to the thread structure
		void * main_stack = malloc(SIGSTKSZ);
		mythread_attr_t * attr_main = (mythread_attr_t *)malloc(sizeof(mythread_attr_t));
		attr_main->stacksize = SIGSTKSZ;//((int)(*(stack));
		attr_main->stackbase = (void *)(main_stack + SIGSTKSZ - sizeof(sigset_t));
		main_thread->attr = *attr_main;
		//new_thread->attr->stackbase = (void *)(stack + SIGSTKSZ - sizeof(sigset_t));
		//new_thread->joinq = NULL;
		main_thread->start_func = NULL;
		void * arg_main=(void *) malloc(sizeof(void));
		main_thread->arg = arg_main;
		main_thread->state = 1;
		main_thread->returnValue = NULL;
		futex_init(&(main_thread->futex),0);
		/*
		struct wrapper_args *send_wrapper_args_main;
		send_wrapper_args_main = (struct wrapper_args *)malloc(sizeof(struct wrapper_args));
		//if(send_wrapper_args_main==NULL) return 0;
		send_wrapper_args_main->function_name = NULL;
		void * arg_main = (void *) malloc(sizeof(void));
		send_wrapper_args_main->function_arguments = arg_main;
		send_wrapper_args_main->tcb = (mythread_t *)main_thread;
		void * wrapper_pointer_main = (void *)send_wrapper_args_main;
		*/
		node * main_thread_node = createNode();
		main_thread_node->thread = main_thread;
		main_thread-> tid = 0;//(pid_t)(clone(&wrapper_func,((char*) stack + SIGSTKSZ), CLONE_VM | CLONE_FS | CLONE_SIGHAND | CLONE_FILES , wrapper_pointer_main));
		setup=1;
		printf("Main thread created\n");

		//Next lines are for creation of idle thraad:
		mythread_t idle_thread=(mythread_t) malloc (sizeof(mythread_t));
		void * idle_stack = malloc(SIGSTKSZ);
		mythread_attr_t * attr_idle = (mythread_attr_t *)malloc(sizeof(mythread_attr_t));
		attr_idle->stacksize = SIGSTKSZ;//((int)(*(stack));
		attr_idle->stackbase = (void *)(main_stack + SIGSTKSZ - sizeof(sigset_t));
		idle_thread->attr = *attr_idle;
		//new_thread->attr->stackbase = (void *)(stack + SIGSTKSZ - sizeof(sigset_t));
		//new_thread->joinq = NULL;
		idle_thread->start_func = &idle_thread_func;
		void * arg_idle=(void *) malloc(sizeof(void));
		
		idle_thread->state = 0;
		idle_thread->returnValue = NULL;
		futex_init(&(idle_thread->futex),-1);
		
		struct wrapper_args *send_wrapper_args_idle;
		send_wrapper_args_idle = (struct wrapper_args *)malloc(sizeof(struct wrapper_args));
		//if(send_wrapper_args_idle==NULL) return 0;
		send_wrapper_args_idle->function_name = &idle_thread_func;
		//void * arg_idle = (void *) malloc(sizeof(void));
		send_wrapper_args_idle->function_arguments = arg_idle;
		send_wrapper_args_idle->tcb = (mythread_t *)idle_thread;
		void * wrapper_pointer_idle = (void *)send_wrapper_args_idle;
		
		idle_thread->arg = arg_idle;
		node * idle_thread_node = createNode();
		idle_thread_node->thread = idle_thread;
		idle_thread-> tid = (pid_t)(clone(&wrapper_func,((char*) stack + SIGSTKSZ), CLONE_VM | CLONE_FS | CLONE_SIGHAND | CLONE_FILES , wrapper_pointer_idle));
		//node * idle_thread_node = createNode();

		//setup=1;
		//create idle thread
		
	}

	new_thread-> tid = (pid_t)(clone(&wrapper_func,((char*) stack + SIGSTKSZ), CLONE_VM | CLONE_FS | CLONE_SIGHAND | CLONE_FILES , wrapper_pointer));
	long a;
	a = (long)(new_thread->tid);
	printf("tid is %ld\n", a);
}