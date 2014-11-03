#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tabletrie.h"
#include "tabletrie.c"


static void debug_print_table_name(unsigned char *table_str);
static void debug_print_table_name(unsigned char *table_str)
{
    unsigned int tmp_len;
    char *name = generate_raw(table_str, &tmp_len);
    printf("DEBUG_TABLE_NAME_IS='%s'\n", name);
    free(name); 
}

static void debug_print_table_names(struct ttrie__obj *tt);
static void debug_print_table_names(struct ttrie__obj *tt)
{
    printf("DEBUG: TABLE_LEN='%u'\n", tt->last_choice_i);
    for (int i=0; i<=tt->last_choice_i ;++i)
    {
        debug_print_table_name(tt->choice_tables[i].name);
    }
}

static void print_result_id(unsigned char result_id[5]);
static void print_result_id(unsigned char result_id[5])
{
    unsigned char table_type;
    unsigned int table_i;
    unsigned long long row_i;
    decode_id(result_id, &table_type, &table_i, &row_i);
    printf("Result_id:: table_type='%u', table_i='%u', row_i='%llu'\n", table_type, table_i, row_i);
}

//--------------------------------------------------------------
// open close
void test_open_close(void)
{
    printf("------------------------\n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'r');
    ttrie__close(ttp);
}

//--------------------------------------------------------------
// Test generate_lenstr, generate_raw
void test_str_lenstr(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_str_lenstr \n");
    char *null_string = "hello ";
    unsigned char len = strlen(null_string);
    unsigned char *len_str_result = generate_lenstr(null_string, len);
    printf("hello last_i-1:: '%u'\n", len_str_result[0]);
    printf("hello str:: '%s'\n", len_str_result+1);

    unsigned int res_len;
    char *raw = generate_raw(len_str_result, &res_len);
    printf("hello '%s'\n", raw);

    free(len_str_result);
    free(raw);
}

//--------------------------------------------------------------
// Test generate_lenstr with int
void test_int_lenstr(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_int_lenstr \n");
    int my_int = 24;
    unsigned char int_len = sizeof(my_int);
    unsigned char *int_result = generate_lenstr(&my_int, int_len);
    printf("int last_i-1:: '%u'\n", int_result[0]);

    unsigned int res_int_len;
    int *raw_int = generate_raw(int_result, &res_int_len);
    printf("int '%d'\n", *raw_int);
    printf("int_len '%d'\n", res_int_len);

    free(int_result);
    free(raw_int);
}

//--------------------------------------------------------------
// Test make_choice_tables

void test_make_choice_tables_read(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_make_choice_tables_read \n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'r');

    char *table_name = "abc";
    unsigned int table_name_len = 3;
    unsigned int table_number = find_add_table(ttp, table_name, table_name_len);
    printf("choice table '%u' '%u' \n", table_number, tt.last_choice_i);
    struct ttrie__choice_table *res_table = ttp->choice_tables + table_number;
    unsigned int len;
    char *name = generate_raw(res_table->name, &len);
    printf("name='%s'\n", name);
    free(name);
    unsigned int table_number1a = find_add_table(ttp, table_name, table_name_len);
    printf("choice table '%u' '%u'\n", table_number1a, tt.last_choice_i);  // SHould be the same


    char *table_name2 = "abxy";
    unsigned int table_name_len2 = 4;
    unsigned int table_number2 = find_add_table(ttp, table_name2, table_name_len2);
    printf("choice table '%u' '%u' \n", table_number2, tt.last_choice_i);
    struct ttrie__choice_table *res_table_abxy = ttp->choice_tables + table_number2;
    unsigned int len2;
    char *name2 = generate_raw(res_table_abxy->name, &len2);
    printf("name='%s'\n", name2);
    free(name2);
    

    char *table_name3 = "a";
    unsigned int table_name_len3 = 4;
    unsigned int table_number3 = find_add_table(ttp, table_name3, table_name_len3);
    printf("choice table '%u' '%u' \n", table_number3, tt.last_choice_i);

    struct ttrie__choice_table *res_table_a = ttp->choice_tables + table_number3;
    unsigned int len3;
    char *name3 = generate_raw(res_table_a->name, &len3);
    printf("name='%s'\n", name3);
    free(name3);


    printf("Checking in order\n");  // These should be in insertion order.
    debug_print_table_names(ttp);


    ttrie__close(ttp);
}


void test_make_choice_tables_write(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_make_choice_tables_write \n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'w');

    char *table_name = "abc";
    unsigned int table_name_len = 3;

    unsigned int table_number = find_add_table(ttp, table_name, table_name_len);
    printf("choice table '%u' '%u' \n", table_number, tt.last_choice_i);      // SHould be  pos 0

    struct ttrie__choice_table *res_table = ttp->choice_tables + table_number;
    unsigned int len;
    char *name = generate_raw(res_table->name, &len);
    printf("name='%s'\n", name);
    free(name);
    
    unsigned int table_number_clone = find_add_table(ttp, table_name, table_name_len);
    printf("choice table '%u' '%u'\n", table_number_clone, tt.last_choice_i);  // SHould be the same, pos 0

    char *table_name2 = "cexy";
    unsigned int table_name_len2 = 4;
    unsigned int table_number_cexy = find_add_table(ttp, table_name2, table_name_len2);
    printf("choice table '%u' '%u' \n", table_number_cexy, tt.last_choice_i);   // Should make a new table. pos 1
    printf("AAAAAAAAAAA, '%u'\n", tt.choice_tables[1].name[0]);

    struct ttrie__choice_table *res_table_cexy = ttp->choice_tables + table_number_cexy;
    unsigned int len2;
    char *name2 = generate_raw(res_table_cexy->name, &len2);
    printf("name='%s'\n", name2);
    free(name2);
    
    char *table_name3 = "bexy";
    unsigned int table_name_len3 = 4;
    unsigned int table_number_bexy = find_add_table(ttp, table_name3, table_name_len3);
    printf("choice table '%u' '%u' \n", table_number_bexy, tt.last_choice_i);   // Should make a new table.
    printf("AAAAAAAAAAA, '%u'\n", tt.choice_tables[1].name[0]);

    struct ttrie__choice_table *res_table_bexy = ttp->choice_tables + table_number_bexy;
    unsigned int len3;
    char *name3 = generate_raw(res_table_bexy->name, &len3);
    printf("name='%s'\n", name3);
    free(name3);

    printf("Checking in order\n");  // These should be sorted
    debug_print_table_names(ttp);


    ttrie__close(ttp);
}


void test_make_choice_table_write_append_row(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_make_choice_table_write_append_row \n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'w');

    char *table_name1 = "foo";
    unsigned int table_name_len1 = strlen(table_name1);
    unsigned char row_ids1[4][5] = {"00001", "00002", "00003", "00004"};
    unsigned char result_id_1[5];
    new_choice_row(ttp, table_name1, table_name_len1, row_ids1, result_id_1);
    print_result_id(result_id_1);

    char *table_name2 = "ba"; 
    unsigned int table_name_len2 = strlen(table_name2);
    unsigned char row_ids2[3][5] = {"00005", "00006", "00007"};
    unsigned char result_id_2[5];
    new_choice_row(ttp, table_name2, table_name_len2, row_ids2, result_id_2);
    print_result_id(result_id_2);

    // Both of these below are broken.

    char *table_name3 = "r"; 
    unsigned int table_name_len3 = strlen(table_name3);
    unsigned char row_ids3[2][5] = {"00007", "00008"};
    unsigned char result_id_3[5];
    new_choice_row(ttp, table_name3, table_name_len3, row_ids3, result_id_3);
    print_result_id(result_id_3);


    printf("Checking in order\n");  // These should be sorted
    debug_print_table_names(ttp);

    printf("Last one\n");

    unsigned char row_ids1a[4][5] = {"00009", "00010", "00011", "00012"};
    unsigned char result_id_1a[5];
    new_choice_row(ttp, table_name1, table_name_len1, row_ids1a, result_id_1a);
    print_result_id(result_id_1a);

    ttrie__close(ttp);
}


void test_new_terminator_node(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_new_terminator_node \n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'w');

    unsigned char result_id[5];
    unsigned long long value = 212;
    terminator_append(ttp, value, result_id);
    print_result_id(result_id);

    terminator_append(ttp, value, result_id);
    print_result_id(result_id);

    terminator_append(ttp, value, result_id);
    print_result_id(result_id);

    ttrie__close(ttp);
}


void test_new_passthrough_node(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_new_passthrough_node \n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'w');

    unsigned char result_id[5];
    unsigned long long value = 212;
    unsigned char child_id[5] = "1234";

    passthrough_append(ttp, child_id, value, result_id);
    print_result_id(result_id);

    passthrough_append(ttp, child_id, value, result_id);
    print_result_id(result_id);

    passthrough_append(ttp, child_id, value, result_id);
    print_result_id(result_id);

    ttrie__close(ttp);
}


void test_new_suffix_node(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_new_suffix_node \n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'w');

    unsigned char result_id[5];
    unsigned long long value = 212;
    unsigned char suffix[] = "3141519";
    unsigned char len = strlen((char *) suffix);

    suffix_append(ttp, suffix, len, value, result_id);
    print_result_id(result_id);

    suffix_append(ttp, suffix, len, value, result_id);
    print_result_id(result_id);

    suffix_append(ttp, suffix, len, value, result_id);
    print_result_id(result_id);

    suffix_append(ttp, suffix, len, value, result_id);
    print_result_id(result_id);

    ttrie__close(ttp);
}




void test_new_infix_node(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_new_infix_node \n");
    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'w');

    unsigned char result_id[5];
    unsigned char child_id[5] = "1234";
    unsigned char infix[] = "3141519";
    unsigned char len = strlen((char *) infix);

    infix_append(ttp, infix, len, child_id, result_id);
    print_result_id(result_id);

    infix_append(ttp, infix, len, child_id, result_id);
    print_result_id(result_id);

    infix_append(ttp, infix, len, child_id, result_id);
    print_result_id(result_id);

    ttrie__close(ttp);
}


static char *STRINGS[] = {"abc", "abcd", "erm", "abc", "erq", "x123456789"};
static unsigned int STRING_LENS[] = {3, 4, 3, 3, 3, 10};              // Done so we can test a null in middle string easy.


void test_conversion(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_conversion \n");


    printf("Generating base trie\n");

    struct rtrie__view view;
    struct rtrie__node *n;    

    //printf("-----------Opening trie--------------\n");
    if (rtrie__open(&view, "trees"))
    {
        printf("ERROR: opening");
        return;
    }
    
    //printf("-----------Adding elements to trie--------------\n");
    int num_elements = sizeof(STRINGS) / sizeof(STRINGS[0]);
    for (int i=0; i<num_elements; ++i)
    {
        //printf("Adding %s, %d\n", STRINGS[i], i);
        n = rtrie__add(&view, STRINGS[i], STRING_LENS[i]);
        if (n==NULL)
        {
            printf("ERROR: adding");
            return;
        }
        n->result = i;
    }

    printf("Opening table trie\n");

    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;
    ttrie__open(ttp, 'w');

    ttrie__convert(ttp, &view);

    printf("Finished convert \n");
    print_result_id(ttp->root_id);

    unsigned char root_table_type_p;
    unsigned int root_table_i_p;
    unsigned long long root_row_i_p;
    decode_id(ttp->root_id, &root_table_type_p, &root_table_i_p, &root_row_i_p);

    ttrie__close(ttp);

    printf("closing trie\n");
    if (rtrie__close(&view))
    {
        printf("ERROR: closing");
        exit(EXIT_FAILURE);
    }


}

static char *SEARCH_STRINGS[] = {"erm", "not_there", "erq", "abc", "abcd", "x123456789"};
static unsigned int SEARCH_STRING_LENS[] = {3, 9, 3, 3, 4, 10};


void test_get(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_get \n");
    printf("Generating base trie\n");

    struct rtrie__view view;
    struct rtrie__node *n;    
    if (rtrie__open(&view, "trees"))
    {
        printf("ERROR: opening");
        return;
    }
    int num_elements = sizeof(STRINGS) / sizeof(STRINGS[0]);
    for (int i=0; i<num_elements; ++i)
    {
        n = rtrie__add(&view, STRINGS[i], STRING_LENS[i]);
        if (n==NULL)
        {
            printf("ERROR: adding");
            return;
        }
        n->result = i;
    }
    printf("Opening table trie\n");

    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;

    ttrie__open(ttp, 'w');
    ttrie__convert(ttp, &view);

    printf("Finished convert----------------------------------------- \n");

    unsigned long long result;
    int num_search_strings = sizeof(SEARCH_STRINGS) / sizeof(SEARCH_STRINGS[0]);
    for (int i=0; i<num_search_strings; ++i)
    {
        printf("<<<<<<<<<<<Looking for key %s >>>>>>>>>>>>>>>>>\n", SEARCH_STRINGS[i]);
        int error = ttrie__get(ttp, SEARCH_STRINGS[i], SEARCH_STRING_LENS[i], &result);
        if (error)
        {
            printf("\tError can't find key '%s' \n", SEARCH_STRINGS[i]);
        }
        else
        {
            printf("\tkey='%s', result='%llu'\n", SEARCH_STRINGS[i], result);
        }
    }

    ttrie__dprint(ttp);
    ttrie__close(ttp);

    printf("closing trie\n");
    if (rtrie__close(&view))
    {
        printf("ERROR: closing");
        exit(EXIT_FAILURE);
    }


}

void test_write_read(void)
{
    printf("------------------------\n");
    printf("TESTING:: test_write_read \n");
    //printf("Generating base trie\n");

    struct rtrie__view view;
    struct rtrie__node *n;    
    if (rtrie__open(&view, "trees"))
    {
        printf("ERROR: opening");
        return;
    }
    int num_elements = sizeof(STRINGS) / sizeof(STRINGS[0]);
    for (int i=0; i<num_elements; ++i)
    {
        n = rtrie__add(&view, STRINGS[i], STRING_LENS[i]);
        if (n==NULL)
        {
            printf("ERROR: adding");
            return;
        }
        n->result = i;
    }
    //printf("Opening table trie\n");

    struct ttrie__obj tt;
    struct ttrie__obj *ttp = &tt;

    ttrie__open(ttp, 'w');
    ttrie__convert(ttp, &view);

    //printf("Finished convert----------------------------------------- \n");
    char *file_path = "foobar.ttrie";

    printf("Current state \n");
    ttrie__dprint(ttp);

    printf("Writing to file %s \n", file_path);
    ttrie__write(ttp, file_path);

    printf("Done writing\n");
    ttrie__close(ttp);


    printf("Opening new read trie\n");
    struct ttrie__obj tt2;
    struct ttrie__obj *ttp2 = &tt2;
    ttrie__open(ttp2, 'r');

    printf("Reading from file %s \n", file_path);
    ttrie__read(ttp2, file_path);
    printf("Done reading\n");

    ttrie__dprint(ttp2);
    ttrie__close(ttp2);

    printf("closing trie\n");
    if (rtrie__close(&view))
    {
        printf("ERROR: closing");
        exit(EXIT_FAILURE);
    }

}




int main()
{
    test_open_close();
    test_str_lenstr();
    test_int_lenstr();

    test_make_choice_tables_read();
    test_make_choice_tables_write();
    test_make_choice_table_write_append_row();

    test_new_terminator_node();
    test_new_passthrough_node();
    test_new_suffix_node();
    test_new_infix_node();

    test_conversion();
    test_get();

    test_write_read();

    return 0;
}
