#include<stdio.h>
#include<sys/ipc.h>
#include<sys/mman.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<unistd.h> // for unix
#include<fcntl.h>
#include<sys/shm.h>


int main() { 
    char *data;
    key_t key;
    int flag = 0;
    char output[108];
    char input[100];
    int id;

    int size = 1024;

    char send[8] = "06872342";
    char fill[8] = "00000000";

    while(1) {
        flag = 0;
        // create the key 
        key = ftok("/tmp/shmem.dat", 2);

        // attached shared memory
        id = shmget(key, size, 0666 | IPC_CREAT);
        data = (char *)shmat(id, (void *)0, 0);

        memmove(input, data, size);

        for (int i = 0; i < 8; i++) {
            if (input[i] == send[i]) {
                flag = 1;
            } else if (input[i] != send[i]) {
                flag = 0;
                break;
            }
        }
    
        while (flag != 0) {
            if (flag == 1) {
                printf("Client Sent: %s", input+8);
                flag = 0;
            }

            // overwrite
            for (int i = 0; i < 8; i++) {
                output[i] = fill[i];
            }
        }

        memcpy(data, output, size);
        // detach the shared memory 
        shmdt(data);

    }   

    // destroy the shared memory 
    shmctl(id, IPC_RMID, NULL);

    return 0; 
}
