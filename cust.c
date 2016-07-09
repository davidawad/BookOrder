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


/**
    Reads a database text file and creates
	a linked list of custNode structs

    @param  FILE *fp to the database file
    @return custNode* of database
*/
custNode *readDatabase(FILE *fp){ //reads the database file for the customers
	int    size;
	size_t fSize;
	char   *buff;
    char   *tok;
	int    error = 1;
	char   delim[3] = "|\"";
	delim[2] = '\0';
	custNode *result = NULL ;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fSize = size;
	fseek(fp , 0 , SEEK_SET) ; //get size of file, reset fseek
	buff = malloc( (sizeof(char)*size)+1);
	while( error > 0 ){
		error = getline(&buff, &fSize, fp);
		if(error < 0){
			break;
		}
		customer *temp = (customer *)malloc(sizeof(customer));
		tok = strtok(buff , delim);
		temp->name = strdup(tok);
		tok = strtok(NULL, delim);
		temp->custID = strtol(tok, NULL , 0);
		tok = strtok(NULL, delim);
		sscanf(tok,"%lf",&temp->credit);
		tok = strtok(NULL, delim);
		temp->address = strdup(tok);
		tok = strtok(NULL, delim);
		temp->state = strdup(tok);
		tok = strtok(NULL, delim);
		temp->zip = strdup(tok);
		result = attachCustNode(result,temp);
	}
	return result;
}

custNode *attachCustNode(custNode *head, customer *target){
	if(!target || !head){
		//printf("\nTarget or Head is NULL!!!\n");
	}
	custNode *temp = malloc(sizeof(custNode));
	temp->customer = target;
	temp->mariam = NULL;
	temp->david = NULL;
	temp->next = NULL;
	custNode *search = head;

	// No matter what head is temp is now the head.
	if(search == NULL){
		temp->next = head;
		return temp;
	}
	while(search->next != NULL){
		search = search->next;
	}
	search->next = temp;
	return head;
}


void printCustomers(custNode *list){
	puts("\n");
	if(!list){
		printf("Customer list empty.\n");
		return;
	}
	int len = 0;
	custNode *curr;
	curr = list;
	customer *temp;
	while(curr){
		temp = curr->customer;
		len++;
		printf("Name:%s ID# %d has credit $%lf. Lives at %s, %s, %s.\n",temp->name,temp->custID,temp->credit,temp->address,temp->state,temp->zip);
		if(curr->next){
			if(curr->next->customer){
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

void freeCustListNode(custNode* theNode){
	free(theNode->customer->name);
	free(theNode->customer->address);
	free(theNode->customer->state);
	free(theNode->customer->zip);
	free(theNode->customer);
	free(theNode);
}
/*
void freeCustomer(){
	custNode* curr = customerList;
	while(customerList != NULL){
		customerList = customerList->next;
		freeAllOrderNodes(curr->mariam);
		freeAllOrderNodes(curr->david);
		freeCustListNode(curr);
		curr = customerList;
	}
}
*/
