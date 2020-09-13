/* BLG 312E - COMPUTER OPERATING SYSTEMS HOMEWORK 3
 * Mehmet Gencay Ertürk - 150130118
 * 150130118-hw3.c
 * To Compile: gcc 150130118-hw3.c -o hw3 
 *
 * To Run: ./hw3 <input file> <output file>
 *
 * I could'nt finish this assignemnt due to the some problems with regard to semaphores that I could't solve. 
 * 
 *  The parts which I have implemented are:
 *     
 *    1- A process creates all required resources and preapres an array showing the color for each box in a shared memory location and creates the box using fork call
 *    2- The parent process removes all resources at the end. 
 *    3- The parent process reads the sequence of box colors from an input file.
 *    4- Each box process reads its color from the array implemented on a shared memory.
 *    5- The program does'nt give any output unfortunately. 
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define _GNU_SOURCE
#define KEYSHM1 1000
#define KEYSHM2 1001
#define SEMKEY 1500


struct BOX {
    char color;
    int id;
};
typedef struct BOX Box;

void sem_signal(int semid, int val){
	struct sembuf semafor;
	semafor.sem_num = 0;
	semafor.sem_op = val;
	semafor.sem_flg = 1;
	semop(semid, &semafor, 1);
}

void sem_wait(int semid, int val){
	struct sembuf semafor;
	semafor.sem_num = 0;
	semafor.sem_op = (-1*val);
	semafor.sem_flg = 1;
	semop(semid, &semafor, 1);
}


void mysignal(void){}

void mysigset(int num){
	struct sigaction mysigaction;
	mysigaction.sa_handler = (void*) mysignal;
	mysigaction.sa_flags = 0;
	sigaction(num, &mysigaction, NULL);
}


int main (int argc, char *argv[]) {
	int f = 0,
		i = 0,           //loop variable
		m = 0,           //number of processes
		index = 0,       //process index (1 to m)
		shmid1 = 0,
		shmid2 = 0;	 //shared memory id for array       
	int lock = 0;	//semafor variable
	mysigset(12);	
	
    FILE* input_file; 
	Box * boxes = NULL;
	int * children = NULL;
	char * renkler = NULL;
	
	char *input = argv[1];
	char *ouput = argv[2];
	
	if(input_file = fopen("input.txt", "r")) {
        fscanf(input_file, "%d", &m);
        fclose(input_file);
    }
    
	children =malloc(sizeof(int)*m);

	for(i=0; i<m; i++) {
		f = fork();    //create m number of processes
		index = i;     //keep the process index
		if(f == -1) {
			printf("Error in fork creation!");
			exit(1);
		}
		if(f == 0) {
			break;
		}
		children[i] = f;
	}
	/*PARENT*/
	if(f!=0) {
        
       lock = semget(SEMKEY,1,0700|IPC_CREAT);
	   semctl(lock,0,SETVAL,0);
 	  
	   shmid1 = shmget(KEYSHM1, sizeof(char)*(m), 0700|IPC_CREAT);  //create a shared memory for colours
       renkler = (char *)shmat(shmid1, 0, 0);
       
       shmid2 = shmget(KEYSHM2, sizeof(Box)*(m), 0700|IPC_CREAT);  //create a shared memory for boxes
       boxes = (Box *)shmat(shmid2, 0, 0);

           
        if(input_file = fopen(input, "r")) {
           char read;
           int i = 0;
           fscanf(input_file, "%d", &m);
           while (i < m) {
              fscanf (input_file, "%c", &read);
              if(read == '\0' || read == '\n')
                continue;
              renkler[i] = read;
              //printf("%c\n", renkler[i]);      
              i++;
      	    }
      	    //printf("%d\n", m);
            fclose (input_file);
    	}  
        else
            printf("Error in opening file!\n");
		
       /*for(i=0; i<m; i++)	//send a signal to the child processes in children array
            kill( children[i], 12);	*/
  
		
        sem_wait(lock,m);
		
		/*for(i=0; i<m; i++) {
            printf("%d %c\n", boxes[i].id, boxes[i].color);
        }*/
		
         shmdt(renkler);
         shmdt(boxes);
         shmctl(shmid1, IPC_RMID, 0);
		 shmctl(shmid2, IPC_RMID, 0); 
		 
         semctl(lock, 0, IPC_RMID, 0);     //Remove the semafor 
		
		exit(0);
    }
    /*CHILD*/
   else {
        //pause();
		lock = semget(SEMKEY,1,0);
		
	    shmid1=shmget (KEYSHM1, sizeof(char)*(m), 0);
	    renkler = (char*) shmat(shmid1,0,0);
	    
	    shmid2=shmget (KEYSHM2, sizeof(Box)*(m), 0);
	    boxes = (Box *) shmat(shmid2,0,0);
	    
        boxes[index].color = renkler[index];
        boxes[index].id = getpid();
        
        //printf("%d  %c\n",  boxes[index].id,  boxes[index].color);
       	
        shmdt(renkler);
        shmdt(boxes);
		sem_signal(lock,1);
		
		exit(0);
    }
    return 0;
}


