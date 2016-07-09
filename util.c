#if defined(__MACH__)
#include    <stdlib.h>
#else
#include    <malloc.h>
#endif

#include	<stdio.h>
#include	<string.h>
#include	<errno.h>
#include	<unistd.h>
#include	<pthread.h>
#include	<sys/syscall.h>

#include     "bookOrder.h"


void createThreads(){
	int i;
	pthread_t IDs[numcats];
	unsigned int error;
	for(i = 0; i < numcats; i++){
		pthread_t	tid;
		if ( (error = pthread_create( &tid, NULL, row_worker, &i )) ){//we make each thread responsible for the ith row of the producer
			printf( "pthread_create() croaked in %s line %d: %s\n", __FILE__ , __LINE__ , strerror( error ) );
			_exit( 1 );	// crash and burn
		}
		IDs[i] = tid;  ///create an array of thread ID's based on the number of categories.
	}
	for(i = 0; i < numcats; i++){
		if((error = pthread_join(IDs[i], NULL))){
			printf("pthread_join() stuck its nose where it doesn't belong in %s on line %d: %s\n", __FILE__, __LINE__, strerror(error));
			_exit(1); //crash and burn
		}
	}
	return;
}


void *row_worker(void *p){
	/*
	   what you are supposed to do is actually pass the address of the variable to the function.
	   we then use the * operator to dereference the pointer after casting it to an int pointer.
	 */
	int cur = *((int *)p);
	printf("Consumer Thread \x1b[33m %lu \x1b[0m is responsible for %s \n", syscall(SYS_gettid), categoryList[cur].name );
	int error;
	while(1){
		if( finished == 0 ){ //if the i'th row of the structure is NOT empty we need to interact with it by processing the orders.
			if( ( error = pthread_mutex_lock(&categoryList[cur].mutex)  )  == 0 ){ //now we want to interact with the structure, lock the mutex
				if(isRowFull(cur)){
					orderNode* temp = categoryList[cur].list;
					categoryList[cur].list = categoryList[cur].list->next;
					processOrder(temp);
					pthread_mutex_unlock(&categoryList[cur].mutex);
				}
				else{
					pthread_mutex_unlock(&categoryList[cur].mutex);
				}
			}
			else{
			//current thread suspended by mutex anyway, relinquish cpu for efficiency
				if( (error = sched_yield() ) == -1 ){
					printf("sched_yield() kept going in %s on line %d :%s \n", __FILE__ , __LINE__ , strerror(error)  );
					_exit(1); //watch it burn
				}
			}
		}
		else{
			printf("Thread ID \x1b[33m%lu\x1b[0m has finished.\n", syscall(SYS_gettid));
			break;
		}
	}
	return NULL;
}

int isRowFull(int i) {
	if(categoryList[i].list){

        #ifdef DEBUG
		    printf("consumer sees that this row contains the node %s \n",
                    categoryList[i].list->order->bookTitle );
        #endif

		return 1;
	}
	else{
		return 0;
	}
}

void finalReport() {
	if(!customerList){
		return;
	}
	custNode* currCust;
	currCust = customerList;
	orderNode* check;
	while(currCust){
		printf("=== BEGIN CUSTOMER INFO ===\n");
		printf("### BALANCE ###\n");
		printf("Customer name: %s \n", currCust->customer->name);
		printf("Customer ID number: %d\n",currCust->customer->custID);
		printf("Remaining credit balance after all purchases (a dollar amount): %.2lf\n", currCust->customer->credit);
		check = currCust->mariam;
		printf("### SUCCESSFUL ORDERS ###\n");
		while(check){
			printf("\"%s\"|%.2lf|%.2lf\n",check->order->bookTitle,check->order->price,check->order->remain);
			check = check->next;
		}
		printf("### REJECTED ORDERS ###\n");
		check = currCust->david;
		while(check){
			printf("\"%s\"|%.2lf\n",check->order->bookTitle,check->order->price);
			check = check->next;
		}
		printf("=== END CUSTOMER INFO ===\n\n\n");
		currCust = currCust->next;
	}
	return;
}
