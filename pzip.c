#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int currWriter = 0;
int availableThreads = 4;


int zipWorker(char *start, char* end, int order){

    

    availableThreads++;
    return 0;
}

int main(int argc, char* argv[]){

    printf("%d %d\n", get_nprocs(), get_nprocs_conf());
    
    int threadsPerFile;

    if (argc >= 5){
        
        threadsPerFile = 1;
    
    }else{
        
        threadsPerFile = 2;
    }
   

    for(int fileNo = 1; fileNo < argc; fileNo++){
    
        int currFile = open(argv[fileNo], O_RDONLY);
        
        struct stat st;
        fstat(currFile, &st);
        int size = st.st_size;

        char* mappedFile = mmap(0, size, PROT_READ, MAP_PRIVATE, currFile, 0);


        // Need to calculate starting / ending address for each thread based off of file size and
        // how many threads I want per file

        for (int currThread = 0; currThread < threadsPerFile; currThread++){
             
             // put semaphore here 
             while(availableThreads < 1)
                ;

             
             

        }

    }

    return 0;
}
