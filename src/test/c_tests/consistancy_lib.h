#include <stdio.h>     // for printf
#include <string.h>    // for memcpy, strlen
#include <assert.h>    // for assert
#include <unistd.h>   // for access
#include <stdlib.h>    // for exit/EXIT_FAILURE
#include <stdbool.h>   // for bool
#include <stdarg.h>   // va_start, va_arg etc

void error(char *msg, ...);
void error(char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    printf("Error::");
    vprintf(msg, args);
    printf("\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

//line becomes key, line+i becomes value.
signed int inplace_split_key_val(unsigned char *line)
{
    int i = 0;
    while (line[i]!='\0')
    {
        if (line[i] == ' ')
        {
            line[i] = '\0';
            return i;
        }
        i++;
    }
    return -1;
}

static unsigned char data[256];
static int val;
int parse_key_value(char *line, int n, unsigned char **key, int **value)
{
    memcpy(data, line, n);
    data[n-1] = '\0';
    int i = inplace_split_key_val(data);
    if (i == -1)
        error("Failed to split key_val\n");
    
    *key = data;
    val = atoi((char *)data+i);
    *value = &val;
    return i;
}

void print_usage(void)
{
    printf("Required_args:: -i={input_path} -m={missing_path} -e={expected_path}\n");
}

void check_file_exists(char *file_path)
{
    if (access(file_path, F_OK)!=0)
        error("File path invalid='%s'\n", file_path);
}

int arg_parser(int argc, char *argv[], char *input_path, char *missing_path, char *expected_path);
int arg_parser(int argc, char *argv[], char *input_path, char *missing_path, char *expected_path)
{

    if (argc != 4)
    {
        print_usage();
        error("Wrong number of options ='%d'. require 3\n", argc-1);
    }

    bool has_input = false;
    bool has_missing = false;
    bool has_results = false;
    for (int i=1; i<argc; ++i)
    {
        char *arg_string = argv[i];

        int arg_str_len = strlen(arg_string);
        if (arg_str_len<= 3)
            error("Arg string too short bad_argstring='%s'\n", arg_string);
        if (arg_string[0] != '-')
            error("All arg strings should start with -, bad_argstring='%s'\n", arg_string);
        if (arg_string[2] != '=')
            error("Missing = in arg string  -option=value, bad_argstring='%s'\n", arg_string);

        char second_letter = arg_string[1];
        switch (second_letter)
        {
            case 'i':
            {
                if (has_input==true)
                    error("Repeated arg -i");  

                has_input = true;
                memcpy(input_path, arg_string+3, arg_str_len-2);
                check_file_exists(input_path);
                break;
            }
            case 'm':
            {
                if (has_missing==true)
                    error("Repeated arg -m");  

                has_missing = true;
                memcpy(missing_path, arg_string+3, arg_str_len-2);
                check_file_exists(missing_path);
                break;
            }
            case 'e':
            {
                if (has_results==true)
                    error("Repeated arg -r");  

                has_results = true;
                memcpy(expected_path, arg_string+3, arg_str_len-2);
                check_file_exists(expected_path);
                break;
            }
            default:
            {
                print_usage();
                error("I don't understand the option ='-%c'\n", second_letter);
                break;
            }
        }
    }
    return 0;
}

/*
int main(int argc, char *argv[])
{
    char input_path[256];
    char missing_path[256];
    char expected_path[256];
    arg_parser(argc, argv, input_path, missing_path, expected_path);
    printf("input_path='%s'\n", input_path);
    printf("missing_path='%s'\n", missing_path);
    printf("expected_path='%s'\n", expected_path);
}
*/