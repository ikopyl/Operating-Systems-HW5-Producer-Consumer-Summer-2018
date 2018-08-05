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
In this project I was trying to solve a problem of synchronization and interthread communication while solving the classical problem. One of the main challenges was to make sure that the Consumer does not oupaces the Producer. I did my best to ensure that the threads behave nicely. I decided to use a the Michael and Scott algorithm for implementing a concurrent queue. But I stumbled upon unexpected issues which could be caused by potential bugs in either the algorithm itself or my implementation.
