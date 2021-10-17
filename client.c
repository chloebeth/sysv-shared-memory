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
    char input[100];
    char write_[108];
    char *data;
    key_t key;
    int id;

    char send[8] = "06872342";

    int size = 1024;

    // get the user input and send the chars to the memory mapped file 
    // so the server may read the input
    while(1) {
        // create the key 
        key = ftok("/tmp/shmem.dat", 2);

        // attached shared memory
        id = shmget(key, size, 0666 | IPC_CREAT);
        data = (char *)shmat(id, (void *)0, 0);

        // get the user input 
        char input[size];
        fgets(input, size, stdin);

        for (int i = 0; i < 8; i++) {
            write_[i] = send[i];
        }

        for (int i = 8; i < 108; i++) {
            write_[i] = toupper(input[i-8]);
        }

        memmove(data, write_, size);

        // detach the shared memory
        shmdt(data);
    }

    // destroy
    shmctl(id, IPC_RMID, NULL);

    return 0; 
}
