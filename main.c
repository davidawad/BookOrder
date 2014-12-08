/*
 *
 *  David Awad                 (ada80)
 *  Mariam Tsilosani           (mt617)
 *
 *  main.c
 *
 */
#include	<errno.h>
#include	<malloc.h>
#include	<pthread.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/syscall.h>
//  gcc -g -O -o bookOrder main.c -lpthread
//  ./bookOrder database.txt orders.txt categories.txt
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

int finished = 0;
int numcats;
double totalProfit;
custNode *customerList;
category *categoryList;

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
		//huge print statement. but useful as all.
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
custNode* attachCustNode(custNode *head, customer *target){
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
custNode *readDatabase(FILE *fp){ //reads the database file for the customers
	int size;
	size_t fSize; //getline wants size_t ... dumb.
	char *buff,*tok;
	int error = 1;
	char delim[3] = "|\"";
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
orderNode *attachOrderNode(orderNode *head, order *target){//attach order node to end to preserve order for report
	if(!target || !head){
		//never happens.
	}
	orderNode *temp = malloc(sizeof(orderNode));
	temp->order = target;
	temp->next = NULL;
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
void *readBookOrders(void *p){ //reads the book order file and sets up our queue
	printf("Thread \x1b[36m %lu \x1b[0m is the producer \n", syscall(SYS_gettid));
	FILE *fp = (FILE *)p;
	int size;
	size_t fSize; //getline wants size_t ... dumb.
	char *buff,*tok;
	int error = 1;  //SIGNED INTEGER. NOT UNSIGNED. SIGNED.
	char delim[3] = "|\"";
	delim[2] = '\0';
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
		order *temp = malloc(sizeof(order));
		tok = strtok(buff , delim);//NAME
		temp->bookTitle = strdup(tok);
		tok = strtok(NULL, delim);
		sscanf(tok,"%lf",&temp->price);//PRICE
		tok = strtok(NULL, delim);
		temp->custID = strtol(tok, NULL , 0);//ID
		tok = strtok(NULL, delim);
		temp->category = strdup(tok);//CATEGORY
		temp->processed = 'F';
		insertOrderToList(temp);//attach order to full structure.
	}
	finished = 1;
	return NULL; //return result;
}
int readCategories(FILE *fp){//reads the categories from the file.
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
void processOrder(orderNode* theNode){
	if(theNode == NULL){
		return;
	}
	custNode *curr;
	curr = customerList;
	while(curr != NULL){
		if(curr->customer->custID == theNode->order->custID){
			if(curr->customer->credit < theNode->order->price){
				printf("Consumer Thread \x1b[33m%lu\x1b[0m reported \x1b[31mFailure: \x1b[0m\n", syscall(SYS_gettid) );
				//printf("Name: %s\n",curr->customer->name);
				printf("Book name: %s\n",theNode->order->bookTitle);
				printf("Book price: $%lf\n",theNode->order->price);
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
int isRowFull(int i){ // 0 if empty , 1 if full
	if(categoryList[i].list){
		//printf("consumer sees that this row contains the node %s \n",categoryList[i].list->order->bookTitle );
		return 1;
	}
	else{
		return 0;
	}

}
void *row_worker(void *p){
	/*
	annoying fix to get a simple argument from pthread_create.
	what you are supposed to do is actually pass the address of the variable to the function.
	we then use the * operator to dereference the pointer after casting it to an int pointer.
	*/
	int cur = *((int *)p);
	printf("Consumer Thread \x1b[33m %lu \x1b[0m is responsible for %s \n", syscall(SYS_gettid), categoryList[cur].name );
	while(1){
		if( finished == 0 ){ //if the i'th row of the structure is NOT empty we need to interact with it by processing the orders.
			pthread_mutex_lock(&categoryList[cur].mutex);//now we want to interact with the structure, lock the mutex
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
			printf("Thread ID \x1b[33m%lu\x1b[0m has finished.\n", syscall(SYS_gettid));
			break;
		}
	}
	return NULL;
}
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
			printf("pthread_join() stuck its nose where it doesn't belong in %s on line %d: %s\n", __FILE__,__LINE__, strerror(error));
			_exit(1); //crash and burn
		}
	}
	return;
}
void finalReport(){
	if(!customerList){
		return;
	}
	custNode* currCust;
	currCust = customerList;
	orderNode* check;
	puts("\n");
	while(currCust){
		printf("=== BEGIN CUSTOMER INFO ===\n");
		printf("### BALANCE ###\n");
		printf("Customer name: %s\n", currCust->customer->name);
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
void freeOrderNode(orderNode* theNode){
	free(theNode->order->bookTitle);
	free(theNode->order->category);
	free(theNode->order);
	free(theNode);
}
void freeAllOrderNodes(orderNode* theNode){
	orderNode* temp;
	while(theNode){
		temp = theNode;
		theNode = theNode->next;
		freeOrderNode(temp);
	}
}
void freeCustListNode(custNode* theNode){
	free(theNode->customer->name);
	free(theNode->customer->address);
	free(theNode->customer->state);
	free(theNode->customer->zip);
	free(theNode->customer);
	free(theNode);
}

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
int main(int argc, char **argv){
	if( argc != 4 ){
		printf( "Must specify a Database File, book order file, and a category names. \n Feel Free to refer to the readme for usage.\n" );
		_exit( 1 );	// crash and burn
	}
	else if ( !argv[1] ){ //sscanf returns the number of scanned arguments throws it into gcd -> a
		printf( "Must specify Database File on the command line\n" );
		_exit( 1 );	// crash and burn
	}
	else if ( !argv[2] ){
		printf( "Must specify Book Order File on the command line\n" );
		_exit( 1 );	// crash and burn
	}
	else if ( !argv[3] ){
		printf( "Must specify Category File on the command line\n" );
		_exit( 1 );	// crash and burn
	}
	unsigned int error;
	FILE *fp;
	if( (fp = fopen(argv[1], "r") ) == NULL){
		printf("fopen didn't work in %s line %d.\n", __FILE__ , __LINE__ );
		printf("Please Enter Valid Database File \n");
		_exit(1); // crash and burn
	}
	else if( (fp = fopen(argv[2], "r") ) == NULL){
		printf("fopen didn't work in %s line %d.\n", __FILE__ , __LINE__ );
		printf("Please Enter Valid Book Order File \n");
		_exit(1); // crash and burn
	}
	else if( (fp = fopen(argv[3], "r") ) == NULL ){
		printf("fopen didn't work in %s line %d.\n", __FILE__ , __LINE__ );
		printf("Please Enter Valid Category File \n");
		_exit(1); // crash and burn
	}
	/* Done with error checking. */
	fp = fopen(argv[1], "r");
	int fSize;
	fseek(fp, 0L, SEEK_END);
	fSize = ftell(fp);
	fseek(fp , 0 , SEEK_SET);
	if(fSize == 0){
		printf("The database file is empty, cannot procceed \n");
		_exit(1);
	}
	customerList = readDatabase(fp);
	if( (error = fclose( fp ) ) > 0 ){
		printf("fclose died in %s line %d: %s \n", __FILE__ , __LINE__ , strerror( error ) );
		_exit(1); // crash and burn
	}
	totalProfit = 0;
	/*
	   this gives us the full set of orders that we can
	   then separate into the array of smaller queues based on the category.
	*/
	fp = fopen(argv[3], "r");
	fseek(fp, 0L, SEEK_END);
	fSize = ftell(fp);
	fseek(fp , 0 , SEEK_SET);
	if(fSize == 0){
		printf("The category file is empty, cannot procceed \n");
		_exit(1);
	}
	numcats = readCategories(fp);
	categoryList = (category*)malloc(sizeof(category)*numcats);
	fillCategory(fp);
	if(( error = fclose( fp ) )> 0 ){
		printf("fclose stuck it's nose where it didn't belong in %s line %d: %s\n", __FILE__ , __LINE__ , strerror( error ) );
		_exit(1); // crash and burn
	}
	fp = fopen(argv[2], "r");
	pthread_t	tid;
	if ((error = pthread_create( &tid, NULL , readBookOrders , fp ))){
		printf( "pthread_create() croaked in %s line %d: %s\n", __FILE__ , __LINE__ , strerror( error ) );
		_exit( 1 );	// crash and burn
	}
	createThreads();
	/*create the consumer threads to wait for the producer,
	force a join on the producer after we create the consumers
	that way all threads are guaranteed to finish.
	*/
	if ( (error = pthread_join( tid, NULL )) ){
		printf( "pthread_join() croaked in %s line %d: %s\n", __FILE__, __LINE__, strerror( error ) );
		_exit( 1 );	// crash and burn
	}

	if( (error = fclose( fp ) ) > 0 ){
		printf("fclose was beaten down by the mob in %s line %d: %s \n", __FILE__ , __LINE__ , strerror( error ) );
		_exit(1); // crash and burn
	}
	finalReport();
	free(categoryList);
	printf("Total Revenue Gained: \x1b[32m$\x1b[0m%.2lf\n", totalProfit);
	return 0; //success
}
