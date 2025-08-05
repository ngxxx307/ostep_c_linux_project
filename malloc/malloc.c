#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

// Renamed to reflect its purpose in a buddy system: it's a header for ANY block.
typedef struct block_header
{
    uint8_t order;
    bool is_free;
    struct block_header *next;
} block_header_t;

#define TOTAL_MEMORY_SIZE (1024 * 1024) // 1MB = 2^20
#define MIN_ORDER 4                     // Smallest block size: 2^4 = 16 bytes
#define MAX_ORDER 20                    // Largest block size: 2^20 = 1MB
#define NUM_FREE_LISTS (MAX_ORDER - MIN_ORDER + 1)

#define DEBUG = false

static block_header_t *free_lists[NUM_FREE_LISTS];
static bool is_initialized = false;
static void *memory_pool_start = NULL;

/**
 * @brief Recursively prints the details of each block in a single free list.
 *
 * @param block The current block in the list to print.
 * @param block_num The sequential number of the block in the list (start with 1).
 */
void print_list_recursive(block_header_t *block, int block_num)
{
    // Base case: If the block is NULL, we've reached the end of the list.
    if (block == NULL)
    {
        return;
    }

    // Print details of the current block.
    // We use the memory pool start to calculate a relative address for readability.
    printf("      - Block %d: Addr=0x%lx (Offset=%ld), Order=%u, is_free=%s\n",
           block_num,
           (unsigned long)block,
           (long)((char *)block - (char *)memory_pool_start),
           block->order,
           block->is_free ? "true" : "false");

    // Recursive step: Call the function for the next block in the list.
    print_list_recursive(block->next, block_num + 1);
}

/**
 * @brief Iterates through all free lists and prints their contents.
 */
void print_all_free_lists()
{
    printf("\n--- Current State of Free Lists ---\n");
    bool any_free_blocks = false;

    // Iterate from the smallest to the largest order.
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        // The order corresponding to the current list index.
        int current_order = i + MIN_ORDER;
        block_header_t *head = free_lists[i];

        // If the list is not empty, print its header and its contents.
        if (head != NULL)
        {
            any_free_blocks = true;
            size_t block_size = 1 << current_order;
            printf("  [Order %d] (Size: %zu bytes):\n", current_order, block_size);

            // Start the recursive printing for this list.
            print_list_recursive(head, 1);
        }
    }

    if (!any_free_blocks)
    {
        printf("  All memory is allocated. No blocks are in the free lists.\n");
    }
    printf("-------------------------------------\n");
}

int get_order(size_t size)
{
    if (size == 0)
    {
        return 0;
    }
    // Handle size=1 edge case, as __builtin_clz(0) is undefined.
    if (size == 1)
    {
        return 0;
    }

    unsigned int n = size - 1;
    int order = (sizeof(unsigned int) * CHAR_BIT) - __builtin_clz(n);

    return order;
}

int unregister_freelist(block_header_t *block) // Remove node from free list
{
    block_header_t *curr = free_lists[block->order - MIN_ORDER];
    block_header_t *prev = NULL;

    while (curr != block && curr != NULL)
    {
        prev = curr;
        curr = curr->next;
    }
    if (curr == NULL) // cannot find node in list
    {
        fprintf(stderr, "Error: Block to unregister was not found in its free list.\n");
        return -1;
    }
    if (prev == NULL) // case curr = head
    {
        free_lists[block->order - MIN_ORDER] = curr->next;
        return 0;
    }
    // Normal case
    prev->next = curr->next;

    // Set block to be not free
    block->next = NULL;
    return 0;
}

int register_freelist(block_header_t *block)
{
    block_header_t *curr = free_lists[block->order - MIN_ORDER];
    if (curr == NULL)
    {
        free_lists[block->order - MIN_ORDER] = block;
        return 0;
    }
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = block;

    return 0;
}

block_header_t *split(block_header_t *block)
{
    block->order = block->order - 1;
    block_header_t *buddy = (block_header_t *)((char *)block + (1 << block->order));
    buddy->order = block->order;
    buddy->is_free = true;
    buddy->next = NULL;
    return buddy;
}

void *my_malloc(size_t size)
{
    // Initilization
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
        is_initialized = true;
    }

    int order = get_order(size + sizeof(block_header_t));

    printf("Order %d \n", order);
    block_header_t *block = NULL;

    if (free_lists[order - MIN_ORDER] != NULL)
    { // Normal Case
        printf("Debug: free list available\n");
        block = free_lists[order - MIN_ORDER];
        unregister_freelist(block);

        block->is_free = false;
        return block;
    }

    if (free_lists[order - MIN_ORDER] == NULL) // Case free list not available
    {
        printf("Debug: free list not available\n");
        int i = order - MIN_ORDER;

        while (i < NUM_FREE_LISTS && free_lists[i] == NULL)
        {
            i++;
        }

        if (i >= NUM_FREE_LISTS)
        {
            printf("cannot find block\n");
            return NULL;
        }

        block = free_lists[i];
        unregister_freelist(block);

        while (block->order > order)
        {
            block_header_t *buddy = split(block);
            register_freelist(buddy);
        }
        block->is_free = false;
        block->next = NULL;

        return block;
    }
    return NULL;
}

void my_free(void *ptr)
{
}

int main(int argc, char *argv[])
{
    // Example Usage
    printf("Buddy Allocator Test\n");
    printf("Size of block header: %zu bytes\n", sizeof(block_header_t));

    // Request memory
    printf("Allocating 100 bytes...\n");
    void *p1 = my_malloc(1024);
    if (p1)
    {
        printf("Successfully allocated 100 bytes at %p\n", p1);
    }
    else
    {
        printf("Failed to allocate 100 bytes.\n");
    }

    print_all_free_lists();

    // The my_free function is not yet implemented.
    // my_free(p1);
    // my_free(p2);
    // my_free(p3);

    return 0;
}