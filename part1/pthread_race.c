#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
/**
 * THESE DEFINE VALUES CANNOT BE CHANGED.
 * DOING SO WILL CAUSE POINTS TO BE DEDUCTED
 * FROM YOUR GRADE
 */
 /** BEGIN VALUES THAT CANNOT BE CHANGED */
#define MAX_THREADS 10
#define MAX_ITERATIONS 25
/** END VALUES THAT CANNOT BE CHANGED */


/**
 * use this struct as a parameter for the function
 * nanosleep. 
 * For exmaple : nanosleep(&ts, NULL);
 */
//struct timespec ts = {2, 0 };
struct timespec ts = { 0, 10 };              /** 10ns - one "shake", (as in a "shake of a lamb's tail"); approximate time of one generation of a nuclear chain reaction with fast neutrons */

static volatile int global_variable = 0;

static void * thread_sub(void *);
static void * thread_add(void *);
ssize_t print_thread_info(size_t);
void check_for_errors_and_terminate(int, char *);

int main(int argc, char * argv[])
{
    pthread_t t1, t2;
    int status_code, i;
    size_t count;

    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        count = 0;

        while (count < MAX_THREADS)
        {
            status_code = pthread_create(&t1, NULL, thread_sub, (void *) ++count);
            check_for_errors_and_terminate(status_code, "pthread_create failed...");

            status_code = pthread_create(&t2, NULL, thread_add, (void *) ++count);
            check_for_errors_and_terminate(status_code, "pthread_create failed...");

            status_code = pthread_join(t1, NULL);
            check_for_errors_and_terminate(status_code, "pthread_join failed...");

            status_code = pthread_join(t2, NULL);
            check_for_errors_and_terminate(status_code, "pthread_join failed...");
        }

    }

    printf("Final Value of Shared Variable is: %d\n", global_variable);



    return 0;
}

static void * thread_sub(void * args)
{
    int temp = global_variable;
    temp -= 1;
    nanosleep(&ts, NULL);
    global_variable = temp;
    return (void *) print_thread_info((size_t) args);
}

static void * thread_add(void * args)
{
    int temp = global_variable;
    temp += 1;
    global_variable = temp;
    return (void *) print_thread_info((size_t) args);
}

ssize_t print_thread_info(size_t thread_num)
{
    return printf("Current Value written to Global Variables by thread: %6lu \tis %6d\n", thread_num, global_variable);
}

void check_for_errors_and_terminate(int status_code, char * error_message)
{
    if (status_code != 0) {
        perror(error_message);
        exit(status_code);
    }
}

