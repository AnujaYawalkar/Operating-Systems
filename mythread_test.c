#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "mythread.h"
#include <sys/syscall.h>


void* func(void* arg)
{
	int i;
	for(i=1; i<=5; i++)
		{
			printf("i = %d\n", i);
			if(i==3)
			{
				printf("Exiting\n");
				mythread_exit(0);
			}
		}

	return NULL;
}

int main()
{
	t1 tids[5];
	int i;
	pid_t t1 = (pid_t)syscall(SYS_gettid);

	printf("%d",(int)t1);
	//mythread_create(&tids[0], NULL, func, NULL);

	for(i=0;i<5;i++)
	{
		mythread_t temp;
		temp=&tids[i];
		mythread_create(&temp, NULL, func, NULL);
		printf("Created thread %d\n", i);
	}
	for(i=0; i<5; i++)
    {
        //my_thread_join(tids[i], NULL);
    }

	printf("Back in main. Exiting program now!\n");
	return 0;
}