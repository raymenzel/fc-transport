#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define cleanup(log1, log2) { \
    fclose(log1); \
    fclose(log2); \
    return EXIT_FAILURE; \
}

#define raise(code, log1, log2) { \
    if ((code) != 0) \
    { \
        cleanup(log1, log2); \
    } \
}


int get_tokens(char * c, char ** tokens, int max_token_length,
               int num_tokens, char separator)
{
    int j;
    for (j=0; j<num_tokens; ++j)
    {
        char * token = tokens[j];
        int i = 0;
        while (*c != separator)
        {
            if (*c == '\n')
            {
                if (i == 0)
                {
                    fprintf(stderr, "Error: empty token.\n");
                    return 1;
                }
                if (j != num_tokens - 1)
                {
                    fprintf(stderr, "Error: found end of line before all tokens.\n");
                    return 1;
                }
                break;
            }
            if (i >= max_token_length)
            {
                fprintf(stderr, "Error: cannot parse line.\n");
                return 1;
            }
            token[i] = *c;
            c++;
            i++;
        }
        if (i == 0)
        {
            fprintf(stderr, "Error: empty token.\n");
            return 1;
        }
        token[i] = '\0'; /* Completed token.*/
        c++;
    }
    return 0;
}


int compare_floats(char * token1, char * token2, int line_num)
{
    float value1 = atof(token1);
    float value2 = atof(token2);
    float tolerance = 1.e-5;
    if (fabs(value1 - value2) > tolerance)
    {
        fprintf(stderr, "Error: floating point difference (%e != %e) on line: %d.\n",
                value1, value2, line_num);
        return 1;
    }
    return 0;
}


int main(int argc, char ** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s log1 log2\n", argv[0]);
        return EXIT_SUCCESS;
    }

    FILE * log1 = fopen(argv[1], "r");
    if (log1 == NULL)
    {
        fprintf(stderr, "Error: could not open log %s.\n", argv[1]);
    }

    FILE * log2 = fopen(argv[2], "r");
    if (log1 == NULL)
    {
        fprintf(stderr, "Error: could not open log %s.\n", argv[1]);
    }

    char line_a[256];
    char line_b[256];
    int num_lines = 0;
    char * tokens_a[4];
    char * tokens_b[4];
    int i;
    for (i=0; i<4; ++i)
    {
        tokens_a[i] = malloc(sizeof(char)*256);
        memset(tokens_a[i], 0, sizeof(char)*256);
        tokens_b[i] = malloc(sizeof(char)*256);
        memset(tokens_b[i], 0, sizeof(char)*256);
    }

    while(fgets(line_a, 256, log1) != NULL && fgets(line_b, 256, log2) != NULL)
    {
        if (line_a == NULL || line_b == NULL)
        {
            fprintf(stderr, "Error: the log files have different numbers of lines.\n");
            cleanup(log1, log2);
        }

        num_lines++;
        char * character_a = line_a;
        int code = get_tokens(character_a, tokens_a, 256, 4, ',');
        raise(code, log1, log2);
        char * character_b = line_b;
        code = get_tokens(character_b, tokens_b, 256, 4, ',');
        raise(code, log1, log2);
        for (i=0; i<4; ++i)
        {
            code = compare_floats(tokens_a[i], tokens_b[i], num_lines);
            raise(code, log1, log2);
        }
    }
    fclose(log1);
    fclose(log2);
    for (i=0; i<4; ++i)
    {
        free(tokens_a[i]);
        free(tokens_b[i]);
    }
    return EXIT_SUCCESS;
}
