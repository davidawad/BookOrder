/**
    bookOrder.h
    Purpose: Main Header file for BookOrder System

    @author  David Awad
    @version 1.2 7/9/2016
*/

#ifndef BOOK_ORDER
#define BOOK_ORDER

#define ANSI_RESET     "\x1b[0m"
#define GREEN          "\x1b[32m"

/* struct definitions */
typedef struct order{
    int    custID;
    char   processed;
    char   *category;
    char   *bookTitle;
    double price;
    double remain;
} order;

typedef struct orderNode{
	order  *order;
	struct orderNode *next;
} orderNode;

typedef struct category{
	char      *name;
	orderNode *list;
	pthread_mutex_t mutex;
} category;

typedef struct customer{
	int    custID;
	char   *address;
	char   *state;
	char   *zip;
	char   *name;
	double credit;
} customer;

typedef struct custNode{
	customer        *customer;
	orderNode       *mariam;
	orderNode       *david;
	struct custNode *next;
} custNode;

/* external shared variables */
extern int numcats;
extern int finished;
extern int numcats;
extern int error;
extern custNode *customerList;
extern category *categoryList;

/* Customer Node functions - cust.c */
custNode *readDatabase(FILE*);
custNode *attachCustNode(custNode*, customer*);

void freeCustomer();
void freeCustListNode(custNode*);
void printCustomers(custNode);

/* Book Order Node Functions - order.c */
void *readBookOrders(void*);
void insertOrderToList(order*);

orderNode *attachOrderNode(orderNode*, order*);
void printOrders(orderNode*);

void freeAllOrderNodes(orderNode*);
void freeOrderNode(orderNode*);

/* Cateory Functions - category.c */
int  readCategories(FILE*);
void fillCategory(FILE*);
void processOrder(orderNode*);

/* Utility Functions - util.c */
void createThreads();
void *row_worker(void*);
int  isRowFull(int);
void finalReport();
#endif
