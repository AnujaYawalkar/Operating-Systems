#include "myqueue.h"
#include "mythread.h"
#include <stdio.h>
#include <stdlib.h>

//initialise the queue
void myqueue_init()
{
	info = (struct myqueue_info*)malloc(sizeof(struct myqueue_info));	
	info->myqueue_size = 0;
	info->head = info->tail = NULL;
}

/*Function to return head of queue*/
node* get_myqueue_head()
{	return info->head;	}

/*Function to return tail of queue*/
node* get_myqueue_tail()
{	return info->tail;	}

/*Function to return size of queue*/
int get_myqueue_size()
{	return info->myqueue_size;	}

//creating a new queue element for the queue and inserting it in position according to FIFO i.e. at end
node* createNode()
{
	node *my_node;
	my_node = (node*)malloc(sizeof(node));
	my_node->right = NULL;
	my_node->left = NULL;
	my_node->thread = (mythread_t)malloc(sizeof(mythread_t));
	(info -> myqueue_size)++;
	int x = info -> myqueue_size;
	my_node -> position = x;	

	//now adding in queue
	if(info->head==NULL) //if queue is empty
	{
		info->head = my_node;
		info->tail = my_node;
	}
	else
	{
		my_node->left = info->tail;
		info->tail->right = my_node;
		info->tail = my_node;
		my_node->right = NULL;
	} //if queue is not empty, insert at end

	return my_node;
}

void displayPosition(node * element)
{
	if(element == NULL) printf("Null element\n");
	else printf("position is %d\n", element->position);
}

void traverseQueue()
{
	node *n = info->head;
	while(n != NULL)
	{
		printf("Position = %d\n", n->position);
		n = n->right;
	}
}

void deleteNode(node * element)
{
	if(element == NULL)
		printf("Element is NULL...Cannot be deleted! \n");

	else //if element is first node
	{
		if(info->head == element)
		{
			if(element->right == NULL)
			{
				info->head = NULL;
				info->tail = NULL;
			} //if element is the only node
			else
			{
				info->head = element->right;
				node * a = info->head;
				a->left = NULL;
			} //if there are other nodes after element
			(info->myqueue_size)--;
			free(element);
		}
	

		else if(info->tail == element) //element is the last node
		{
			node * left = element->left;
			left->right = NULL;
			info->tail = left;
			(info->myqueue_size)--;
			free(element);
		}

		else //element is middle node in queue
		{
			node * left = element->left;
			node * right = element->right;
			left->right = right;
			right->left = left;
			(info->myqueue_size)--;
			free(element);
		}
	}
}//end of delete function

node * findInQueue(pid_t thread_id)
{
	node * n = info -> head;
	while(n!=NULL)
	{
		mythread_t t1 = n->thread;
		if(t1->tid == thread_id)
		{
			printf("Found thread in queue\n");
			return n;
		}
		n = n->right;
	}

	printf("Not found in queue\n");
	return NULL;
}//findInQueue ends here

node * nextInQueue(node * element) //finding node next in queue with state not suspended
{
	node * next = element->right;
	//if(next!=NULL)
	//{

	//}
	while(next!=NULL)
	{
		node * tail = info->tail;
		node * head = info->head;
		tail->right=element;
		element->right=NULL;
		element->left=tail;
		info->tail=element;
		next->left=NULL;
		info->head=next;

		mythread_t nextThread = (next->thread);
		if(nextThread->state==0)
			return next;
		next=next->right;
	}
	return element;
	/*
	while(next!=NULL)
	{
		if(next->state==0)
			return next;
		next=next->right;
	}
	return element;
	*/
/*
	if(next!=NULL)
	{
		while(next->state==0)
			{
				next->state=0;
				return 
			}
	}
*/	
}//end function nextInQueue

