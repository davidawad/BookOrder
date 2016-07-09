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

#include    "bookOrder.h"


int finished = 0;
custNode *customerList;
category *categoryList;


int main(int argc, char **argv){
	if( argc != 4 ){
		printf( "Must specify a Database File, book order file, and a category names. \n Feel Free to refer to the readme for usage.\n" );
		_exit( 1 );	// crash and burn
	}
	FILE *fp;
    double totalProfit;

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
	fseek(fp, 0 , SEEK_SET);
	if(fSize == 0){
		printf("The database file is empty, cannot procceed \n");
		_exit(1);
	}
	customerList = readDatabase(fp);
	if( (error = fclose( fp ) ) > 0 ){
		printf("fclose died in %s line %d: %s \n", __FILE__ , __LINE__ , strerror(error));
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
		printf( "pthread_create() sparked the flames of revolution in %s line %d: %s\n", __FILE__ , __LINE__ , strerror( error ) );
		_exit( 1 );	// crash and burn
	}
	createThreads();
	/*create the consumer threads to wait for the producer,
	  force a join on the producer after we create the consumers
	  that way all threads are guaranteed to finish.
	 */
	if ((error = pthread_join(tid, NULL))) {
		printf( "pthread_join()ed the dark side in %s line %d: %s \n", __FILE__, __LINE__, strerror( error ) );
		_exit(1);	// crash and burn
	}

	if( (error = fclose(fp) ) > 0 ) {
		printf("fclose was beaten down by the mob in %s line %d: %s \n", __FILE__ , __LINE__ , strerror( error ) );
		_exit(1); // crash and burn
	}
	finalReport();
	free(categoryList);
	printf("Total Revenue Gained: GREEN$ANSI_RESET%.2lf\n", totalProfit);
	return 0; //success
}
