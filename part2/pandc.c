#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>

/** color output thingy */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KCYN  "\x1B[36m"

typedef struct __node_t {
    size_t value;
    struct __node_t *next;
} node_t;

typedef struct __queue_t {
    node_t *head;
    node_t *tail;

    size_t consumed_index;
    size_t produced_index;

    size_t *log_of_produced_items;
    size_t *log_of_consumed_items;

    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;

    sem_t empty_buffers;
    sem_t full_buffers;

} queue_t;

                                    /** COMMAND LINE PARAMETERS: */
static size_t N = 0;                /** number of buffers of size 1 */
static size_t P = 0;                /** number of producer threads */
static size_t C = 0;                /** number of consumer threads */
static size_t X = 0;                /** number of elements to enqueue for each Producer thread */
static struct timespec Ptime;       /** thread sleep time after each Enqueue() */
static struct timespec Ctime;       /** thread sleep time after each Dequeue() */

static size_t expected_produced_amount_total = 0;
static size_t consume_amount_per_thread = 0;

static pthread_mutex_t sneaky_mutex;

static int is_overconsume = 0;
static size_t over_consume_amount = 0;

static queue_t queue;


void init(queue_t *, size_t);
size_t dequeue_item(queue_t *);
size_t enqueue_item(queue_t *, size_t);
size_t is_queue_full(queue_t *);
size_t is_queue_empty(queue_t *);

void * produce(void *);
void * consume(void *);

void print_queue_recursively(node_t *, node_t *);
void print_ux_message_wrong_number_of_arguments();
void print_ux_message_success();
time_t print_current_time();
void check_for_errors_and_terminate(int, char *);

int compare_two_arrays_verbose_mode(size_t *, size_t *, size_t);




int main(int argc, char * argv[])
{
    if (argc != 7)
    {
        print_ux_message_wrong_number_of_arguments();
    }
    else
    {
        time_t start_time = print_current_time();

        Ptime.tv_sec = 0;
        Ptime.tv_nsec = 0;

        Ctime.tv_sec = 0;
        Ctime.tv_nsec = 0;

        N = (size_t) strtol(argv[1], NULL, 0);
        P = (size_t) strtol(argv[2], NULL, 0);
        C = (size_t) strtol(argv[3], NULL, 0);
        X = (size_t) strtol(argv[4], NULL, 0);
        Ptime.tv_sec = strtol(argv[5], NULL, 0);
        Ctime.tv_sec = strtol(argv[6], NULL, 0);

        expected_produced_amount_total = P * X;
        consume_amount_per_thread = P * X / C;
        is_overconsume = P * X % C > 0 ? 1 : 0;
        over_consume_amount = (P * X % C);

        print_ux_message_success();

        init(&queue, N);




        int status_code;
        size_t producer_threads_count = 0;
        size_t consumer_threads_count = 0;

        pthread_t * producers = calloc(P, sizeof(pthread_t));
        pthread_t * consumers = calloc(C, sizeof(pthread_t));

        pthread_mutex_init(&sneaky_mutex, NULL);


        for (int i = 0; i < P; i++) {
            producer_threads_count++;
            status_code = pthread_create(&producers[i], NULL, produce, (void *) producer_threads_count);
            check_for_errors_and_terminate(status_code, "Failed to create a producer thread...");
        }

        for (int i = 0; i < C; i++) {
            consumer_threads_count++;
            status_code = pthread_create(&consumers[i], NULL, consume, (void *) consumer_threads_count);
            check_for_errors_and_terminate(status_code, "Failed to create a consumer thread...");
        }

        for (int i = 0; i < P; i++) {
            status_code = pthread_join(producers[i], NULL);
            check_for_errors_and_terminate(status_code, "Failed to join a producer thread...");
            printf("Producer Thread #%d joined.\n", i+1);
        }

        for (int i = 0; i < C; i++) {
            status_code = pthread_join(consumers[i], NULL);
            check_for_errors_and_terminate(status_code, "Failed to join a consumer thread...");
            printf("Consumer Thread #%d joined.\n", i+1);
        }
        time_t finish_time = print_current_time();

        int cmp = compare_two_arrays_verbose_mode(
                queue.log_of_produced_items,
                queue.log_of_consumed_items,
                expected_produced_amount_total);
        printf("\nConsume and Produce Arrays %s\n\n", cmp == 0 ? "Match!" : "Do Not Match!");

        printf("Total runtime: %6.1f seconds\n", (double) finish_time - start_time);

    }



    return 0;
}


/** Tribute to Jozo */
void print_queue_recursively(node_t * head, node_t * tail)
{
    printf("%zu\n", head->value);
//    printf("%zu\t", head->value);
    if (head == tail) {
        return;
    } else {
        print_queue_recursively(head->next, tail);
    }
}


void check_for_errors_and_terminate(int status_code, char * error_message)
{
    if (status_code < 0) {
        perror(error_message);
        exit(errno);
    }
}


void * produce(void * args)
{
    for (int i = 0; i < X; i++)                                            /***/
    {
        sem_wait(&queue.empty_buffers);
        pthread_mutex_lock(&queue.tail_lock);

        size_t enqueued_value = enqueue_item(&queue, (size_t) args);                 /***/

        pthread_mutex_unlock(&queue.tail_lock);
        sem_post(&queue.full_buffers);

        printf(KGRN "Item #%zu was produced by producer thread #%zu\n", enqueued_value, (size_t) args);
        printf(KNRM);

        nanosleep(&Ptime, NULL);
    }
    return (void *) 0;
}


void * consume(void * args)
{
    for (int i = 0; i < consume_amount_per_thread; i++)
    {
        int tmp = 0;
        sem_getvalue(&queue.empty_buffers, &tmp);

        if (is_overconsume && over_consume_amount > 0) {                  /** dirty little hack */
            pthread_mutex_lock(&sneaky_mutex);
            i--;
            over_consume_amount--;
            pthread_mutex_unlock(&sneaky_mutex);
        }

        sem_wait(&queue.full_buffers);
        pthread_mutex_lock(&queue.head_lock);

        size_t dequeued_value = dequeue_item(&queue);

        pthread_mutex_unlock(&queue.head_lock);
        sem_post(&queue.empty_buffers);

        printf(KYEL "Item #%zu was consumed by consumer thread #%zu\n", dequeued_value, (ssize_t) args);
        printf(KNRM);

        nanosleep(&Ctime, NULL);
    }
    return (void *) 0;
}


void init(queue_t *q, size_t queue_size)
{
//    node_t *tmp = malloc(sizeof(node_t));
    node_t *tmp = calloc(1, sizeof(node_t));
    tmp->value = 0;
    tmp->next = NULL;

    q->consumed_index = 0;
    q->produced_index = 0;

    q->log_of_produced_items = calloc(P * X, sizeof(size_t));
    q->log_of_consumed_items = calloc(P * X, sizeof(size_t));

    q->head = q->tail = tmp;

    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);

    int status = sem_init(&q->empty_buffers, 0, (unsigned int) queue_size);
    check_for_errors_and_terminate(status, "Failed to initialize a semaphore...");
    status = sem_init(&q->full_buffers, 0, 0);
    check_for_errors_and_terminate(status, "Failed to initialize a semaphore...");
}


/* 
 * Function to remove item.
 * Item removed is returned
 */
size_t dequeue_item(queue_t *q)
{
//    pthread_mutex_lock(&q->head_lock);

    node_t *tmp = q->head;
    node_t *new_head = tmp->next;
    if (new_head == NULL)
    {
//        pthread_mutex_unlock(&q->head_lock);
        perror("Failed to dequeue an item: the queue is empty...");
        return 0;
    }
    else
    {
        size_t consumed_value = q->head->value;
        q->head = new_head;

        q->tail->next = q->head;

        q->log_of_consumed_items[q->consumed_index] = consumed_value;
        q->consumed_index++;

//        pthread_mutex_unlock(&q->head_lock);
        free(tmp);

        return consumed_value;
    }
}

/* 
 * Function to add item.
 * Item added is returned.
 * It is up to you to determine
 * how to use the return value.
 * If you decide to not use it, then ignore
 * the return value, do not change the
 * return type to void. 
 */
size_t enqueue_item(queue_t *q, size_t item)
{
//    node_t *new_node = malloc(sizeof(node_t));
    node_t *new_node = calloc(1, sizeof(node_t));
    if (new_node == NULL)
    {
        perror("Failed to allocate memory for new node...");
        return 0;
    }
    else
    {
        new_node->value = item;
        new_node->next = NULL;

//        pthread_mutex_lock(&q->tail_lock);

        q->tail->next = new_node;
        q->tail = new_node;

        if (is_queue_empty(q)) {                /** for now q->current_capacity constitutes the emptiness/fullness of the queue */
            q->head = q->tail;
        }

        q->log_of_produced_items[q->produced_index] = q->tail->value;
        q->produced_index++;


        if (is_queue_full(q)) {
            q->tail->next = q->head;
        }

//        int tmp = 0;
//        sem_getvalue(&q->empty_buffers, &tmp);
//        if (tmp == 1) {
//            q->tail->next = q->head;
//            q->head->next = q->tail;
//        }

//        pthread_mutex_unlock(&q->tail_lock);

        return q->tail->value;
    }
}


size_t is_queue_full(queue_t *q)
{
    int tmp = 0;
    sem_getvalue(&q->empty_buffers, &tmp);
    if (tmp == 0)
        return 1;
    return 0;
}

size_t is_queue_empty(queue_t *q)
{
    int tmp = 0;
    sem_getvalue(&q->full_buffers, &tmp);
    if (tmp == 0)
        return 1;
    return 0;
}


void print_ux_message_wrong_number_of_arguments()
{
    puts("Wrong number of arguments.");
    puts("Please specify 6 command line arguments as follows:");
    printf("\t$ ./pandc <N> <P> <C> <X> <Ptime> <Ctime>\n");
    printf("where\n");
    printf("\tN - number of buffers of size 1 (size of the queue)\n");
    printf("\tP - number of Producer threads\n");
    printf("\tC - number of Consumer threads\n");
    printf("\tX - number of items to enqueue for each Producer thread");
    printf("\tPtime - sleep time (seconds) for a Producer thread after Enqueue() call\n");
    printf("\tCtime - sleep time (seconds) for a Consumer thread after Dequeue() call\n");
}


void print_ux_message_success()
{
    printf("                 Number of buffers of size1, N : %6zu\n", N);
    printf("                 Number of Producer threads, P : %6zu\n", P);
    printf("                 Number of Consumer threads, C : %6zu\n", C);
    printf("Number of items to produce by each Producer, X : %6zu\n", X);
    printf("   Number of items to consume by each Consumer : %6zu\n", consume_amount_per_thread);
    printf("                              Over consume on? : %6d\n", is_overconsume);

    if (is_overconsume)
    {
        printf(KCYN"                           Over consume amount : %6zu\n", consume_amount_per_thread + over_consume_amount);
        printf(KNRM);
    }


    printf("           Time each Producer sleeps (seconds) : %6zu\n", Ptime.tv_sec);
    printf("           Time each Consumer sleeps (seconds) : %6zu\n", Ctime.tv_sec);
    puts("");
}


time_t print_current_time()
{
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("Current time: %s\n\n", asctime (timeinfo));

    return rawtime;
}


int compare_two_arrays_verbose_mode(size_t * producer_array, size_t * consumer_array, size_t n)
{
    int is_mismatch_found = 0;
    printf("Producer Array\t| Consumer Array\n");

    for (int i = 0; i < n; i++) {
        if (producer_array[i] == consumer_array[i]) {
            printf(KGRN "%3zu\t\t| %3zu\n", producer_array[i], consumer_array[i]);
        } else {
            printf(KRED "%3zu\t\t| %3zu\n", producer_array[i], consumer_array[i]);
            is_mismatch_found = 1;
        }
        printf(KNRM);
    }
    return is_mismatch_found;
}
