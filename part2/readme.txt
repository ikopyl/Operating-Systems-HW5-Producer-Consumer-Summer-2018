Name  : Ilya Kopyl
Date  : 08/04/2018
Class : CSC 415 Summer 2018

Compile Instructions:
$ make

Cleanup Instructions (deleting the previous build):
$ make clean


Run Instructions:
$ ./pandc <N> <P> <C> <X> <Ptime> <Ctime>
- where:
    N - number of buffers of size 1 (size of the queue)
    P - number of Producer threads
    C - number of Consumer threads
    X - number of items to enqueue for each Producer thread 
    Ptime - sleep time (seconds) for a Producer thread after Enqueue() call
    Ctime - sleep time (seconds) for a Consumer thread after Dequeue() call

For example:
./pandc 7 5 3 16 1 1


Project Description:


