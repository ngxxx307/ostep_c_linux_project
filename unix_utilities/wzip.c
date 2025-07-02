#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char *source_file = argv[1];
    char *dest_file = argv[2];

    if (source_file == NULL || dest_file == NULL)
    {
        perror("wzip: file1 [file2 ...]");
        exit(1);
    }
    int count = 1;
    int prev_c;
    int c;

    FILE *source_fp = fopen(source_file, "r");
    FILE *dest_fp = fopen(dest_file, "wb");

    if (source_fp == NULL || dest_fp == NULL)
    {
        perror("wzip: cannot open file");
        exit(1);
    }

    if ((prev_c = fgetc(source_fp)) == EOF)
    {
        fclose(source_fp);
        fclose(dest_fp);
        return 0;
    }

    while ((c = fgetc(source_fp)) != EOF)
    {
        if (prev_c != c)
        {
            fwrite(&count, sizeof(int), 1, dest_fp);
            fwrite(&prev_c, sizeof(char), 1, dest_fp);
            prev_c = c;
            count = 1;
            continue;
        }
        count++;
    }

    fwrite(&count, sizeof(int), 1, dest_fp);
    fwrite(&prev_c, sizeof(char), 1, dest_fp);

    fclose(source_fp);
    fclose(dest_fp);

    return 0;
}