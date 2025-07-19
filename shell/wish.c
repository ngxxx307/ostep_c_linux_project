#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char **format_args(char *line, int *argc_ptr)
{
    char *line_copy = strdup(line);
    if (line_copy == NULL)
    {
        perror("strdup failed");
        return NULL;
    }
    char *line_ptr = line_copy;

    int size = 64;
    char **argv;
    char *token;

    argv = malloc(size * sizeof(char *));
    if (argv == NULL)
    {
        perror("malloc failed");
        free(line_copy);
        return NULL;
    }

    int count = 0;
    while ((token = strsep(&line_ptr, " ")) != NULL)
    {
        if (strlen(token) == 0)
        {
            continue;
        }

        if (count >= size - 1)
        {
            size = size * 2;
            char **new_argv = realloc(argv, sizeof(char *) * size);
            if (new_argv == NULL)
            {
                perror("realloc failed");
                free(argv);
                free(line_copy);
                return NULL;
            }
            argv = new_argv;
        }
        argv[count] = strdup(token);
        count++;
    }

    argv[count] = NULL;

    *argc_ptr = count;

    free(line_copy);
    return argv;
}

void free_argv(char **argv)
{
    if (argv == NULL)
    {
        return;
    }
    for (int i = 0; argv[i] != NULL; i++)
    {
        free(argv[i]);
    }
    free(argv);
}

void change_directory(char *argv[])
{
    if (chdir(argv[1]) != 0)
    {
        perror("error change directory: ");
    }
    return;
}

void redirect(char *argv[], int argc)
{
    // Look for ">" in the arguments
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            if (i + 1 < argc) // Make sure there's a filename after ">"
            {
                char *output_file = argv[i + 1]; // Get output file

                // Set up redirection
                if (freopen(output_file, "w", stdout) == NULL)
                {
                    perror("freopen failed");
                    exit(1);
                }

                argv[i] = NULL; // Terminate argv at the ">" symbol
                return;
            }
        }
    }
}

void process_line(char *line, char ***paths_ptr)
{
    char **argv;
    int argc;
    argv = format_args(line, &argc);

    // Handle redirection (this modifies argv and sets up file redirection)
    redirect(argv, argc);

    char program1[100];
    char program2[100];
    sprintf(program1, "%s%s", (*paths_ptr)[0], argv[0]);
    sprintf(program2, "%s%s", (*paths_ptr)[1], argv[0]);

    if (access(program1, X_OK) != -1)
    {
        execv(program1, argv);
    }
    else if (access(program2, X_OK) != -1)
    {
        execv(program2, argv);
    }
    else
    {
        printf("Program not found: %s\n", argv[0]);
    }

    free_argv(argv);
    exit(0);
}

char **get_tokens(char *line, int *count_ptr)
{
    char *str_copy = strdup(line);

    char *token = strtok(str_copy, "&");

    while (token != NULL)
    {
        *count_ptr = *count_ptr + 1;
        token = strtok(NULL, "&");
    }

    free(str_copy);

    char **tokens = malloc((*count_ptr + 1) * sizeof(char *));

    if (tokens == NULL)
    {
        perror("malloc failed");
        return NULL;
    }

    int i = 0;

    token = strtok(line, "&");
    while (token != NULL)
    {
        tokens[i] = token;
        token = strtok(NULL, "&");
        i++;
    }
    tokens[i] = NULL;
    return tokens;
}

int main(int argc, char *argv[])
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char *paths[] = {"/bin/", "/usr/bin/"};
    char **paths_ptr = paths;

    while (1)
    {
        printf("wish> ");
        read = getline(&line, &len, stdin);

        if (read == -1)
        {
            break;
        }

        // Remove trailing newline if present
        if (read > 0 && line[read - 1] == '\n')
        {
            line[read - 1] = '\0';
        }

        int count = 0;
        char **tokens = get_tokens(line, &count);

        char **argv;
        int argc;
        argv = format_args(line, &argc);

        int start = 0;
        if (argc > 0)
        {
            if (strcmp(argv[0], "exit") == 0)
            {
                exit(0);
            }
            else if (strcmp(argv[0], "cd") == 0)
            {
                change_directory(argv);
                start++;
            }
            else if (strcmp(argv[0], "path") == 0)
            {
                paths_ptr = &argv[1];
                start++;
            }
        }

        // Process each command separated by &
        for (int i = start; i < count; i++)
        {
            int rc = fork();

            if (rc < 0)
            {
                perror("fork error");
                exit(1);
            }
            else if (rc == 0)
            {
                process_line(tokens[i], &paths_ptr);
                exit(0);
            }
        }

        // Wait for all child processes to complete
        for (int i = 0; i < count; i++)
        {
            wait(NULL);
        }

        if (argc > 0 && strcmp(argv[0], "path") != 0)
        {
            free_argv(argv);
        }
        else if (argc == 0)
        {
            free(argv);
        }
        free(tokens);
    }
    free(line);

    return 0;
}