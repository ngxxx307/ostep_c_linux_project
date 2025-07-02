#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
    struct Node *prev;
    char *line;
} Node;

Node *create_node(Node *prev_node)
{
    Node *new_node = malloc(sizeof(Node));

    if (new_node == NULL)
    {
        perror("malloc failed");
        exit(1);
    }

    new_node->prev = prev_node;
    new_node->line = NULL;

    return new_node;
}

int main(int argc, char *argv[])
{
    FILE *source_fp = fopen(argv[1], "r");
    if (source_fp == NULL)
    {
        char buffer[100];
        sprintf(buffer, "cannot open file '%s'", argv[1]);
        perror(buffer);
        exit(1);
    }

    FILE *dest_fp = fopen(argv[2], "w");
    if (dest_fp == NULL)
    {
        char buffer[100];
        sprintf(buffer, "cannot open file '%s'", argv[2]);
        perror(buffer);
        exit(1);
    }

    Node *curr_node = NULL;
    size_t len = 0;
    ssize_t read;
    char *line = NULL;

    while ((read = getline(&line, &len, source_fp)) > 0)
    {
        Node *new_node = create_node(curr_node);
        new_node->line = strdup(line);
        curr_node = new_node;
    }
    fclose(source_fp);

    Node *copy;
    while (curr_node != NULL)
    {
        fprintf(dest_fp, "%s", curr_node->line);
        copy = curr_node;
        curr_node = curr_node->prev;
        free(copy->line);
        free(copy);
    }

    free(line);
    fclose(dest_fp);

    return 0;
}