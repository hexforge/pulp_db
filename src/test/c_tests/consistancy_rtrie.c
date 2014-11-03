#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "rtrie.h"
#include "consistancy_lib.h"

void populate_rtrie(char *input_path, struct rtrie__view view)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned char *key;
    int *val;
    struct rtrie__node *n = NULL;

    FILE *fp_input = fopen(input_path, "r");
    if (fp_input == NULL)
        exit(EXIT_FAILURE);

    int count = 0;
    while ((read = getline(&line, &len, fp_input)) != -1) 
    {
        int key_size = parse_key_value(line, read, &key, &val);

        //printf("adding line '%s'\n", line);
        bool new_entry = false;
        n = rtrie__add(&view, key, key_size, &new_entry);
        if (n==NULL)
          error("ERROR: adding");
        n->result = *val;
        count++;
    }
    fclose(fp_input);
    free(line);
}

int iterate_rtrie(char *expected_path, struct rtrie__view view)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned char *expected_key;
    int *expected_val;
    int num_checks = 0;
    struct rtrie__node *n = NULL;

    FILE *fp_expected = fopen(expected_path, "r");
    if (fp_expected == NULL)
        exit(EXIT_FAILURE);

    int len_tree = rtrie__len(&view);
    rtrie__iter(&view);
    
    unsigned char actual_key[STACK_SIZE];
    int j=0;
    while ((read = getline(&line, &len, fp_expected)) != -1) 
    {
        int key_size = parse_key_value(line, read, &expected_key, &expected_val);

        int actual_keylen;
        n = rtrie__next(&view);
        if (n==NULL)
          error("ERROR: next");
        rtrie__fullkey(&view, actual_key, &actual_keylen);
        actual_key[actual_keylen] = '\0';

        //printf("Got element %d, actual_key='%s', expected_key='%s' actual_result='%u', expected_result='%u'\n", j, actual_key, expected_key, n->result, *expected_val);
        assert(memcmp(actual_key, expected_key, actual_keylen)==0);
        assert(*expected_val==n->result);
        j++;
        num_checks += 1;
    }
    return num_checks;
}

int check_expected(char *expected_path, struct rtrie__view view)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned char *key;
    int *val;
    int num_checks = 0;
    struct rtrie__node *n = NULL;

    FILE *fp_expected = fopen(expected_path, "r");
    if (fp_expected == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp_expected)) != -1) 
    {
        int key_size = parse_key_value(line, read, &key, &val);

        n = rtrie__get(&view, key, key_size);
        if (n==NULL)
            error("ERROR: getting %s\n", line);
        //printf("Got key'%s' actual_val=%u expected_val=%u \n", key, n->result, value);
        assert(n->result==*val);
        num_checks += 1;
    }
    fclose(fp_expected);
    free(line);
    return num_checks;
}

int check_missing(char *missing_path, struct rtrie__view view)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int num_checks = 0;
    struct rtrie__node *n = NULL;

    FILE *fp_missing = fopen(missing_path, "r");
    if (fp_missing == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp_missing)) != -1) 
    {
        line[read-1] = '\0';
        n = rtrie__get(&view, line, len);
        //printf("Checking missing for %s\n", line);
        assert(n==NULL);
        num_checks += 1;
    }
    fclose(fp_missing);
    free(line);
    return num_checks;
}

int main(int argc, char *argv[])
{
    char input_path[256];
    char missing_path[256];
    char expected_path[256];
    arg_parser(argc, argv, input_path, missing_path, expected_path);
    printf("\tinput_path='%s'\n", input_path);
    printf("\tmissing_path='%s'\n", missing_path);
    printf("\texpected_path='%s'\n", expected_path);

    printf("\tMaking rtrie--------------\n");
    struct rtrie__view view;
    if (rtrie__open(&view))
        error("ERROR: opening rtrie");

    printf("\tpopulating rtrie--------------\n");
    populate_rtrie(input_path, view);

    int num_checks = 0;
    printf("\titerating rtrie--------------\n");
    num_checks += iterate_rtrie(expected_path, view);

    printf("\tchecking expected--------------\n");
    num_checks += check_expected(expected_path, view);

    printf("\tchecking missing--------------\n");
    num_checks += check_missing(missing_path, view);

    printf("\tClosing down--------------\n");
    
    if (rtrie__close(&view))
        error("ERROR: closing");

    printf("expected: %d tests executed. ALL PASSED\n", num_checks);
    return 0;
}

