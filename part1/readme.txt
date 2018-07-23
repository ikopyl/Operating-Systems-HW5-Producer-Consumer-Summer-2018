Name  : Ilya Kopyl
Date  : 07/23/2018
Class : CSc 415 Summer 2018 (csc415-01-su18)

Compile Instructions:
$ make

Cleanup Instructions (delete previous build):
$ make clean

Run Instructions:
$ ./threadracer


Project Description:

For this assignment I copied the source code in its entirety from the homework assignment 4, and I just added mutex (pthread_mutex_t). I inserted the mutex lock inside the functions thread_sub() and thread_add() (the functions pointers to which are passed as arguments during the threads creation). That eliminated the existing race condition and the threads are now synchronized (i.e. not trying to execute at the same time).
