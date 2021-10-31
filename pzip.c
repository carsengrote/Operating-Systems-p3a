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


int zipWorker(char* file, int start, int size, int order){


    // function here yo 
    // Russ if you're reading this I'm a bit intoxicated
    

    availableThreads++;
    return 0;
}

int main(int argc, char* argv[]){

    // decides how many threads to have work on each file
    // if there's less than 5 files then 2 threads per file
    // if theres 5 or more then just one thread per file
    int threadsPerFile;

    if (argc >= 5){
        
        threadsPerFile = 1;
    
    }else{
        
        threadsPerFile = 2;
    }
    
    // represents the total amount of threads that have been created
    int totalThreads = 0;

    // for loop for each file given as an arguemnt
    for(int fileNo = 1; fileNo < argc; fileNo++){
    
        int currFile = open(argv[fileNo], O_RDONLY);
        
        // gets size of the file
        struct stat st;
        fstat(currFile, &st);
        int size = st.st_size;

        // maps the whole file to memory
        char* mappedFile = mmap(0, size, PROT_READ, MAP_PRIVATE, currFile, 0);


        // Need to calculate starting / ending address for each thread based off of file size and
        // how many threads I want per file

        int sectionSize = (size / threadsPerFile);
        char* currThreadAddr = mappedFile; 
        pthread_t threads[threadsPerFile];       
        
        // for loop for the number of threads per file
        for (int currThread = 0; currThread < threadsPerFile; currThread++){
             
             // wait til we can get a new thread - only have 4 total
             while(availableThreads < 1)
                ;

             int startAddr = (sectionSize * currThread);
             
             availableThreads--;
             pthread_create(&threads[currThread], NULL, zipWorker, mappedFile, startAddr, size, totalThreads);
             totalThreads++;

        }

    }

    return 0;
}
