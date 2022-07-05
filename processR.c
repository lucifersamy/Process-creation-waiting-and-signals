#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h> 
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>


float calculateVariance(int* arr){
	int i=0;
	float sum=0,avr;

    for(i=0;i<10;++i){
    	sum += arr[i];
    }
    avr = sum/10;
    sum = 0;
    for(i=0;i<10;++i){
    	sum += pow((arr[i]-avr),2);
    }

    return (sum/10);
}



float calculateCovariance(int* arr1, int* arr2){
	float sum1=0, sum2=0;
	int i=0;
	float avr1, avr2;
	for(i=0;i<10;++i){
    	sum1 += arr1[i];
    	sum2 += arr2[i];
    }
    avr1 = sum1/10;
    avr2 = sum2/10;
    sum1=0;
    int sum = 0;
    for(i=0;i<10;++i){
    	sum += (arr1[i]-avr1)*(arr2[i]-avr2);
    }
    return (sum/10);
}	
void my_handler_children(int sig){
    exit(EXIT_FAILURE);
}
extern char** environ;

int main(int argc,char *argv[]){
	struct sigaction newact_children;
    newact_children.sa_handler = &my_handler_children;
    newact_children.sa_flags = 0;

    if((sigemptyset(&newact_children.sa_mask) == -1) || (sigaction(SIGINT, &newact_children, NULL) == -1)){
        perror("Failled to install SIGINT signal handler");
        exit(EXIT_FAILURE);
    }

    char* coordinates = getenv ("COORDINATES_SEMA");
    if(coordinates == NULL)
    {
    	fprintf(stderr, "there is no match in environment\n" );
    	exit(EXIT_FAILURE);
    }
    int childNum = atoi(getenv ("CHILDNUM_SEMA"));

    printf("Created R_%d with %s\n",childNum+1, coordinates);

    //printf("%s",argv[4]);
    int coordinatesINT[30];
    sscanf(coordinates,"(%d,%d,%d),(%d,%d,%d),(%d,%d,%d),(%d,%d,%d),(%d,%d,%d),(%d,%d,%d),(%d,%d,%d),(%d,%d,%d),(%d,%d,%d),(%d,%d,%d),",
    	&coordinatesINT[0],&coordinatesINT[1],&coordinatesINT[2],&coordinatesINT[3],&coordinatesINT[4],&coordinatesINT[5],&coordinatesINT[6],&coordinatesINT[7],
    	&coordinatesINT[8],&coordinatesINT[9],&coordinatesINT[10],&coordinatesINT[11],&coordinatesINT[12],&coordinatesINT[13],&coordinatesINT[14],&coordinatesINT[15],
    	&coordinatesINT[16],&coordinatesINT[17],&coordinatesINT[18],&coordinatesINT[19],&coordinatesINT[20],&coordinatesINT[21],&coordinatesINT[22],&coordinatesINT[23],
    	&coordinatesINT[24],&coordinatesINT[25],&coordinatesINT[26],&coordinatesINT[27],&coordinatesINT[28],&coordinatesINT[29]);
   	

    int X[10], Y[10], Z[10];
    int i=0, a=0;
    for(i=0;i<28;i=i+3){
    	X[a] = coordinatesINT[i];
    	Y[a] = coordinatesINT[i+1];
    	Z[a] = coordinatesINT[i+2];
    	a++;
    }

	char newBuf[2000];


    sprintf(newBuf, "%f %f %f\n%f %f %f\n%f %f %f\n\n",  calculateVariance(X), calculateCovariance(X,Y), calculateCovariance(X,Z),
    													calculateCovariance(X,Y), calculateVariance(Y), calculateCovariance(Y,Z),
    													calculateCovariance(X,Z), calculateCovariance(Y,Z), calculateVariance(Z));


    //printf("%s", newBuf);
    struct flock lock;

    int outputFd = atoi(getenv ("FILED_SEMA"));


	//printf ("locking\n");
	/* Initialize the flock structure. */
	memset (&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	/* Place a write lock on the file. */
	if(fcntl(outputFd, F_SETLKW, &lock) == -1){
        perror("fcntl error");
        exit(EXIT_FAILURE); 
	}

	int bytesWritten, bytesRead;
	bytesRead = strlen(newBuf);
	
	if(lseek (outputFd, 0, SEEK_END)==-1){
        perror("lseek error");
        exit(EXIT_FAILURE);
    }

	while(bytesRead > 0){
        while( (bytesWritten = write(outputFd, newBuf, bytesRead)) == -1 && ((errno == EINTR)))
        if(bytesWritten < 0){
            perror("Write error");
            exit(EXIT_FAILURE);
        }

        bytesRead -= bytesWritten;
       //sonBuf += bytesWritten;
    }

	//printf ("UNLOCK");
	/* Release the lock. */
	lock.l_type = F_UNLCK;
	if(fcntl (outputFd, F_SETLKW, &lock) == -1){
		perror("fcntl error");
        exit(EXIT_FAILURE); 
	}
    exit(EXIT_SUCCESS); 
}