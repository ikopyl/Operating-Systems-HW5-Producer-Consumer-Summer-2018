#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>


/** All hail OOP! */
typedef struct __node_t {
    size_t value;
    struct __node_t *next;
} node_t;

typedef struct __queue_t {
    node_t *head;
    node_t *tail;

    size_t max_capacity;
    size_t current_capacity;

    node_t ** node_array;

    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;

} queue_t;



void init(queue_t *, size_t);
ssize_t dequeue_item(queue_t *);
ssize_t enqueue_item(queue_t *, size_t);
size_t is_queue_full(queue_t *);
size_t is_queue_empty(queue_t *);


int main(int argc, char * argv[])
{

    queue_t queue;
    init(&queue, 10);

    printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
    printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");
    printf("Size of queue: %zu\n", sizeof(queue));

    for (size_t i = 1; i <= 10; i++) {
        printf("i = %zu\n", i);

        printf("Enqueued: %zu\n", enqueue_item(&queue, i));

        printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
        printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");
    }

    puts("---------------------------------");

    size_t j = 0;
    while (j < 30) {
        ssize_t val = queue.node_array[j % queue.current_capacity]->value;              // DEBUG INFO
        printf("Current value: %zu\n", val);
        printf("\tand next: %zu\n", queue.node_array[j % queue.current_capacity]->next->value);     // DEBUG INFO
        j++;
    }

    puts("---------------------------------");

    printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
    printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");

    for (size_t i = 1; i <= 10; i++) {
        printf("i = %zu\n", i);

        printf("Dequeued: %zu\n", dequeue_item(&queue));

        printf("Is queue empty? %s\n", is_queue_empty(&queue) ? "true" : "false");
        printf("Is queue full? %s\n", is_queue_full(&queue) ? "true" : "false");
    }



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

    q->head = q->tail = tmp;
    q->node_array[q->current_capacity] = tmp;

    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);
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

        if (is_queue_empty(q)) {
            q->head = q->tail;
        }

        q->node_array[q->current_capacity++] = new_node;

        if (is_queue_full(q)) {
            q->tail->next = q->head;
        }

        pthread_mutex_unlock(&q->tail_lock);

//        printf("Enqueued: %zu\n", q->tail->value);                                  // DEBUG INFO
//        printf("Enqueued: %zu\n", q->node_array[q->current_capacity-1]->value);     // DEBUG INFO

        return q->tail->value;
    }
}


size_t is_queue_full(queue_t *q)
{
    if (q->current_capacity == q->max_capacity)
        return 1;
    return 0;
}

size_t is_queue_empty(queue_t *q)
{
    if (q->current_capacity == 0)
        return 1;
    return 0;
}


