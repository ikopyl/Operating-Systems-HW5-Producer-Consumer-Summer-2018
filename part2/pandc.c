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
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

typedef struct __node_t {
    size_t value;
    struct __node_t *next;
} node_t;

typedef struct __queue_t {
    node_t *head;
    node_t *tail;

    size_t max_capacity;
    size_t current_capacity;
    size_t total_processed;

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

static queue_t queue;


void init(queue_t *, unsigned int);
ssize_t dequeue_item(queue_t *);
ssize_t enqueue_item(queue_t *, size_t);
size_t is_queue_full(queue_t *);
size_t is_queue_empty(queue_t *);

void * produce(void *);
void * consume(void *);

void print_queue_recursively(node_t *, node_t *);
void check_for_errors_and_terminate(int, char *);




int main(int argc, char * argv[])
{
    init(&queue, 10);

    Ptime.tv_sec = 0;
    Ptime.tv_nsec = 0;

    Ctime.tv_sec = 0;
    Ctime.tv_nsec = 0;

    if (argc != 7)
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
    else
    {
        N = (size_t) strtol(argv[1], NULL, 0);
        P = (size_t) strtol(argv[2], NULL, 0);
        C = (size_t) strtol(argv[3], NULL, 0);
        X = (size_t) strtol(argv[4], NULL, 0);
        Ptime.tv_sec = strtol(argv[5], NULL, 0);
        Ctime.tv_sec = strtol(argv[6], NULL, 0);

        printf("                 Number of buffers of size1, N : %6zu\n", N);
        printf("                 Number of Producer threads, P : %6zu\n", P);
        printf("                 Number of Consumer threads, C : %6zu\n", C);
        printf("Number of items to produce by each Producer, X : %6zu\n", X);
        printf("   Number of items to consume by each Consumer : %6zu\n", P * X / C + P * X % C);
        printf("                              Over consume on? : %6zu\n", (size_t) 1);

        printf(KCYN"                           Over consume amount : %6zu\n", (size_t) 12);
        printf(KNRM);

        printf("           Time each Producer sleeps (seconds) : %6zu\n", Ptime.tv_sec);
        printf("           Time each Consumer sleeps (seconds) : %6zu\n", Ctime.tv_sec);

    }




//    printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
//    printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");
//    printf("Size of queue: %zu\n", sizeof(queue));
//
//    for (size_t i = 1; i <= 10; i++) {
//        printf("i = %zu\n", i);
//
//        printf("Enqueued: %zu\n", enqueue_item(&queue, i));
//
//        printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
//        printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");
//    }
//
//    print_queue_recursively(queue.head, queue.tail);
//
//    puts("---------------------------------");
//
//    printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
//    printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");
//
//    for (size_t i = 1; i <= 5; i++) {
//        printf("i = %zu\n", i);
//
//        printf("Dequeued: %zu\n", dequeue_item(&queue));
//
//        printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
//        printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");
//    }
//
//    print_queue_recursively(queue.head, queue.tail);
//
//
//    for (size_t i = 1; i <= 5; i++) {
//        printf("i = %zu\n", i);
//
//        printf("Dequeued: %zu\n", dequeue_item(&queue));
//
//        printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
//        printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");
//    }
//
//    print_queue_recursively(queue.head, queue.tail);

    return 0;
}


/** Tribute to Jozo */
void print_queue_recursively(node_t * head, node_t * tail)
{
    printf("%zu\n", head->value);
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
    for (int i = 0; i < 10; i++)                                            /***/
    {
        sem_wait(&queue.empty_buffers);
        ssize_t enqueued_value = enqueue_item(&queue, 231);                 /***/
        sem_post(&queue.full_buffers);
        printf(KGRN "Item #%zu was produced by producer thread #%zu\n", enqueued_value, (size_t) args);
        printf(KNRM);
        nanosleep(&Ptime, NULL);
    }
}


void * consume(void * args)
{
    for (int i = 0; i < 10; i++)
    {
        sem_wait(&queue.full_buffers);
        ssize_t dequeued_value = dequeue_item(&queue);
        sem_post(&queue.empty_buffers);
        printf(KYEL "Item #%zu was consumed by consumer thread #%zu\n", dequeued_value, (ssize_t) args);
        printf(KNRM);
        nanosleep(&Ctime, NULL);
    }
}




void init(queue_t *q, unsigned int queue_size)
{
    node_t *tmp = malloc(sizeof(node_t));
    tmp->value = 0;
    tmp->next = NULL;

//    q->node_array = calloc(queue_size, sizeof(node_t *));
    q->max_capacity = queue_size;
    q->current_capacity = 0;
    q->total_processed = 0;

    q->head = q->tail = tmp;
//    q->node_array[q->current_capacity] = tmp;

    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);

    int status = sem_init(&q->empty_buffers, 0, queue_size);
    check_for_errors_and_terminate(status, "Failed to initialize a semaphore...");
    status = sem_init(&q->full_buffers, 0, 0);
    check_for_errors_and_terminate(status, "Failed to initialize a semaphore...");
}

/* 
 * Function to remove item.
 * Item removed is returned
 */
ssize_t dequeue_item(queue_t *q)
{
    pthread_mutex_lock(&q->head_lock);

    node_t *tmp = q->head;
    node_t *new_head = tmp->next;
    if (new_head == NULL)
    {
        pthread_mutex_unlock(&q->head_lock);
        perror("Failed to dequeue an item: the queue is empty...");
        return -1;
    }
    else
    {
        ssize_t old_value = q->head->value;
        q->head = new_head;

        q->tail->next = q->head;

        q->current_capacity--;

        pthread_mutex_unlock(&q->head_lock);
        free(tmp);

        return old_value;
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
ssize_t enqueue_item(queue_t *q, size_t item)
{
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL)
    {
        perror("Failed to allocate memory for new node...");
        return -1;
    }
    else
    {
        new_node->value = item;
        new_node->next = NULL;

        pthread_mutex_lock(&q->tail_lock);


        q->tail->next = new_node;
        q->tail = new_node;

        if (is_queue_empty(q)) {                /** for now q->current_capacity constitutes the emptiness/fullness of the queue */
            q->head = q->tail;
        }

//        q->node_array[q->current_capacity++] = new_node;
        q->current_capacity++;

        if (is_queue_full(q)) {
            q->tail->next = q->head;
        }

        pthread_mutex_unlock(&q->tail_lock);

        return q->tail->value;
    }
}


size_t is_queue_full(queue_t *q)
{
    if (q->current_capacity == q->max_capacity)
//    if (q->tail != q->head && q->head->next == q->tail)
        return 1;
    return 0;
}

size_t is_queue_empty(queue_t *q)
{
    if (q->current_capacity == 0)
//    if (q->head == q->tail)
        return 1;
    return 0;
}


