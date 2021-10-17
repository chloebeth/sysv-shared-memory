#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/times.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h> 
#include <sys/sem.h>

#define FILE_SIZE 11
#define NO_PROC   10
#define KEY 0x0000

int DelayCount = 0;
int readerID = 0;
int writerID = 0;
char* shared_buffer;
const int LOOP_TIME = 3;

union semun {
    int val; 
    struct semid_ds *buf;
    unsigned short *array;
};

struct sembuf p = { 0, -1, SEM_UNDO}; // wait
struct sembuf v = { 0, +1, SEM_UNDO};

int id; 


/*-------------------------------------------   
                The reader 
--------------------------------------------*/
void reader(){  
    int i,j;  
    char results[FILE_SIZE];  
    printf("Reader %d (pid = %d) arrives\n", readerID, getpid());   
    
    for (i=0; i<LOOP_TIME; i++)  {    
        semop(id, &p, 1);

        printf("   Reader %d starts round %d\n", readerID,i);    
        
        /* read data from shared data */     
        for (j=0; j<FILE_SIZE; j++) {       
            results[j] = shared_buffer[j];       
            sleep(2);     
        }     
            
        /* display result */     
        results[j] = 0;     
        printf("Reader %d (round %d) gets results : %s\n", readerID, i, results);  

        semop(id, &v, 1);
        sleep(2);
    }
}
        
/*-------------------------------------------   
            The writer. It tries to fill the buffer repeatly with the same digit  
--------------------------------------------*/
void writer(){  
    int i,j;  
    char data[FILE_SIZE];
  
    for (j=0; j<FILE_SIZE-1; j++) {      
        data[j]= writerID + '0';  
    }  
    
    data[j]= 0;  
    printf("Writer %d (pid = %d) arrives, writing %s to buffer\n", writerID, getpid(), data);  
    
    for (i=0; i<LOOP_TIME; i++)  {     
        semop(id, &p, 1);

        printf("   Write %d starts round %d\n", writerID,i);    
        
        /* write to shared buffer */   
        for (j=0; j<FILE_SIZE-1; j++) {   
            shared_buffer[j]= data[j];       
            sleep(1);  
        }    
        
        printf("Writer %d (round %d) finishes\n", writerID, i); 
        semop(id, &v, 1);
        sleep(2);
    }
}

/*-------------------------------------------
Routines for creating readers and writers*
-------------------------------------------*/
void create_reader(){    
    if (0 == fork()) {        
        reader();        
        exit(0);    
    }    
    
    readerID++;
}

void create_writer(){    
    if (0 == fork()) {       
        writer();        
        exit(0);   
    }    
    
    writerID++;
}

/*------------------------------------------- 
--------------------------------------------*/
void main() {   
    int return_value;  
    char InitData[]="0000000000\n";
    int i;  
    int fd;  
    
    id = semget(KEY, 1, 0666 | IPC_CREAT);
    union semun u;
    u.val = 1;

    semctl(id, 0, SETVAL, u);
    /*-------------------------------------------------------
    The following code segment creates a memory region shared by all child processes
    -------------------------------------------------------*/   
    
    fd = open("race.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);  
    if ( fd < 0 ) {     
        perror("race.dat ");     
        exit(1);  
    }  
    
    write(fd, InitData, FILE_SIZE);  
    unlink("race.dat"); // delete the file. Since the file is already opened,                      
    
    // it will not been deleted until the process terminates.  
    shared_buffer = mmap(0, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    
    if ( shared_buffer == (caddr_t) -1) {     
        perror("mmap");     
        exit(2);  
    }  
    
    /*------------------------------------------------------- 
    Create some readers and writes (processes)  
    -------------------------------------------------------*/   
    
    create_writer();  
    sleep(1); 
    create_writer();  
    sleep(1);  
    create_reader();  

    /*-------------------------------------------------------  
    Wait until all children terminate  
    --------------------------------------------------------*/  
    for (i=0; i<(readerID+writerID); i++) {      
        wait(NULL);  
    }
}
