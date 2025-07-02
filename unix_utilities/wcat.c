#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int buffer_size = 100;
    char buffer[buffer_size];

    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL)
        {
            sprintf(buffer, "cannot open file '%s'", argv[i]);
            perror(buffer);
            exit(1);
        }

        while (fgets(buffer, buffer_size, fp) != NULL)
        {
            printf("%s", buffer);
        }

        fclose(fp);
    }

    return 0;
}