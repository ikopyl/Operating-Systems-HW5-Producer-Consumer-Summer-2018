#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>


typedef struct __node_t {
    int value;
    struct __node_t *next;
} node_t;

typedef struct __queue_t {
    node_t *head;
    node_t *tail;
    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;

    size_t max_capacity;
    size_t current_capacity;
    size_t index;

    node_t ** node_array;
} queue_t;



void init(queue_t *, size_t);
int dequeue_item();
int enqueue_item(int);



int main(int argc, char * argv[])
{

    return 0;
}



void init(queue_t *q, size_t queue_size)
{
    node_t *tmp = malloc(sizeof(node_t));
    tmp->value = 0;
    tmp->next = NULL;

    q->node_array = calloc(queue_size, sizeof(node_t *));
    q->max_capacity = queue_size;
    q->current_capacity = 0;
    q->index = 0;

    q->head = q->tail = tmp;
    q->node_array[q->index] = tmp;

    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);
}

/* 
 * Function to remove item.
 * Item removed is returned
 */
int dequeue_item()
{

    return 0;
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
int enqueue_item(int item)
{
    printf("Enqueued: %d\n", item);
    return 0;
}

