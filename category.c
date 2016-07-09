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

void fillCategory(FILE *fp){
	int fSize;
	unsigned int error;
	fseek(fp, 0L, SEEK_END);
	fSize = ftell(fp);
	fseek(fp , 0 , SEEK_SET);
	int counter = 0;
	size_t fileSize = fSize;
	while(counter < numcats){
		categoryList[counter].name = malloc( (sizeof(char)*fSize)+1);
		categoryList[counter].list = NULL;
		error = getline(&categoryList[counter].name, &fileSize, fp);
		if(error == 0 ){
			printf("failed to read file in %s on line %d :%s \n", __FILE__ , __LINE__ , strerror(error));
			_exit(1); //crash and burn, baby
		}
		counter++;
	}
}

int readCategories(FILE *fp){ // reads the categories from the file.
	int fSize;
	unsigned int error;
	int result = 0;
	char *buff;
	fseek(fp, 0L, SEEK_END);
	fSize = ftell(fp);
	fseek(fp , 0 , SEEK_SET);
	buff = malloc( (sizeof(char)*fSize)+1);
	if( (error = fscanf(fp , "%s\n", buff ) ) == 0 ){
		printf("failed to read file in %s on line %d :%s \n", __FILE__ , __LINE__ , strerror(error)  );
		_exit(1); //crash and burn, baby
	}
	result++;
	while(error != -1){
		error = fscanf(fp , "%s\n", buff );
		if(error == EOF){
			result--;
		}
		result ++;
	}
	return result;
}


void processOrder(orderNode* theNode){
	if(theNode == NULL){
		return;
	}
	custNode *curr;
	curr = customerList;
	while(curr != NULL){
		if(curr->customer->custID == theNode->order->custID){
			if(curr->customer->credit < theNode->order->price){
				printf("Consumer Thread \x1b[33m %lu \x1b[0m reported \x1b[31mFailure: \x1b[0m\n", syscall(SYS_gettid) );
				//printf("Name: %s\n",curr->customer->name);
				printf("Book name: %s\n"   , theNode->order->bookTitle);
				printf("Book price: $%lf\n", theNode->order->price);
				//printf("Balance remaining: %.2lf\n\n",curr->customer->credit);
				if(curr->david == NULL){
					curr->david = theNode;
					theNode->next = NULL;
					return;
				}
				else{
					orderNode* search = curr->david;
					while(search->next != NULL){
						search = search->next;
					}
					search->next = theNode;
					theNode->next = NULL;
					return;
				}
			}
			printf("Consumer Thread \x1b[33m%lu\x1b[0m reported \x1b[32mSuccess: \x1b[0m\n" , syscall(SYS_gettid) );
			printf("Book name: %s\n",theNode->order->bookTitle);
			printf("Book price: %.2lf\n",theNode->order->price);
			//printf("Name: %s\n", curr->customer->name);
			curr->customer->credit = curr->customer->credit - theNode->order->price;
			printf("Credit Remaining: \x1b[32m$%.2lf\x1b[0m\n", curr->customer->credit );
			totalProfit = totalProfit + theNode->order->price;
			if(curr->mariam == NULL){
				curr->mariam = theNode;
				theNode->next = NULL;
				curr->mariam->order->remain = curr->customer->credit;
				return;
			}
			else{
				orderNode* search = curr->mariam;
				while(search->next != NULL){
					search = search->next;
				}
				search->next = theNode;
				theNode->next = NULL;
				search->next->order->remain = curr->customer->credit;
				return;
			}


		}
		curr = curr->next;
	}
	return;
}
