#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "rtrie.h"
#include "ttrie.h"
#include "consistancy_lib.h"

void populate_rtrie(char *input_path, struct rtrie__view view)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned char *key;
    int *val;
    struct rtrie__node *n;

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

int check_iteration(char *expected_path, struct ttrie__obj *ttp)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned char *key;
    int *val;
    int num_checks = 0;

    FILE *fp_iteration = fopen(expected_path, "r");
    if (fp_iteration == NULL)
        exit(EXIT_FAILURE);

    struct ttrie__iterator iter;
    ttrie__iter(&iter, ttp);
    unsigned long long *actual_value;
    int actual_key_len;
    char actual_key[256];

    while ((read = getline(&line, &len, fp_iteration)) != -1) 
    {
        int key_size = parse_key_value(line, read, &key, &val);
        //printf("looking for value\n");
        int rc = ttrie__next_node(&iter, actual_key, &actual_key_len, &actual_value);
        assert (rc == 0);  // Else we have early termination.
        //printf("%u, %u\n", actual_key_len, key_size);
        //printf("Got actual_key'%.*s', expected_key '%.*s\n'", actual_key_len, actual_key, key_size, key);
        assert(memcmp(actual_key, key, key_size) == 0);
        //printf("actual_val=%llu expected_val=%u \n", *actual_value, *val);
        assert(*actual_value == *val);
        num_checks += 1;
    }
    int rc = ttrie__next_node(&iter, actual_key, &actual_key_len, &actual_value);
    assert (rc != 0);  // Else we have early termination.

    ttrie__close_iter(&iter);
    fclose(fp_iteration);
    free(line);
    return num_checks;
}

int check_expected(char *expected_path, struct ttrie__obj *ttp)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned char *key;
    int *val;
    int num_checks = 0;
    FILE *fp_expected = fopen(expected_path, "r");
    if (fp_expected == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp_expected)) != -1) 
    {
        int key_size = parse_key_value(line, read, &key, &val);

        unsigned long long result;
        int rc = ttrie__get(ttp, key, key_size, &result);
        
        //printf("Got key'%s' actual_val=%llu expected_val=%u \n", key, result, *val);
        assert (rc == 0);
        assert(result==*val);
        num_checks += 1;
    }
    fclose(fp_expected);
    free(line);
    return num_checks;
}

int check_missing(char *missing_path, struct ttrie__obj *ttp)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int num_checks = 0;

    FILE *fp_missing = fopen(missing_path, "r");
    if (fp_missing == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp_missing)) != -1) 
    {
        line[read-1] = '\0';
        unsigned long long result;
        int rc = ttrie__get(ttp, line, read, &result);
        //printf("Checking missing for %s %d\n", line, rc);
        assert(rc!=0);
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

    printf("\tConverting to ttrie--------------\n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp_write = &tt;
    ttrie__open(ttp_write, 'w');
    ttrie__convert(ttp_write, &view);

    //ttrie__print_root(ttp_write);

    int num_checks = 0;

    printf("\tchecking just written iterates as expected--------------\n");
    num_checks += check_iteration(expected_path, ttp_write);

    printf("\tchecking just written expected--------------\n");
    num_checks += check_expected(expected_path, ttp_write);

    printf("\tchecking missing--------------\n");
    num_checks += check_missing(missing_path, ttp_write);

    printf("\tWritting ttrie to disk--------------\n");
    char *ttrie_file_path = "foo.ttrie";
    ttrie__write(ttp_write, ttrie_file_path);

    printf("\tClosing down write__ttrie--------------\n");
    if (rtrie__close(&view))
        error("ERROR: closing");
    ttrie__close(ttp_write);

    // READ
    printf("\tOpening read__ttrie--------------\n");
    struct ttrie__obj tt_read;
    struct ttrie__obj *ttp_read = &tt_read;
    ttrie__open(ttp_read, 'r');

    //ttrie__dprint(ttp_read);

    printf("\tPopulating read__ttrie--------------\n");
    ttrie__read(ttp_read, ttrie_file_path);

    printf("\tchecking read iterates as expected--------------\n");
    num_checks += check_iteration(expected_path, ttp_read);

    printf("\tchecking read ttrie--------------\n");
    num_checks += check_expected(expected_path, ttp_read);

    printf("\tchecking missing keys read ttrie--------------\n");
    num_checks += check_missing(missing_path, ttp_read);

    printf("\tClosing down read__ttrie--------------\n");
    ttrie__close(ttp_read);

    printf("expected: %d tests executed. ALL PASSED\n", num_checks);
    return 0;
}
