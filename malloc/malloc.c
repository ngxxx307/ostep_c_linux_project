#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef struct __node_t
{
    int size;
    struct __node_t *next;
} node_t;

// Global variable to store the head of the heap.
// We initialize it to NULL. In my_malloc, we can check if it's NULL
// to see if we need to initialize the heap.
static node_t *head = NULL;

void *my_malloc(size_t size)
{
    if (head == NULL)
    {
        size_t heap_size = 1024 * 1024; // 1mb
        head = mmap(NULL, heap_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

        if (head == MAP_FAILED) // Use MAP_FAILED for better portability and readability
        {
            perror("error in mmap");
            return NULL; // Return NULL on failure
        }
        head->size = heap_size - sizeof(node_t);
        head->next = NULL;
    }
    return NULL;
}

void my_free(void *ptr)
{
}

int main(int argc, char *argv[])
{
    // You can test your malloc here, for example:
    // void *p = my_malloc(100);
    // if (p == NULL) {
    //     printf("my_malloc failed\n");
    // } else {
    //     printf("Successfully allocated 100 bytes\n");
    // }
    return 0;
}