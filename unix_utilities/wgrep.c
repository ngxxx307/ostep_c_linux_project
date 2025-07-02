#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char *filename = argv[1];
    char *search_string = argv[2];

    if (search_string == NULL)
    {
        printf("%s", "wgrep: searchterm [file ...]\n");
        exit(1);
    }

    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        char error_msg[256];
        sprintf(error_msg, "wgrep: cannot open file '%s'", filename);
        perror(error_msg);
        exit(1);
    }

    ssize_t read;
    size_t len = 0;
    char *line = NULL;

    while ((read = getline(&line, &len, fp)) > 0)
    {
        if (strstr(line, search_string) != NULL)
        {
            printf("%s", line);
        }
    }
    fclose(fp);

    return 0;
}