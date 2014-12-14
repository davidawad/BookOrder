#include	<errno.h>
#include	<malloc.h>
#include	<pthread.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/syscall.h>


typedef struct order{
	char *bookTitle;
	double price;
	char *category;
	double remain;
	int custID;
	char processed;
} order;

typedef struct orderNode{
	order *order;
	struct orderNode *next;
} orderNode;

typedef struct category{
	char* name;
	orderNode* list;
	pthread_mutex_t mutex;
}category;

typedef struct customer{
	char *name;
	int custID;
	double credit;
	char *address;
	char *state;
	char *zip;
} customer;

typedef struct custNode{
	customer *customer;
	orderNode* mariam;
	orderNode* david;
	struct custNode *next;
}custNode;







