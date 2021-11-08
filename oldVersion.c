#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include<sched.h>
 
sem_t sem;
int currentWriter;
int availableThreads[3];
int lastThreadCount;
char lastThreadChar;
int totalThreads;
int mode; 
int badFirst;

struct thread_args{ 
    //add size
    char* fileName;
    int myTurn;
    int arrayIndex;
};

void * zipWorker(void* inputArgs){

    struct thread_args* args = (struct thread_args*) inputArgs;
    int myTurn = args->myTurn;
    int arrayIndex = args->arrayIndex;
    char* fileName = args->fileName;

    int file = open(fileName, O_RDONLY);

    if (file < 0){
        if(myTurn == 0) {
            badFirst = 1;
        }
        if (myTurn == totalThreads){
            fwrite(&(lastThreadCount), 4, 1, stdout);
            fwrite(&(lastThreadChar), 1, 1, stdout); 
        }
        free(args->fileName);
        free(args);
        while(currentWriter != myTurn){
            sched_yield();
        }
        //lastThreadCount = 0;
        currentWriter++;
        availableThreads[arrayIndex] = 0;
        sem_post(&sem);
        return 0;
    }
        
    // gets size of the file
    //overflowing stack?
    struct stat st;
    fstat(file, &st);
    int size = st.st_size;
    //printf("\nsize: %d\n", size);

    char* mappedFile = mmap(0, size, PROT_READ, MAP_PRIVATE, file, 0);

    char* charOutput = malloc((size + 1) * sizeof(char));
    int* intOutput = malloc((size + 1) * sizeof(int));
    int outputIndex  = 0;

    int currentCount = 1;
    
    int index = 0;
    while(mappedFile[index] == '\0') {
        index++;
    }
    char currentChar = mappedFile[index];

    while(index < (size - 1)){

        //ignore null characters
        if(mappedFile[index+1] == '\0') {
            index++;
            continue;
        }

        // if current char is same as next
        if(mappedFile[index + 1] == currentChar){
            currentCount++;
            charOutput[outputIndex] = currentChar;
            intOutput[outputIndex] = currentCount;
            // if second to last char have to increment index
            if (index  == (size - 2)){
                outputIndex++;
            }

        } else{

            charOutput[outputIndex] = currentChar;
            intOutput[outputIndex] = currentCount;
            currentCount = 1; 
            outputIndex++;

            if (index == (size - 2)){
                charOutput[outputIndex] = mappedFile[index + 1];
                intOutput[outputIndex] = 1;
                outputIndex++;
            }
        }
    
        currentChar = mappedFile[index + 1];
        index++;
    }
    charOutput[outputIndex] = '\0';

    //int writeLength = strlen(charOutput);
    int writeLength = outputIndex; 

    int currentWrite = 0;

    while(currentWriter != myTurn){
        sched_yield();
    }

    if ((myTurn > 0) && (lastThreadChar == charOutput[0]) && (lastThreadCount != 0)){
        intOutput[0] = intOutput[0] + lastThreadCount;
    } else if (myTurn > 0 && (lastThreadCount != 0)){

        if(mode == 0){
            printf("%d", lastThreadCount);
            printf("%c\n", lastThreadChar);
        }else {
            // check to make sure lastThreadCount is NULL
            if(badFirst != 1 || myTurn != 1) {
                fwrite(&(lastThreadCount), 4, 1, stdout);
                fwrite(&(lastThreadChar), 1, 1, stdout); 
            }
            //else {
            //    printf("bad file %s\n", fileName);
            //}
        }
    }

    while(currentWrite < (writeLength - 1)){
        if (mode == 0){
            printf("%d", intOutput[currentWrite]);
            printf("%c\n", charOutput[currentWrite]);
        } else {
            fwrite(&(intOutput[currentWrite]), 4, 1, stdout);
            fwrite(&(charOutput[currentWrite]), 1, 1, stdout);
        }
        currentWrite++;
    }

    // if it's the last thread in the whole program - write the last char + int to output,
    // if it's not the last thread, save the last char and count for next thread
    if (myTurn == totalThreads){
        if (mode == 0){
            printf("%d", intOutput[currentWrite]);
            printf("%c\n", charOutput[currentWrite]);
        } else{
            fwrite(&(intOutput[currentWrite]), 4, 1, stdout);
            fwrite(&(charOutput[currentWrite]), 1, 1, stdout);
        }
    } else{
        lastThreadChar = charOutput[currentWrite];
        lastThreadCount = intOutput[currentWrite];
    }

    close(file);
    free(args->fileName);
    free(intOutput);
    free(charOutput);
    free(args);
    currentWriter++;
    availableThreads[arrayIndex] = 0;
    sem_post(&sem);
    return 0;
}


int main(int argc, char* argv[]){

    mode = 1;
    badFirst = 0;

    int numFiles = argc - 1;

    if (numFiles == 0){
        printf("pzip: file1 [file2 ...]\n");
        exit(1);
    }
    totalThreads = numFiles - 1;

    pthread_t threads[3];

    sem_init(&sem, 0, 3);

    int currentThreadNumber = 0;

    currentWriter = 0;

    for(int fileNo = 0; fileNo < numFiles; fileNo++){

        sem_wait(&sem);

        // finding availible thread index
        int threadIndex = 0;
        while (availableThreads[threadIndex] == 1){
            threadIndex++;
        }
        // get size and find out if we need to multithread a single file
        //change struct to include size (one more than max index)
        // mem map file befroe threading
        // mallocing and setting arguments for thread
        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->fileName = strdup(argv[fileNo + 1]);
        args->myTurn = currentThreadNumber;
        args->arrayIndex = threadIndex;

        // setting the index in available threads to used
        availableThreads[threadIndex] = 1;

        pthread_create(&threads[threadIndex], NULL, zipWorker, args);
        currentThreadNumber++;
    }

    if (availableThreads[0] == 1){
        pthread_join(threads[0], NULL);
    }
    if (availableThreads[1] == 1){
        pthread_join(threads[1], NULL);
    }
    if (availableThreads[2] == 1){
        pthread_join(threads[2], NULL);
    }

    return 0;
}
