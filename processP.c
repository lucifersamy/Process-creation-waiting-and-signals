#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/stat.h>


int signalArrived=0;
int childNum=0;

char *outPath;
pid_t allChildren[400];

void my_handler(int sig)
{
    signalArrived++;
}
float calculateFrobeniusNorm(float matrix[3][3]){
    float sum=0;
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            sum += pow(matrix[i][j],2);
        }
    }

    return sqrt(sum);
}
extern char** environ;

int main(int argc,char *argv[]){
    int opt;
    char inputFilePath[40];
    char outputFilePath[40];
    int inputFd;

    while((opt = getopt(argc, argv, ":i:o:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':
                strcpy(inputFilePath,optarg);
                break;
            case 'o':
                strcpy(outputFilePath,optarg);
                break;
            case ':':  
                errno=EINVAL;
                fprintf(stderr, "Wrong format.\n" );
                fprintf(stderr, "Usage: %s -i inputPath -o outputPath", argv[0]);
                exit(EXIT_FAILURE);     
                break;  
            case '?':  
                errno=EINVAL;
                fprintf(stderr, "Wrong format.\n" );
                fprintf(stderr, "Usage: %s -i inputPath -o outputPath", argv[0]);
                exit(EXIT_FAILURE); 
                break; 
            case -1:
                break;
            default:
                abort (); 
        }
    }

    if(optind!=5){
        errno=EINVAL;
        fprintf(stderr, "Wrong format.\n" );
        fprintf(stderr, "Usage: %s -i inputPath -o outputPath", argv[0]);
        exit(EXIT_FAILURE); 
    }


    struct sigaction newact;
    newact.sa_handler = &my_handler;
    newact.sa_flags = 0;

    if((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT, &newact, NULL) == -1)){
        perror("Failled to install SIGINT signal handler");
        exit(EXIT_FAILURE);
    }

    outPath = argv[4];
    /* opens the input file */
    inputFd=open(inputFilePath, O_RDONLY );
    if(inputFd<0){
        perror("Open file error");
        exit(EXIT_FAILURE); 
    }

    char buffer[30];
    int bytesRead;


    int outputFd;
    outputFd = open (argv[4], O_RDWR);
    if(outputFd<0){
        perror("Open file error");
        exit(EXIT_FAILURE); 
    }

    if(signalArrived>0){
        for (int i = 0; i < childNum; ++i)
        {
            if(kill(allChildren[i], SIGINT) == -1){
                perror("kill error");
                exit(EXIT_FAILURE); 
            }

        }
        if (close(outputFd) < 0){ 
            perror("Close error"); 
            exit(EXIT_FAILURE); 
        } 

        if (close(inputFd) < 0){ 
            perror("Close error"); 
            exit(EXIT_FAILURE); 
        } 
        remove(outPath);
        exit(EXIT_SUCCESS);
    }


    char str[10000];
    sprintf(str, "%d", outputFd);
    if (setenv("FILED_SEMA", str , 1) == -1){
        perror("setenv");
        exit(EXIT_FAILURE);
    }

    int status;          
    pid_t childPid;

    int condition;
    
    int i, j,x=0;
    
    printf("Process P reading %s\n", argv[2]);

    for (;;) {
        if(signalArrived>0){
            for (int i = 0; i < childNum; ++i)
            {
                if(kill(allChildren[i], SIGINT) == -1){
                    perror("kill error");
                    exit(EXIT_FAILURE); 
                }

            }
            if (close(outputFd) < 0){ 
                perror("Close error"); 
                exit(EXIT_FAILURE); 
            } 

            if (close(inputFd) < 0){ 
                perror("Close error"); 
                exit(EXIT_FAILURE); 
            } 
            remove(outPath);
            exit(EXIT_SUCCESS);
        }

        int coordinatesINT[30];
        char coordBefore[5000] = "";
        char coordinates[30000] = "";

        while( ((bytesRead = read(inputFd, buffer, 30)) == -1) && (errno == EINTR))
        if(bytesRead<0){
            perror("Read error");
            exit(EXIT_FAILURE);
        }
        if(bytesRead<30){
            break;
        }

        for(i=0; i<30; ++i){
            coordinatesINT[i] = buffer[i];
           // x++;
        }

        for(i = 0; i < 30; i += 3)
        {
            if(i==27)
                sprintf(coordBefore, "(%d,%d,%d)", (coordinatesINT[i]), (coordinatesINT[i+1]),(coordinatesINT[i+2]));
            else
                sprintf(coordBefore, "(%d,%d,%d),", (coordinatesINT[i]), (coordinatesINT[i+1]),(coordinatesINT[i+2]));
            strcat(coordinates, coordBefore);
        }
        if (setenv("COORDINATES_SEMA", coordinates, 1) == -1){
            perror("setenv");
            exit(EXIT_FAILURE);
        }
        if(signalArrived>0){
            for (int i = 0; i < childNum; ++i)
            {
                if(kill(allChildren[i], SIGINT) == -1){
                    perror("kill error");
                    exit(EXIT_FAILURE); 
                }

            }
            if (close(outputFd) < 0){ 
                perror("Read error"); 
                exit(EXIT_FAILURE); 
            } 

            if (close(inputFd) < 0){ 
                perror("Read error"); 
                exit(EXIT_FAILURE); 
            } 
            remove(outPath);
            exit(EXIT_SUCCESS);
        }
        condition = fork();

        if(condition== -1){
            perror("Fork error");
            exit(EXIT_FAILURE);
        } 
        else if(condition == 0){ /* Child*/
            char str2[10000];
            sprintf(str2, "%d", childNum);
            if (setenv("CHILDNUM_SEMA", str2 , 1) == -1){
                perror("setenv");
                exit(EXIT_FAILURE);
            }
            sleep(0.8);
            execve("./processR", argv, environ);
        }
        else{ /* Parent */ 
            if(signalArrived>0){
                for (int i = 0; i < childNum; ++i)
                {
                    if(kill(allChildren[i], SIGINT) == -1){
                        perror("kill error");
                        exit(EXIT_FAILURE); 
                    }

                }
                if (close(outputFd) < 0){ 
                    perror("Read error"); 
                    exit(EXIT_FAILURE); 
                } 

                if (close(inputFd) < 0){ 
                    perror("Read error"); 
                    exit(EXIT_FAILURE); 
                } 
                remove(outPath);
                exit(EXIT_SUCCESS);
            }
            //sleep(1);
            allChildren[childNum] = condition;
            childNum++;
        }
        
    }
    for (;;) { /* Parent waits for each child to exit */
        childPid = wait(NULL);

        if (childPid == -1) {
            if (errno == ECHILD) 
            {
                break;
            } 
            else 
            { 
                perror("Fork error");
                exit(EXIT_FAILURE); 
            }
        }
        else{
            if(signalArrived>0){
                for (int i = 0; i < childNum; ++i)
                {
                    if(kill(allChildren[i], SIGINT) == -1){
                        perror("kill error");
                        exit(EXIT_FAILURE); 
                    }

                }
                if (close(outputFd) < 0){ 
                    perror("Read error"); 
                    exit(EXIT_FAILURE); 
                } 

                if (close(inputFd) < 0){ 
                    perror("Read error"); 
                    exit(EXIT_FAILURE); 
                } 
                remove(outPath);
                exit(EXIT_SUCCESS);
            }
        }
    }

    if(childNum < 2){
        fprintf(stderr, "Insufficient input to compare.\n" );   
        exit(EXIT_SUCCESS);
    }
    printf("Reached EOF, collecting outputs from %s", argv[4]);
    
    if(lseek (outputFd, 0, SEEK_SET)==-1){
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    char buffer2[1000000];
    int bytesRead2;

    while( (bytesRead2 = read(outputFd, buffer2, 1000000) == -1) && (errno == EINTR))
    if(bytesRead2 < 0){
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    if(signalArrived>0){
        if (close(outputFd) < 0){ 
            perror("Read error"); 
            exit(EXIT_FAILURE); 
        } 

        if (close(inputFd) < 0){ 
            perror("Read error"); 
            exit(EXIT_FAILURE); 
        } 
        remove(outPath);
        exit(EXIT_SUCCESS);
    }
    
    float matrix[3][3];
    int t=0;
    float results[childNum];
    int n;
    char* data = buffer2;
    for(t=0;t<childNum; ++t){
        if(signalArrived>0){
            if (close(outputFd) < 0){ 
                perror("Read error"); 
                exit(EXIT_FAILURE); 
            } 

            if (close(inputFd) < 0){ 
                perror("Read error"); 
                exit(EXIT_FAILURE); 
            } 
            remove(outPath);
            exit(EXIT_SUCCESS);
        }
        sscanf(data,"%f %f %f\n%f %f %f\n%f %f %f\n%n", &matrix[0][0],  &matrix[0][1], &matrix[0][2],
                                                            &matrix[1][0], &matrix[1][1], &matrix[1][2],
                                                              &matrix[2][0], &matrix[2][1], &matrix[2][2] , &n);
        results[t] = calculateFrobeniusNorm(matrix); 
        data+=n;
        //printf("%f ",results[t]);
    }
    int k=0;
    float value;
    float minDist;

    value=results[0] - results[1];
    if(value<0)
        value = -value;
    minDist = value;

    int minMatrix1=0, minMatrix2=1;

    for(t=0; t<childNum; ++t){
        for(k=t+1; k<childNum; ++k){
            value = results[t] - results[k];
            if(value<0)
                value = -value;
            if(value < minDist){
                minDist = value;
                minMatrix1=k;
                minMatrix2=t;
            }
        }
    } 
    if(signalArrived>0){
        if (close(outputFd) < 0){ 
            perror("Read error"); 
            exit(EXIT_FAILURE); 
        } 

        if (close(inputFd) < 0){ 
            perror("Read error"); 
            exit(EXIT_FAILURE); 
        } 
        remove(outPath);
        exit(EXIT_SUCCESS);
    }  
    //printf("\n\n---%d %d----\n\n",minMatrix1,minMatrix2);
    printf("\nThe closest 2 matrices are ");
    
    char* data2 = buffer2;
    int flag=0;
    for(t=0;t<childNum; ++t){
        if(signalArrived>0){
            if (close(outputFd) < 0){ 
                perror("Read error"); 
                exit(EXIT_FAILURE); 
            } 

            if (close(inputFd) < 0){ 
                perror("Read error"); 
                exit(EXIT_FAILURE); 
            } 
            remove(outPath);
            exit(EXIT_SUCCESS);
        }
        sscanf(data2,"%f %f %f\n%f %f %f\n%f %f %f\n%n", &matrix[0][0],  &matrix[0][1], &matrix[0][2],
                                                            &matrix[1][0], &matrix[1][1], &matrix[1][2],
                                                              &matrix[2][0], &matrix[2][1], &matrix[2][2] , &n);
        
        if(t == minMatrix1 || t == minMatrix2){
            printf("[");
            for(int i=0; i<3; ++i){
                for(int j=0; j<3; ++j){
                    printf("%f ", matrix[i][j]);
                }
                if( i != 2)
                    printf("/ ");
            }
            printf("]");
            if(!flag){
                printf(" and ");
                flag=1;
            }
        }

        data2+=n;
    }

    printf(", and their distance is %f\n", minDist);

    if (close(outputFd) < 0){ 
        perror("Read error"); 
        exit(EXIT_FAILURE); 
    } 

    if (close(inputFd) < 0){ 
        perror("Read error"); 
        exit(EXIT_FAILURE); 
    } 
}

