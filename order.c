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



void *readBookOrders(void *p){ //reads the book order file and sets up our queue
	printf("Thread \x1b[36m%lu\x1b[0m is the producer \n", syscall(SYS_gettid));
	FILE   *fp = (FILE *)p;
	int    size;
	size_t fSize;
	char   *buff;
    char   *tok;
	int    error = 1;
	char   delim[3] = "|\"";
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fSize = size;
	fseek(fp , 0 , SEEK_SET) ; //get size of file, reset fseek
	buff = malloc((sizeof(char) * size) + 1);
	while( error > 0 ){
		error = getline(&buff, &fSize, fp);
		if( error < 0 ){
			break;
		}
		order *temp = malloc(sizeof(order));
		tok = strtok(buff , delim); //NAME
		temp->bookTitle = strdup(tok);
		tok = strtok(NULL, delim);
		sscanf(tok,"%lf",&temp->price); //PRICE
		tok = strtok(NULL, delim);
		temp->custID = strtol(tok, NULL , 0); //ID
		tok = strtok(NULL, delim);
		temp->category = strdup(tok); //CATEGORY
		temp->processed = 'F';
		insertOrderToList(temp); //attach order to full structure.
	}
	finished = 1;
	return NULL; //return result;
}

void insertOrderToList(order *target){
	if(!target){
		puts("node doesn't exist");
	}
	int curr = 0;
	while(curr < numcats){
		if(strcmp(categoryList[curr].name,target->category) == 0){
			pthread_mutex_lock(&categoryList[curr].mutex);  //now we need to write to the database
			categoryList[curr].list = attachOrderNode(categoryList[curr].list, target);
			printf("Producer thread \x1b[36m%lu\x1b[0m added a book order to %s", syscall(SYS_gettid), categoryList[curr].list->order->category);
			pthread_mutex_unlock(&categoryList[curr].mutex);
			break;
		}
		else{
			curr++;
		}
	}
	return;
}

orderNode *attachOrderNode(orderNode *head, order *target){//attach order node to end to preserve order for report
	if(!target || !head){
		//never happens.
	}
	orderNode *temp = malloc(sizeof(orderNode));
	temp->order     = target;
	temp->next      = NULL;
	orderNode *curr;
	if(!head){
		head = temp;
		return head;
	}
	curr = head;
	while(curr != NULL){
		if(curr->next){
			curr=curr->next;
		}
		else{
			break;
		}
	}
	curr->next = temp;
	return head;
}


void printOrders(orderNode *list){
	if(!list){
		printf("Order list empty\n");
		return;
	}
	int len = 0;
	orderNode *curr;
	curr = list;
	order *temp;
	while(curr){
		temp = curr->order;
		len++;
		printf("%s ordered for $%lf in the %s category for customer #%d. Processed:[%c] The order left $%lf in the account. \n",temp->bookTitle,temp->price,temp->category,temp->custID,temp->processed, temp->remain);
		if(curr->next){
			if(curr->next->order){
				curr = curr->next;
			}
			else{
				printf("next orderNode contains empty order struct");
				break;
			}
		}
		else{
			break;
		}
	}
	printf("reached end of list, list contained %d nodes \n \n", len);
	return;
}

void freeAllOrderNodes(orderNode* theNode){
	orderNode* temp;
	while(theNode){
		temp = theNode;
		theNode = theNode->next;
		freeOrderNode(temp);
	}
}

void freeOrderNode(orderNode* theNode){
	free(theNode->order->bookTitle);
	free(theNode->order->category);
	free(theNode->order);
	free(theNode);
}

