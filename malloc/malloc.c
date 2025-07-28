#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdbool.h>

// Renamed to reflect its purpose in a buddy system: it's a header for ANY block.
typedef struct block_header
{
    uint8_t order;
    bool is_free;              // Using stdbool.h is great!
    struct block_header *next; // For the free list
} block_header_t;

// Global variable to store the head of the heap.
// We initialize it to NULL. In my_malloc, we can check if it's NULL
// to see if we need to initialize the heap.
#define TOTAL_MEMORY_SIZE (1024 * 1024) // 1MB = 2^20
#define MIN_ORDER 4                     // Smallest block size: 2^4 = 16 bytes
#define MAX_ORDER 20                    // Largest block size: 2^20 = 1MB
#define NUM_FREE_LISTS (MAX_ORDER - MIN_ORDER + 1)

static block_header_t *free_lists[NUM_FREE_LISTS];
static bool is_initialized = false;

void *my_malloc(size_t size)
{
    if (!is_initialized)
    {
        block_header_t *head = mmap(NULL, TOTAL_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        int n_free_list = TOTAL_MEMORY_SIZE / MIN_ORDER;

        if (head == MAP_FAILED) // Use MAP_FAILED for better portability and readability
        {
            perror("error in mmap");
            return NULL; // Return NULL on failure
        }
        head->order = MAX_ORDER;
        head->next = NULL;
        head->is_free = true;

        free_lists[MAX_ORDER - MIN_ORDER] = head;
    }
    return NULL;
}

void my_free(void *ptr)
{
}

int main(int argc, char *argv[])
{
    return 0;
}