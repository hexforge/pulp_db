#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "tabletrie.h"
#include "mmbuf.h"

//******************IDEAS
//  Can generate from bottom up and delaocate orig trie as we go to save memory :D
// We could reuse string allocations from orig trie
// 

// Markup data store.  Passthrough flatfile or Consume.

// write once, read many.  datastore
// Double pass self optiziming storage.
// For jagged data.  Indexing.
// Optimized for lookup, insertion in possible
// Persistable

// Need todo all the lookup methods
// Serialise and deserialse for each table type.

// We chave a problem with the ordering time and space of the choice table names
// Need to retain insertion order and be fast searchable
// One improvement is it to have them is sperate lists for each length of the table name
// With equal length names, we know the possition of each as it same bytes. This saves us pointers to strings
// As we have shorter lists we would not need to move so much to reorder.



// First we figure out how to allocate.
// First need to know name and number of rows to allocate for each table name.
// We can make the map.


/*
node has value and no children ---> terminator
node has value and children ---> passthrough

just one child
       if    continues with one child without value untill termination  -----> suffix terminator
       else  continues and either gets many children or gets a value  -----> shortcut

more than one child --->  new choice table
                            get the row and iterate
*/

// Then we need to reiterate the trie. 

static bool power_of_two_or_zero(unsigned int i);
static bool power_of_two_or_zero(unsigned int i)
{
    // Including zero as we want to expand then
    if (((i) & (i - 1)) == 0)   // See exercise 2.9 K&R
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void decode_id(const unsigned char id[5], unsigned char *table_type, unsigned int *table_i, unsigned long long *row_i);
static void decode_id(const unsigned char id[5], unsigned char *table_type, unsigned int *table_i, unsigned long long *row_i)
{
    if ((id[0] & 128) == 0)  //First bit defines if special table
    {
        *table_type = 0;
        *row_i = id[4];
        *table_i = 0;
        *table_i += ((unsigned int) id[0] << 24);
        *table_i += ((unsigned int) id[1] << 16);
        *table_i += ((unsigned int) id[2] << 8);
        *table_i += ((unsigned int) id[3]);
    }
    else
    {
        *table_type = ((id[0] & 96) >> 5) + 1;   
        *table_i = 0;

        unsigned long long row_number = 0;
        row_number += id[4];
        row_number += ((unsigned long long) id[3]) << 8;
        row_number += ((unsigned long long) id[2]) << 16;
        row_number += ((unsigned long long) id[1]) << 24;
        row_number += ((unsigned long long) id[0] & ~224) << 32;
        *row_i = row_number;
    }
}



static void encode_id(unsigned char table_type, unsigned int table_i, unsigned long long row_i, unsigned char id[5]);
static void encode_id(unsigned char table_type, unsigned int table_i, unsigned long long row_i, unsigned char id[5])
{
    if (table_type==CHOICE_TABLE)
    {        
        id[0] = (table_i >> 24) & 127;
        id[1] = (table_i >> 16) & 255;
        id[2] = (table_i >> 8) & 255;
        id[3] = (table_i & 255); 
        id[4] = (unsigned char) row_i;
    }
    else
    {
        id[0] = ((table_type -1) << 5) + 128;      // table_type -1 is two bits, BIT_SPECIAL, BIT_TABLE_TYPE1, BIT_TABLE_TYPE2, OTHERBITS.....
        id[0] += (row_i >> 32) & ~224;             //(128+64 + 32)
        id[1] = (row_i >> 24) & 255;
        id[2] = (row_i >> 16) & 255;
        id[3] = (row_i >> 8) & 255;
        id[4] = row_i & 255;
    }
}

static unsigned char *generate_lenstr(void *str, unsigned int str_len);
static unsigned char *generate_lenstr(void *str, unsigned int str_len)
{
    unsigned char *len_str = malloc(sizeof(unsigned char[str_len + 1]));
    assert (str_len <= 256);
    len_str[0] = str_len -1;                // Extra char, [0] element stores last_i -1.
    memcpy(len_str+1, str, str_len);
    return len_str;
}

static void *generate_raw(unsigned char *len_str, unsigned int *len_p);
static void *generate_raw(unsigned char *len_str, unsigned int *len_p)
{
    unsigned char len = len_str[0] + 1;
    unsigned char *str = malloc(sizeof(unsigned char[len]));
    memcpy(str, len_str+1, len);
    *len_p = len;
    return str;
}

static int num_digits_lenstr(const unsigned char *len_str);
static int num_digits_lenstr(const unsigned char *len_str)
{
    return len_str[0]+2;
}


/**
 *   CHOICE
*/
static unsigned int linearsearch_choice_tables(const struct ttrie__choice_table *choice_tables, const unsigned int from, const unsigned int last_i, const unsigned char *table_name);
static unsigned int linearsearch_choice_tables(const struct ttrie__choice_table *choice_tables, const unsigned int from, const unsigned int last_i, const unsigned char *table_name)
{
    for (int i=from; i<=last_i; ++i)
    {
        if (memcmp(table_name, choice_tables[i].name, num_digits_lenstr(table_name)) == 0)
        {
            return i;
        }
    }
    return last_i + 1;
}

static int get_choice_table_width(struct ttrie__choice_table *ct);
static int get_choice_table_width(struct ttrie__choice_table *ct)
{
    if (ct->name == NULL)
    {
        return 0;
    }
    return ct->name[0] + 1;
}

static int get_choice_table_row_count(struct ttrie__choice_table *ct);
static int get_choice_table_row_count(struct ttrie__choice_table *ct)
{
    if (ct->ids == NULL)
    {
        return 0;
    }
    return ct->ids[0] + 1;
}

static bool choice_table_full(struct ttrie__choice_table *ct);
static bool choice_table_full(struct ttrie__choice_table *ct)
{
    int depth = get_choice_table_row_count(ct);
    if (depth == MAX_CHOICE_TABLE_DEPTH)
        return true;
    else
        return false;
}

static int resize_choice_tables(struct ttrie__obj *tt, unsigned int new_nitems);
static int resize_choice_tables(struct ttrie__obj *tt, unsigned int new_nitems)
{
    tt->choice_tables = realloc(tt->choice_tables, sizeof(struct ttrie__choice_table[new_nitems]));
    return 0;
}


static void insert_table(struct ttrie__obj *tt, unsigned char *table_str, unsigned int pos, bool auto_resize);
static void insert_table(struct ttrie__obj *tt, unsigned char *table_str, unsigned int pos, bool auto_resize)
{
    if (auto_resize && power_of_two_or_zero(tt->last_choice_i + 1))
    {
        if (tt->choice_tables == NULL)
        {
            resize_choice_tables(tt, 1);
            insert_table(tt, table_str, 0, 0);
            tt->last_choice_i = 0;
            return;
        }
        else
        {
            unsigned int old_nitems = tt->last_choice_i + 1;
            unsigned int new_nitems = old_nitems << 1;
            resize_choice_tables(tt, new_nitems);
        }
    }

    tt->choice_tables[pos].name = table_str;
    tt->choice_tables[pos].ids = NULL;
    
    if (pos > tt->last_choice_i)
        tt->last_choice_i = pos;



    if (tt->index_table_names)
    {       
        //if (pos==0)
        //{
        //    printf("!!!!!!!!!!!!!!!!!!!!\n");
        //}

        struct rtrie__node *n = rtrie__add(tt->table_names_index, table_str+1, table_str[0] + 1);        // [0] is size -2.   
        if (n==NULL)
        {
            printf("ERROR: adding\n");
            exit(EXIT_FAILURE);
        }
        n->result = pos;  //this must be wrong.

        //printf("In insert tables pos=%u, tt->last_choice_i=%u %p\n",pos, tt->last_choice_i, tt->choice_tables );
        //printf("sssssssssssssss %.*s %.*s \n", table_str[0] + 1, tt->choice_tables[pos].name+1, table_str[0] + 1, table_str+1);
        assert(tt->choice_tables != NULL);
        assert(memcmp(tt->choice_tables[pos].name+1, table_str+1, table_str[0] + 1)==0);

        //printf("Existing table len='%u' name='%.*s'  depth=%d\n", name_len, name_len, (char *)table_name, get_choice_table_row_count(tt->choice_tables +p));
        //unsigned int width = get_choice_table_width(tt->choice_tables + p);
        //printf("Found existing table at pos='%u', for name='%.*s', n_elements='%u'\n", p, width, tt->choice_tables[p].name+1, width);

    }


}

static unsigned int find_add_table(struct ttrie__obj *tt, void *table_name, unsigned int name_len);
static unsigned int find_add_table(struct ttrie__obj *tt, void *table_name, unsigned int name_len)
{
    unsigned char *table_str = generate_lenstr(table_name, name_len);   // This allocates memory.!!!!!!!!!!!!!!!
    //printf("searching for %u %.*s\n", name_len, name_len, (char *)table_name);

    if (tt->choice_tables == NULL)
    {
        insert_table(tt, table_str, 0, true);  // With auto_resize on this will sort us out.
        return 0;
    }

    /** 
     *  Here we decide what finder to use to find any tables with table_name
    **/
    unsigned long long p;
    if (tt->index_table_names) 
    {
        // This structure points to the last known non full table. If a table fills we make a new one and point to it.
        struct rtrie__node *n = rtrie__get(tt->table_names_index, table_str+1, table_str[0] + 1);

        if (n==NULL)
        {
            //printf("Not found: key \n");
            p = tt->last_choice_i + 1;
        }
        else
        {
            p = n->result;               
            if (choice_table_full(tt->choice_tables + p))
            {
                //printf("Table full added new one \n");
                p = tt->last_choice_i + 1;
            }
            else
            {
                //printf("Suggested pos %llu\n", p);
                //int existing_width = num_digits_lenstr(tt->choice_tables[p].name);
                //printf("Existing table len='%u' name='%.*s'  depth=%d\n", existing_width-1, existing_width-1, tt->choice_tables[p].name+1, get_choice_table_row_count(tt->choice_tables +p));
                //int width = num_digits_lenstr(table_str);
                //printf("Searched for len='%u' name='%.*s'\n", width-1, width-1, table_str+1);
                //printf("%llu, '%.*s' '%.*s'\n", p, num_digits_lenstr(table_str), tt->choice_tables[p].name, num_digits_lenstr(table_str), table_str);
                assert (memcmp(tt->choice_tables[p].name, table_str, num_digits_lenstr(table_str))==0);
            }
        }
        //printf("I am here\n");
    }
    else
    {
        bool found_empty = false;
        unsigned int start = 0;

        while (!found_empty && start <= tt->last_choice_i)
        {   // There can be many matches.  The last match should be the least full or the latest to be filled.
            p = linearsearch_choice_tables(tt->choice_tables, start, tt->last_choice_i, table_str);

            if (p == tt->last_choice_i+1)
                found_empty = true;
            else if (p > tt->last_choice_i+1)
            {
                printf("ERROR ERROR\n");
                exit(EXIT_FAILURE);
            }
            else if (!choice_table_full(tt->choice_tables + p))
                found_empty = true;
            else
                start = p + 1;
        }
        if (found_empty == false)
            p = tt->last_choice_i + 1;
    } 

    // Check if p should be appended.
    bool append = (p == tt->last_choice_i+1);
    bool error = (p > tt->last_choice_i+1);
    
    //printf("Debug table_name='%s' p=%d append=%u error=%u\n", (char *)table_name, p, append, error);

    if (append)
    {
        insert_table(tt, table_str, tt->last_choice_i+1, true);
        return p;
    }
    else if (error)
    {
        printf("ERROR: can only use this to append or inplace insertion\n");
        free(table_str);
        exit(EXIT_FAILURE);
    }
    else
    {
        //printf("Existing table len='%u' name='%.*s'  depth=%d\n", name_len, name_len, (char *)table_name, get_choice_table_row_count(tt->choice_tables +p));
        //unsigned int width = get_choice_table_width(tt->choice_tables + p);
        //printf("Found existing table at pos='%llu', for name='%.*s', n_elements='%u'\n", p, width, tt->choice_tables[p].name+1, width);
        assert (memcmp(tt->choice_tables[p].name, table_str, num_digits_lenstr(table_str))==0 && !choice_table_full(tt->choice_tables + p));
        free(table_str);
        return p;
    }
    printf("ERROR: how did I get here. P.S. clean up this flow.\n");
    free(table_str);
    exit(EXIT_FAILURE);
}

static int resize_choice_table(struct ttrie__choice_table *ct, unsigned long long new_nrows);
static int resize_choice_table(struct ttrie__choice_table *ct, unsigned long long new_nrows)
{
    int width;
    if ((width = get_choice_table_width(ct)) == 0)
    {
        printf("ERROR: choice table not inited\n");
        exit(EXIT_FAILURE);
    }

    ct->ids = realloc(ct->ids, sizeof(unsigned char [new_nrows*width*5 +1]));   // This looks wrong
    return 0;
}

static unsigned int choice_row_id_i(unsigned int row_i, unsigned int width, unsigned int element_i);
static unsigned int choice_row_id_i(unsigned int row_i, unsigned int width, unsigned int element_i)
{
    return 1 + (row_i*width*5) + (element_i*5);
}


static int choice_row_append(struct ttrie__obj *tt, unsigned int table_number, unsigned char (*row_id)[5]);
static int choice_row_append(struct ttrie__obj *tt, unsigned int table_number, unsigned char (*row_id)[5])
{
    assert (table_number <= tt->last_choice_i);
    struct ttrie__choice_table *ct = tt->choice_tables + table_number;
    int width = get_choice_table_width(ct);
    int num_rows = get_choice_table_row_count(ct);  // num_rows
    //printf("num_rows %u\n", num_rows);
    // If the table depth is going to be
    // 1,2,4,8,16,32,64,128, Then double
    //else if > 256
    // STOP
    
    if (power_of_two_or_zero(num_rows) && num_rows < MAX_CHOICE_TABLE_DEPTH)  // Never let it expand when it 255 +1.
    {
        if (ct->ids == NULL)
        {
            resize_choice_table(ct, 1);
            ct->ids[0] = 0;
            for (int i=0; i<width; ++i)
            {
                for (int j=0; j<5; ++j)
                {
                    ct->ids[1 + (i*5) + j] = row_id[i][j];
                }
            }
            return ct->ids[0];
        }
        else
        {
            resize_choice_table(ct, num_rows*2);
        }
    }

    for (int i=0; i<width; ++i)
    {
        for (int j=0; j<5; ++j)
        {
            ct->ids[choice_row_id_i(num_rows, width, i) + j] = row_id[i][j];  //row_i = num_rows when appending
        }
    }
    ct->ids[0]++;
    return ct->ids[0];
}

static int new_choice_row(struct ttrie__obj *tt, char *table_name, unsigned char name_len, unsigned char (*row_id)[5], unsigned char result_id[5]);
static int new_choice_row(struct ttrie__obj *tt, char *table_name, unsigned char name_len, unsigned char (*row_id)[5], unsigned char result_id[5])
{
    unsigned int table_number = find_add_table(tt, table_name, name_len);  
    unsigned int row_i = choice_row_append(tt, table_number, row_id);

    //printf("new_choice_row, %u\n", ct->table_number);
    encode_id(CHOICE_TABLE, table_number, row_i, result_id);

    return 0;
}

static unsigned long long  choice_tables_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long  choice_tables_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    mmbuf__append(m, &(tt->last_choice_i), sizeof(tt->last_choice_i));
    offset += sizeof(tt->last_choice_i);

    for (unsigned int i=0; i<=tt->last_choice_i; ++i)
    {
        unsigned char len_m_2 = tt->choice_tables[i].name[0];
        mmbuf__append(m, &len_m_2, 1);
        
        offset += 1;
        mmbuf__append(m, tt->choice_tables[i].name+1, len_m_2+1);
        offset += len_m_2+1;

        unsigned char last_row_i = tt->choice_tables[i].ids[0];
        mmbuf__append(m, &last_row_i, 1);
        offset += 1;

        unsigned int nrows = (unsigned int) last_row_i + 1;
        unsigned int width = (unsigned int) len_m_2 + 1;
        unsigned int size = nrows*width*5;

        mmbuf__append(m, tt->choice_tables[i].ids+1, size);
        offset += (last_row_i+1)*(len_m_2+1);
    }

    return offset;
}

static unsigned long long choice_tables_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long choice_tables_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    unsigned char *read_from = NULL;

    mmbuf__get_data(m, (void **) &read_from, offset, sizeof(tt->last_choice_i));
    memcpy((void *) &(tt->last_choice_i), read_from, sizeof(tt->last_choice_i));
    offset += sizeof(tt->last_choice_i);
    
    resize_choice_tables(tt, tt->last_choice_i + 1);           // Here we make the the required number of tables

    for (unsigned int i=0; i<=tt->last_choice_i; ++i)
    {

        unsigned char len_m_2;
        mmbuf__get_data(m, (void **) &read_from, offset, 1);
        memcpy(&len_m_2, read_from, 1);

        mmbuf__get_data(m, (void **) &read_from, offset, len_m_2+2);
        
        unsigned char *tmp = malloc(len_m_2+2);
        memcpy(tmp, read_from, len_m_2+2);
        offset += len_m_2+2;
        insert_table(tt, tmp, i, false);    // Don't auto size.

        mmbuf__get_data(m, (void **) &read_from, offset, 1);
        unsigned char rows_m_1;
        memcpy(&rows_m_1, read_from, 1);

        unsigned int nrows = (unsigned int) rows_m_1 + 1;
        unsigned int width = (unsigned int) len_m_2 + 1;

        resize_choice_table(tt->choice_tables + i, nrows);
        unsigned int size = nrows*width*5+1;
        mmbuf__get_data(m, (void **) &read_from, offset, size);
        memcpy(tt->choice_tables[i].ids, read_from, size);
        offset += size;
    }
    return offset;
}


static int choice_tables_teardown(struct ttrie__obj *tt);
static int choice_tables_teardown(struct ttrie__obj *tt)
{
    printf("in choice_tables_teardown\n");
    if (tt->choice_tables != NULL)
    {
        for (int i=0; i<=tt->last_choice_i; ++i)
        {
            free(tt->choice_tables[i].name);
            free(tt->choice_tables[i].ids);    
        }
        free(tt->choice_tables);
    }
    return 0;
}

static unsigned int get_element_i(struct ttrie__choice_table *ct, unsigned int table_i, unsigned long long row_i, unsigned char k);
static unsigned int get_element_i(struct ttrie__choice_table *ct, unsigned int table_i, unsigned long long row_i, unsigned char k)
{
    //printf("get_element_i, table_i=%u row_i=%llu k=%c, name='%.*s'\n", table_i, row_i, k, ct->name[0] + 1, ct->name+1);
    int low = 1;
    int high = ct->name[0] + 2; //One beyond max index 
    
    unsigned char c;
    int i = 0;
    while (low < high)
    {
        i = (high+low)/2;
        c = ct->name[i];
        //printf("fkeys %c %c %u %u %u\n", c, k, i, low, high);
        if (k > c)
        {
            low = i + 1;
        }
        else if (k < c)
        {
            high = i;
        }
        else
        {
            int width = get_choice_table_width(ct);
            //int num_rows = get_choice_table_row_count(ct);
            return (i-1)*5 + (row_i*width*5) + 1;   // First element is a len for both ids, and table_name
        }
    }
    //printf("Couldn't find key \n");
    return 0;
}       

static int choice_lookup(struct ttrie__obj *tt, unsigned int table_i, unsigned long long row_i, unsigned char k, unsigned char id[5]);
static int choice_lookup(struct ttrie__obj *tt, unsigned int table_i, unsigned long long row_i, unsigned char k, unsigned char id[5])
{

    struct ttrie__choice_table *ct = tt->choice_tables + table_i;
    unsigned int i = get_element_i(ct, table_i, row_i, k);
    //printf("Choice lookup for k=%c, %u\n", k, i);
    if (i==0)
    {
        return 1;
    }
    else
    {
        memcpy(id, ct->ids + i, 5);
        //printf("fooy\n");
        //*id =  (unsigned char (*)[5]) ct->ids + i;  // Zeroth element is always just the ids len -2
        return 0;
    }
    return 1;
}

static void printall_choice_tables(struct ttrie__obj *tt);
static void printall_choice_tables(struct ttrie__obj *tt)
{
    unsigned int num_tables = tt->last_choice_i + 1;
    printf("=============CHOICE===============\n");
    printf("| number tables = '%u'\n", num_tables);
    if (tt->choice_tables == NULL)
    {
        printf("WARNING: NULL choice_tables\n");
        return;
    }

    for (unsigned int i=0; i<num_tables; ++i)
    {
        struct ttrie__choice_table ct = tt->choice_tables[i];
        printf("|----\n");
        unsigned int width = get_choice_table_width(&ct);
        unsigned int nrows = get_choice_table_row_count(&ct);
        printf("| table_%u name='%.*s', n_elements='%u'\n", i, width, ct.name+1, width);
        printf("| number_rows='%u'\n", nrows);
        for (unsigned int j=0; j<nrows; ++j)
        {
            printf("|\t row%u::", j);
            for (unsigned int k=0; k<width; ++k)
            {
                unsigned int offset = choice_row_id_i(j, width, k);
                unsigned char table_type;
                unsigned int table_i;
                unsigned long long row_i;
                decode_id(tt->choice_tables[i].ids+offset, &table_type, &table_i, &row_i);
                printf("%u=|typ='%u', ti='%u', ri='%llu'|, ", k, table_type, table_i, row_i);
            }
            printf("\n");
        }
    }
    return;
}

/**
 *   TERMINATOR
 */
static int resize_terminator_table(struct ttrie__obj *tt, unsigned long long new_nitems);
static int resize_terminator_table(struct ttrie__obj *tt, unsigned long long new_nitems)
{
    tt->term_table.results = realloc(tt->term_table.results, sizeof(unsigned long long[new_nitems]));
    return 0;
}

static void terminator_set(struct ttrie__obj *tt, unsigned long long pos, unsigned long long value, unsigned char node_id[5]);
static void terminator_set(struct ttrie__obj *tt, unsigned long long pos, unsigned long long value, unsigned char node_id[5])
{
    tt->term_table.results[pos] = value;
    encode_id(TERMINATOR_TABLE, 0, pos, node_id);
}

static int terminator_append(struct ttrie__obj *tt, unsigned long long value, unsigned char node_id[5]);
static int terminator_append(struct ttrie__obj *tt, unsigned long long value, unsigned char node_id[5])
{
    if (power_of_two_or_zero(tt->term_table.last_i + 1))
    {
        if (tt->term_table.results == NULL)
        {
            resize_terminator_table(tt, 1);
            tt->term_table.last_i = 0;
            terminator_set(tt, tt->term_table.last_i, value, node_id);
            return 0;
        }
        else
        {
            unsigned long long old_nitems = tt->term_table.last_i + 1;
            unsigned long long new_nitems = old_nitems << 1;
            resize_terminator_table(tt, new_nitems);
        }
    }
    tt->term_table.last_i++;
    terminator_set(tt, tt->term_table.last_i, value, node_id);
    return 0;
}

static unsigned long long terminator_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long terminator_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    mmbuf__append(m, &(tt->term_table.last_i), sizeof(tt->term_table.last_i));
    offset += sizeof(tt->term_table.last_i);

    unsigned int size_results = sizeof(unsigned long long)*(tt->term_table.last_i+1);
    mmbuf__append(m, tt->term_table.results, size_results);
    offset += size_results;
    return offset;
}

static unsigned long long terminator_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long terminator_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    unsigned char *read_from = NULL;

    mmbuf__get_data(m, (void **) &read_from, offset, sizeof(tt->term_table.last_i));
    memcpy(&(tt->term_table.last_i), read_from, sizeof(tt->term_table.last_i));
    offset += sizeof(tt->term_table.last_i);

    resize_terminator_table(tt, tt->term_table.last_i+1);

    unsigned int size_results = sizeof(unsigned long long)*(tt->term_table.last_i+1);
    mmbuf__get_data(m, (void **) &read_from, offset, size_results);
    memcpy(tt->term_table.results, read_from, size_results);
    offset += size_results;

    return offset;
}

static int terminator_teardown(struct ttrie__obj *tt);
static int terminator_teardown(struct ttrie__obj *tt)
{
    printf("in terminator_teardown\n");
    if (tt->term_table.results != NULL)
        free(tt->term_table.results);
    return 0;
}

static unsigned long long terminator_get_result(struct ttrie__obj *tt, unsigned long long row_i);
static unsigned long long terminator_get_result(struct ttrie__obj *tt, unsigned long long row_i)
{
    return tt->term_table.results[row_i];
}

static void printall_terminators(struct ttrie__obj *tt);
static void printall_terminators(struct ttrie__obj *tt)
{
    unsigned long long num_terminators = tt->term_table.last_i + 1;
    printf("=========TERMINATORS===============\n");
    printf("| number terminators = '%llu'\n", num_terminators);
    if (tt->term_table.results == NULL)
    {
        printf("WARNING: NULL term_table\n");
        return;
    }

    for (unsigned long long i=0; i<num_terminators; ++i)
    {
        printf("| %llu='%llu'\n", i, tt->term_table.results[i]);
    }
    return;
}

/**
 *   PASSTHROUGH
**/
static int resize_passthrough_table(struct ttrie__obj *tt, unsigned long long new_nitems);
static int resize_passthrough_table(struct ttrie__obj *tt, unsigned long long new_nitems)
{
    tt->pass_table.ids = realloc(tt->pass_table.ids, sizeof(unsigned char [new_nitems][5]));
    tt->pass_table.results = realloc(tt->pass_table.results, sizeof(unsigned long long[new_nitems]));
    return 0;
}

static void passthrough_set(struct ttrie__obj *tt, unsigned long long pos, unsigned char child_id[5], unsigned long long value, unsigned char node_id[5]);
static void passthrough_set(struct ttrie__obj *tt, unsigned long long pos, unsigned char child_id[5], unsigned long long value, unsigned char node_id[5])
{
    tt->pass_table.results[pos] = value;
    memcpy(tt->pass_table.ids[pos], child_id, 5);
    encode_id(PASSTHROUGH_TABLE, 0, pos, node_id);
}

static int passthrough_append(struct ttrie__obj *tt, unsigned char child_id[5], unsigned long long value, unsigned char node_id[5]);
static int passthrough_append(struct ttrie__obj *tt, unsigned char child_id[5], unsigned long long value, unsigned char node_id[5])
{
    if (power_of_two_or_zero(tt->pass_table.last_i + 1))
    {
        if (tt->pass_table.results == NULL)
        {
            resize_passthrough_table(tt, 1);
            tt->pass_table.last_i = 0;
            passthrough_set(tt, tt->pass_table.last_i, child_id, value, node_id);
            return 0;
        }
        else
        {
            unsigned long long old_nitems = tt->pass_table.last_i + 1;
            unsigned long long new_nitems = old_nitems << 1;
            resize_passthrough_table(tt, new_nitems);
        }
    }
    tt->pass_table.last_i++;
    passthrough_set(tt, tt->pass_table.last_i, child_id, value, node_id);
    return 0;
}

static unsigned long long passthrough_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long passthrough_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    mmbuf__append(m, &(tt->pass_table.last_i), sizeof(tt->pass_table.last_i));
    offset += sizeof(tt->pass_table.last_i);

    unsigned int size_ids = 5*(tt->pass_table.last_i+1);
    mmbuf__append(m, tt->pass_table.ids, size_ids);
    offset += size_ids;

    unsigned int size_results = sizeof(unsigned long long)*(tt->pass_table.last_i+1);
    mmbuf__append(m, tt->pass_table.results, size_results);
    offset += size_results;
    return offset;
}

static unsigned long long passthrough_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long passthrough_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    unsigned char *read_from = NULL;

    mmbuf__get_data(m, (void **) &read_from, offset, sizeof(tt->pass_table.last_i));
    memcpy(&(tt->pass_table.last_i), read_from, sizeof(tt->pass_table.last_i));
    offset += sizeof(tt->pass_table.last_i);

    resize_passthrough_table(tt, tt->pass_table.last_i+1);

    unsigned int size_ids = 5*(tt->pass_table.last_i+1);
    mmbuf__get_data(m, (void **) &read_from, offset, size_ids);
    memcpy(tt->pass_table.ids, read_from, size_ids);
    offset += size_ids;

    unsigned int size_results = sizeof(unsigned long long)*(tt->pass_table.last_i+1);
    mmbuf__get_data(m, (void **) &read_from, offset, size_results);
    memcpy(tt->pass_table.results, read_from, size_results);
    offset += size_results;

    return offset;
}

static int passthrough_teardown(struct ttrie__obj *tt);
static int passthrough_teardown(struct ttrie__obj *tt)
{
    printf("in passthrough_teardown\n");
    if (tt->pass_table.ids != NULL)
        free(tt->pass_table.ids);
    if (tt->pass_table.results != NULL)
        free(tt->pass_table.results);
    return 0;
}

static unsigned long long passthrough_lookup(struct ttrie__obj *tt, unsigned long long row_i, unsigned char id[5]);
static unsigned long long passthrough_lookup(struct ttrie__obj *tt, unsigned long long row_i, unsigned char id[5])
{
    memcpy(id, tt->pass_table.ids + row_i, 5);
    //*id = tt->pass_table.ids[row_i];
    return tt->pass_table.results[row_i];
}

static void printall_passthrough(struct ttrie__obj *tt);
static void printall_passthrough(struct ttrie__obj *tt)
{
    unsigned long long num_passthrough = tt->pass_table.last_i + 1;
    printf("=========PASSTHROUGH===============\n");
    printf("| number passthrough = '%llu'\n", num_passthrough);
    if (tt->pass_table.ids == NULL)
    {
        printf("WARNING: NULL pass_table\n");
        return;
    }

    for (unsigned long long i=0; i<num_passthrough; ++i)
    {
        unsigned char table_type;
        unsigned int table_i;
        unsigned long long row_i;
        decode_id( (unsigned char *)tt->pass_table.ids + i*5, &table_type, &table_i, &row_i);
        printf("| %llu  result='%llu' child=|typ='%u', ti='%u', ri='%llu'| \n", i, tt->pass_table.results[i], table_type, table_i, row_i);
    }
    return;
}


/**
 *   SUFFIX
**/
static int resize_suffix_table(struct ttrie__obj *tt, unsigned long long new_nitems);
static int resize_suffix_table(struct ttrie__obj *tt, unsigned long long new_nitems)
{
    //printf("reszing suffix to %lu\n", sizeof(unsigned long long[new_nitems]));
    tt->suffix_table.results = realloc(tt->suffix_table.results, sizeof(unsigned long long[new_nitems]));
    tt->suffix_table.suffix_lens = realloc(tt->suffix_table.suffix_lens, sizeof(unsigned char [new_nitems]));
    tt->suffix_table.suffixs = realloc(tt->suffix_table.suffixs, sizeof(unsigned char *[new_nitems]));
    return 0;
}

static void suffix_set(struct ttrie__obj *tt, unsigned long long pos, unsigned char *suffix, char n, unsigned long long value, unsigned char node_id[5]);
static void suffix_set(struct ttrie__obj *tt, unsigned long long pos, unsigned char *suffix, char n, unsigned long long value, unsigned char node_id[5])
{
    //printf("suffix len n =%u\n", n);
    //printf("last suffix table i =%llu\n", tt->suffix_table.last_i);
    tt->suffix_table.results[pos] = value;
    tt->suffix_table.suffix_lens[pos] = n;
    
    unsigned char *string_storage = malloc(n);               // Remeber to clean this up
    memcpy(string_storage, suffix, n);
    tt->suffix_table.suffixs[pos] = string_storage;

    encode_id(SUFFIX_TABLE, 0, pos, node_id);
}

static int suffix_append(struct ttrie__obj *tt, unsigned char *suffix, unsigned char n, unsigned long long value, unsigned char node_id[5]);
static int suffix_append(struct ttrie__obj *tt, unsigned char *suffix, unsigned char n, unsigned long long value, unsigned char node_id[5])
{
    if (power_of_two_or_zero(tt->suffix_table.last_i + 1))
    {
        if (tt->suffix_table.results == NULL)
        {
            resize_suffix_table(tt, 1);
            tt->suffix_table.last_i = 0;
            suffix_set(tt, tt->suffix_table.last_i, suffix, n, value, node_id);
            return 0;
        }
        else
        {
            unsigned long long old_nitems = tt->suffix_table.last_i + 1;
            unsigned long long new_nitems = old_nitems << 1;

            resize_suffix_table(tt, new_nitems);
        }
    }
    tt->suffix_table.last_i++;
    suffix_set(tt, tt->suffix_table.last_i, suffix, n, value, node_id);
    return 0;
}

static unsigned long long suffix_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long suffix_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    mmbuf__append(m, &(tt->suffix_table.last_i), sizeof(tt->suffix_table.last_i));
    offset += sizeof(tt->suffix_table.last_i);

    unsigned int size_results = (sizeof(unsigned long long))*(tt->suffix_table.last_i+1);
    printf("pre pre core, %u\n", size_results);
    printf("pre core, %u %p\n", size_results, tt->suffix_table.results);
    mmbuf__append(m, tt->suffix_table.results, size_results);
    offset += size_results;

    // Need todo lens and suffixs together :(
    for (unsigned int i=0; i<=tt->suffix_table.last_i; ++i)
    {
        unsigned int suffix_size = tt->suffix_table.suffix_lens[i];
        mmbuf__append(m, tt->suffix_table.suffix_lens + i, 1);
        offset += 1;

        mmbuf__append(m, tt->suffix_table.suffixs[i], suffix_size);
        offset += suffix_size;
    }
    
    return offset;
}

static unsigned long long suffix_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long suffix_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    unsigned char *read_from = NULL;

    mmbuf__get_data(m, (void **) &read_from, offset, sizeof(tt->suffix_table.last_i));
    memcpy((void *) &(tt->suffix_table.last_i), read_from, sizeof(tt->suffix_table.last_i));
    offset += sizeof(tt->suffix_table.last_i);

    resize_suffix_table(tt, tt->suffix_table.last_i+1);

    unsigned int size_results = sizeof(unsigned long long)*(tt->suffix_table.last_i+1);
    mmbuf__get_data(m, (void **) &read_from, offset, size_results);
    memcpy(tt->suffix_table.results, read_from, size_results);
    offset += size_results;
    
    // Need todo lens and suffixs   together :(
    for (unsigned int i=0; i<=tt->suffix_table.last_i; ++i)
    {
        mmbuf__get_data(m, (void **) &read_from, offset, 1);
        memcpy(tt->suffix_table.suffix_lens + i, read_from, 1);
        offset += 1;
        unsigned int str_len = tt->suffix_table.suffix_lens[i];


        mmbuf__get_data(m, (void **) &read_from, offset, str_len);
        tt->suffix_table.suffixs[i] = malloc(str_len);
        memcpy(tt->suffix_table.suffixs[i], read_from, str_len);
        offset += str_len;
    }

    return offset;
}

static int suffix_teardown(struct ttrie__obj *tt);
static int suffix_teardown(struct ttrie__obj *tt)
{
    printf("in suffix_teardown\n");
    if (tt->suffix_table.results != NULL)
        free(tt->suffix_table.results);
    if (tt->suffix_table.suffix_lens != NULL)
        free(tt->suffix_table.suffix_lens);
    if (tt->suffix_table.suffixs != NULL)
    {
        for (int i=0; i <= tt->suffix_table.last_i; i++)
        {
            free(tt->suffix_table.suffixs[i]);
        }
        free(tt->suffix_table.suffixs);
    }
    return 0;
}

static unsigned char suffix_get_len(struct ttrie__obj *tt, unsigned long long row_i);
static unsigned char suffix_get_len(struct ttrie__obj *tt, unsigned long long row_i)
{
    return tt->suffix_table.suffix_lens[row_i];
}

static void suffix_get_suffix(struct ttrie__obj *tt, unsigned long long row_i, unsigned char **suffix);
static void suffix_get_suffix(struct ttrie__obj *tt, unsigned long long row_i, unsigned char **suffix)
{
    *suffix = tt->suffix_table.suffixs[row_i];
}

static unsigned long long suffix_get_result(struct ttrie__obj *tt, unsigned long long row_i);
static unsigned long long suffix_get_result(struct ttrie__obj *tt, unsigned long long row_i)
{
    return tt->suffix_table.results[row_i];
}

static void printall_suffix(struct ttrie__obj *tt);
static void printall_suffix(struct ttrie__obj *tt)
{
    unsigned long long num_suffix = tt->suffix_table.last_i + 1;
    printf("============SUFFIX===============\n");
    printf("| number suffix = '%llu'\n", num_suffix);
    if (tt->suffix_table.results == NULL)
    {
        printf("WARNING: NULL suffix_table\n");
        return;
    }

    for (unsigned long long i=0; i<num_suffix; ++i)
    {
        printf("| %llu result='%llu' len='%u' suffix='%.*s'  \n", i, tt->suffix_table.results[i], tt->suffix_table.suffix_lens[i], tt->suffix_table.suffix_lens[i], tt->suffix_table.suffixs[i]);
    }
    return;
}


/**
 *   INFIX
**/
static int resize_infix_table(struct ttrie__obj *tt, unsigned long long new_nitems);
static int resize_infix_table(struct ttrie__obj *tt, unsigned long long new_nitems)
{
    tt->infix_table.ids = realloc(tt->infix_table.ids, sizeof(unsigned char [new_nitems][5]));
    tt->infix_table.infix_lens = realloc(tt->infix_table.infix_lens, sizeof(unsigned char [new_nitems]));
    tt->infix_table.infixs = realloc(tt->infix_table.infixs, sizeof(unsigned char *[new_nitems]));

    return 0;
}

static int infix_set(struct ttrie__obj *tt, unsigned long long pos, unsigned char *infix, char n, unsigned char child_node_id[5], unsigned char node_id[5]);
static int infix_set(struct ttrie__obj *tt, unsigned long long pos, unsigned char *infix, char n, unsigned char child_node_id[5], unsigned char node_id[5])
{
    //printf("ehlooooo %u\n", n);
    memcpy(tt->infix_table.ids[pos], child_node_id, 5);
    
    tt->infix_table.infix_lens[pos] = n;

    unsigned char *string_storage = malloc(n);               // Remeber to clean this up
    memcpy(string_storage, infix, n);
    tt->infix_table.infixs[pos] = string_storage;

    encode_id(INFIX_TABLE, 0, pos, node_id);
    return 0;
}

static int infix_append(struct ttrie__obj *tt, unsigned char *infix, char n, unsigned char child_node_id[5], unsigned char node_id[5]);
static int infix_append(struct ttrie__obj *tt, unsigned char *infix, char n, unsigned char child_node_id[5], unsigned char node_id[5])
{
    //printf("infix_append hello %u\n", n);
    if (power_of_two_or_zero(tt->infix_table.last_i + 1))
    {
        if (tt->infix_table.infix_lens == NULL)
        {
            resize_infix_table(tt, 1);
            tt->infix_table.last_i = 0;
            infix_set(tt, tt->infix_table.last_i, infix, n, child_node_id, node_id);
            return 0;
        }
        else
        {
            unsigned long long old_nitems = tt->infix_table.last_i + 1;
            unsigned long long new_nitems = old_nitems << 1;
            resize_infix_table(tt, new_nitems);
        }
    }
    tt->infix_table.last_i++;
    infix_set(tt, tt->infix_table.last_i, infix, n, child_node_id, node_id);
    return 0;
} 

static unsigned long long  infix_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long  infix_serialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    mmbuf__append(m, &(tt->infix_table.last_i), sizeof(tt->infix_table.last_i));
    offset += sizeof(tt->infix_table.last_i);

    unsigned int size_ids = 5*(tt->infix_table.last_i+1);
    mmbuf__append(m, tt->infix_table.ids, size_ids);
    offset += size_ids;

    // Need todo lens and suffixs together :(
    for (unsigned int i=0; i<=tt->infix_table.last_i; ++i)
    {
        unsigned int infix_size = tt->infix_table.infix_lens[i];
        mmbuf__append(m, tt->infix_table.infix_lens + i, 1);
        offset += 1;

        mmbuf__append(m, tt->infix_table.infixs[i], infix_size);
        offset += infix_size;
    }
    
    return offset;
}

static unsigned long long  infix_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset);
static unsigned long long  infix_deserialise(struct ttrie__obj *tt, struct mmbuf_obj *m, unsigned long long offset)
{
    unsigned char *read_from = NULL;

    mmbuf__get_data(m, (void **) &read_from, offset, sizeof(tt->infix_table.last_i));
    memcpy(&(tt->infix_table.last_i), read_from, sizeof(tt->infix_table.last_i));
    offset += sizeof(tt->infix_table.last_i);

    resize_infix_table(tt, tt->infix_table.last_i+1);

    unsigned int size_ids = 5*(tt->infix_table.last_i+1);
    mmbuf__get_data(m, (void **) &read_from, offset, size_ids);
    memcpy(tt->infix_table.ids, read_from, size_ids);
    offset += size_ids;

    // Need todo lens and suffixs   together :(
    for (unsigned int i=0; i<=tt->infix_table.last_i; ++i)
    {
        mmbuf__get_data(m, (void **) &read_from, offset, 1);
        memcpy(tt->infix_table.infix_lens + i, read_from, 1);
        offset += 1;
        unsigned int str_len = tt->infix_table.infix_lens[i];

        mmbuf__get_data(m, (void **) &read_from, offset, str_len);
        tt->infix_table.infixs[i] = malloc(str_len);
        memcpy(tt->infix_table.infixs[i], read_from, str_len);
        offset += str_len;
    }

    return offset;
}


static int infix_teardown(struct ttrie__obj *tt);
static int infix_teardown(struct ttrie__obj *tt)
{
    printf("in infix_teardown\n");
    if (tt->infix_table.ids != NULL)
        free(tt->infix_table.ids);
    if (tt->infix_table.infix_lens != NULL)
        free(tt->infix_table.infix_lens);
    if (tt->infix_table.infixs != NULL)
    {
        for (int i=0; i <= tt->infix_table.last_i; ++i)
        {
            free(tt->infix_table.infixs[i]);
        }
        free(tt->infix_table.infixs);
    }
    return 0;
}

static unsigned char infix_get_len(struct ttrie__obj *tt, unsigned long long row_i);
static unsigned char infix_get_len(struct ttrie__obj *tt, unsigned long long row_i)
{
    return tt->infix_table.infix_lens[row_i];
}

static void infix_get_infix(struct ttrie__obj *tt, unsigned long long row_i, unsigned char **infix);
static void infix_get_infix(struct ttrie__obj *tt, unsigned long long row_i, unsigned char **infix)
{
    *infix = tt->infix_table.infixs[row_i];
}

static void infix_get_childnode(struct ttrie__obj *tt, unsigned long long row_i, unsigned char id[5]);
static void infix_get_childnode(struct ttrie__obj *tt, unsigned long long row_i, unsigned char id[5])
{
    assert (tt->infix_table.ids!=NULL);
    //printf("infix child pointer address = %p %p\n", tt->infix_table.ids, tt->infix_table.ids + row_i);
    memcpy(id, tt->infix_table.ids + row_i, 5);
}


static void printall_infix(struct ttrie__obj *tt);
static void printall_infix(struct ttrie__obj *tt)
{
    unsigned long long num_infix = tt->infix_table.last_i + 1;
    printf("============INFIX===============\n");
    printf("| number infix = '%llu'\n", num_infix);
    if (tt->infix_table.infixs == NULL)
    {
        printf("WARNING: NULL infix_table\n");
        return;
    }

    for (unsigned long long i=0; i<num_infix; ++i)
    {
        printf("| %llu len='%u' infix='%.*s'  \n", i, tt->infix_table.infix_lens[i], tt->infix_table.infix_lens[i], tt->infix_table.infixs[i]);
        unsigned char table_type;
        unsigned int table_i;
        unsigned long long row_i;
        decode_id((unsigned char *)tt->infix_table.ids + i*5, &table_type, &table_i, &row_i);
        printf("|     child=|typ='%u', ti='%u', ri='%llu'| \n", table_type, table_i, row_i);

    }
    return;
}



/***************************
 *
 *   CONVERSION
 *
***************************/
static int convert_subtrie(struct ttrie__obj *tt, struct rtrie__node *from_node, unsigned char result_id[5]);

static int convert_subtrie_to_choice(struct ttrie__obj *tt, struct rtrie__node *from_node, int num_children, unsigned char tmp_result_id[5]);
static int convert_subtrie_to_choice(struct ttrie__obj *tt, struct rtrie__node *from_node, int num_children, unsigned char tmp_result_id[5])
{
    //printf("In convert_subtrie_to_choice %p\n", from_node);
    int i;
    char tmp_table_name[num_children];
    //printf("choice:: %.*s\n",  from_node->last_i+1, from_node->keys);
    for (i=0; i<num_children; ++i)
    {
        tmp_table_name[i] = from_node->keys[i];   
    }
    //printf("choice node children='%.*s' \n", i+1, tmp_table_name);

    unsigned char row_pointer[num_children][5];
    for (i=0; i<num_children; ++i)
    {
        convert_subtrie(tt, from_node->u.children + i, row_pointer[i]);
    }
    new_choice_row(tt, tmp_table_name, num_children, row_pointer, tmp_result_id);

    return 0;
}

static int convert_subtrie_to_suffix(struct ttrie__obj *tt, struct rtrie__node *from_node, int num_children, unsigned char tmp_result_id[5]);
static int convert_subtrie_to_suffix(struct ttrie__obj *tt, struct rtrie__node *from_node, int num_children, unsigned char tmp_result_id[5])
{
    //printf("suffix:: %u %.*s\n",  from_node->suffix_len, from_node->suffix_len, from_node->u.suffix);
    //printf("In convert_subtrie_to_suffix_infix %p\n", from_node);
    assert (num_children == 0);
    assert (rtrie__node_has_value(from_node));
    assert (rtrie__node_has_suffix(from_node));
    assert (from_node->suffix_len>0);

    suffix_append(tt, from_node->u.suffix, from_node->suffix_len, from_node->result, tmp_result_id);

    return 0;
}


static int convert_subtrie_to_infix(struct ttrie__obj *tt, struct rtrie__node *from_node, int num_children, unsigned char tmp_result_id[5]);
static int convert_subtrie_to_infix(struct ttrie__obj *tt, struct rtrie__node *from_node, int num_children, unsigned char tmp_result_id[5])
{
    //printf("In convert_subtrie_to_suffix_infix %p\n", from_node);
    
    //printf("infix:: %.*s\n",  from_node->last_i+1, from_node->keys);

    unsigned char tmp_string[STACK_SIZE];          
    int depth = 0;


    assert(num_children == 1);
    while (true)
    {
        if (depth >= STACK_SIZE)
        {
            printf("ERROR: Out of ttrie stack\n");
            exit(EXIT_FAILURE);
        }

        tmp_string[depth] = from_node->keys[0];  //bank it and move on

        from_node = from_node->u.children;   // next node = first child
        num_children = rtrie__node_has_children(from_node);

        if ((num_children > 1) || (rtrie__node_has_value(from_node)))
        {
            unsigned char tmp_sub_trie_id[5] = {0, 0, 0, 0, 0};
            //printf("infix node %p %.*s\n", from_node, depth+1, tmp_string);
            convert_subtrie(tt, from_node, tmp_sub_trie_id);
            infix_append(tt, tmp_string, depth+1, tmp_sub_trie_id, tmp_result_id);
            //print_result_id(tmp_result_id);
            break;
        }
        


        depth += 1;
    }
    return 0;
}

static int convert_subtrie(struct ttrie__obj *tt, struct rtrie__node *from_node, unsigned char result_id[5])
{
    //printf("In convert subtrie for node %p\n", from_node);
    
    if (rtrie__node_has_suffix(from_node))
    {
        
        int num_children = rtrie__node_has_children(from_node);
        //printf("a %u\n", from_node->suffix_len);
        return convert_subtrie_to_suffix(tt, from_node, num_children, result_id);
    }
    else if (!rtrie__node_has_children(from_node))
    {
        //printf("b\n");
        if (rtrie__node_has_value(from_node)) 
        {
            //printf("At a terminator with value='%llu'\n", from_node->result);
            return terminator_append(tt, from_node->result, result_id);
        }
        else
        {
            printf("ERROR: No children no value\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        //printf("c\n");
        struct rtrie__node *orig_node = from_node;
        unsigned char tmp_result_id[5] = {0, 0, 0, 0, 0};  //Possibility of passthrough
        
        int num_children = rtrie__node_has_children(from_node);
        if (num_children > 1)
        {
            //printf("In a node with children >1 \n");
            convert_subtrie_to_choice(tt, from_node, num_children, tmp_result_id);
        }
        else if (num_children == 1)
        {
            //printf("whhhh\n");
            convert_subtrie_to_infix(tt, from_node, num_children, tmp_result_id);
        }

        // As We haven't yet handled what todo if the origin node has a value.
        if (rtrie__node_has_value(orig_node))
        {
            //printf("passthrough node value = %llu \n", orig_node->result);
            passthrough_append(tt, tmp_result_id, orig_node->result, result_id);
        }
        else
        {
            //printf("Setting result_id\n");
            memcpy(result_id, tmp_result_id, 5);
        }
        return 0;
    }
    return 1;
}

/*---
  EXTRA
---*/



/*---
 GLOBAL METHODS
---*/

/* Allocate choice table
malloc(sizeof(struct ttrie__choice_table[1]));
tt->choice_tables->name = '\0';                                       //last_i -1
tt->choice_tables->id = malloc(sizeof(char[1][5]));
tt->choice_tables->id = {0, 0, 0, 0, 0}

First elements
name[0] must be able to address 256 possible key_char vals.  0 = 1 element following in the array.
name[0] = total_array_size-2 = number_of_elements_following -1;

ids[0][0] must be able to address 256 rows. Hence is the same as above.
*/

int ttrie__open(struct ttrie__obj *tt, char mode)
{
    tt->len = 0;
    for (int i=0; i<5; ++i)
    {
        tt->root_id[i] = 0;
    }

    if (mode=='w')
    {
        tt->index_table_names = true;
        tt->table_names_index = malloc(sizeof(struct rtrie__view));
        //printf("!!!!!!!!!!!!!!!%p\n", tt->table_names_index);
        rtrie__open(tt->table_names_index, "trees");
    }
    else if (mode=='r')
    {
        tt->index_table_names = false;
        tt->table_names_index = NULL;
    }
    else
    {
        printf("ERROR: Only two modes possible\n");
        exit(EXIT_FAILURE);
    }
    tt->mode = mode;
    tt->last_choice_i = 0;
    tt->choice_tables = NULL;  

    tt->term_table.results = NULL;
    tt->term_table.last_i = 0;

    tt->pass_table.ids = NULL; 
    tt->pass_table.results = NULL;
    tt->pass_table.last_i = 0;

    tt->suffix_table.results = NULL;
    tt->suffix_table.suffix_lens = NULL;
    tt->suffix_table.suffixs = NULL;
    tt->suffix_table.last_i = 0;

    tt->infix_table.ids = NULL;
    tt->infix_table.infix_lens = NULL;
    tt->infix_table.infixs = NULL;
    tt->infix_table.last_i = 0;

    return 0;
}


void ttrie__unindex(struct ttrie__obj *tt)
{
    if (tt->table_names_index!=NULL)
    {
        //printf("tt->index_table_names = %u \n", tt->index_table_names);
        rtrie__close(tt->table_names_index);
        free(tt->table_names_index); 
        tt->index_table_names = false;
    }
}

int ttrie__close(struct ttrie__obj *tt)
{
    ttrie__unindex(tt);

    infix_teardown(tt);
    suffix_teardown(tt);
    passthrough_teardown(tt);
    terminator_teardown(tt);
    choice_tables_teardown(tt);

    return 0;
}



int ttrie__convert(struct ttrie__obj *tt, struct rtrie__view *rt)
{
    if (tt->len != 0)
    {
        printf("Error can't reuse open tt instances\n");
        return 1;
    }
    tt->len = rt->len;
    return convert_subtrie(tt, rt->root, tt->root_id);
}

int ttrie__write(struct ttrie__obj *tt, char *trie_file_name)
{
    struct mmbuf_obj m;
    mmbuf__setup(&m, trie_file_name, "ws");
    unsigned long long offset = 0;

    offset = choice_tables_serialise(tt, &m, offset);
    offset = terminator_serialise(tt, &m, offset);
    offset = passthrough_serialise(tt, &m, offset);
    offset = suffix_serialise(tt, &m, offset);
    offset = infix_serialise(tt, &m, offset);


    mmbuf__teardown(&m);
    return 0;
}


int ttrie__read(struct ttrie__obj *tt, char *trie_file_name)
{
    struct mmbuf_obj m;
    mmbuf__setup(&m, trie_file_name, "rs");
    unsigned long long offset = 0;

    offset = choice_tables_deserialise(tt, &m, offset);
    offset = terminator_deserialise(tt, &m, offset);
    offset = passthrough_deserialise(tt, &m, offset);
    offset = suffix_deserialise(tt, &m, offset);
    offset = infix_deserialise(tt, &m, offset);

    mmbuf__teardown(&m);
    return 0;
}

unsigned long long ttrie__len(struct ttrie__obj *tt)
{
    return tt->len;
}


// Must be a more polymorphic way todo this.
int ttrie__get(struct ttrie__obj *tt, void *key, unsigned int keylen, unsigned long long *result)
{
    unsigned char current_node_id[5];
    memcpy(current_node_id, tt->root_id, 5);
    unsigned char *_key = key;

    unsigned char table_type;
    unsigned int table_i;
    unsigned long long row_i;

    int index = 0;
    do
    {
        
        decode_id(current_node_id, &table_type, &table_i, &row_i);
        //printf("eeee %u %u %llu\n", table_type, table_i, row_i);

        int rc;
        switch (table_type)
        {
            case CHOICE_TABLE:
            {
                //printf("CHOICE_TABLE\n");
                unsigned char k = (unsigned char) _key[index++];
                rc = choice_lookup(tt, table_i, row_i, k, current_node_id);
                if (rc)
                {
                    printf("choice not found for k=%c\n", k);
                    return rc;
                }
                break;
            }
            case TERMINATOR_TABLE:
            {
                //printf("erm %lld\n", terminator_get_result(tt, row_i));
                //printf("TERMINATOR_TABLE\n");
                //printf("early terminator, key not found\n");
                return 1;
                break;
            }
            case PASSTHROUGH_TABLE:
            {
                //printf("PASSTHROUGH_TABLE\n");
                passthrough_lookup(tt, row_i, current_node_id);
                break;
            }
            case SUFFIX_TABLE:
            {
                //printf("SUFFIX_TABLE\n");
                int len = suffix_get_len(tt, row_i);
                if (keylen > index + len)
                {
                    //printf("Short suffix, key not found, %d, %d, %d\n", keylen, index, len);
                    return 1;
                }
                else if (keylen < index + len)
                {
                    //printf("dddd %u %u %u \n",keylen, index, len);
                    //printf("Long suffix, key not found\n");
                    return 1;
                }
                else
                {   
                    unsigned char (*tmp_suffix_p) = NULL;
                    suffix_get_suffix(tt, row_i, &tmp_suffix_p);
                    if (memcmp(tmp_suffix_p, _key+index, len) !=0)
                    {
                        //printf("Wrong suffix, key not found\n");
                        return 1;
                    }
                    index = keylen;
                }
                break;
            }
            case INFIX_TABLE:
            {
                //printf("INFIX_TABLE\n");
                int len = infix_get_len(tt, row_i);
                //printf("infix len = %d\n", len);
                if (keylen < index + len)
                {
                    //printf("Long infix, key not found\n");
                    return 1;
                }
                else
                {
                    //printf("Infix ok size i=%d\n", index);
                    unsigned char *tmp_infix_p = NULL;
                    infix_get_infix(tt, row_i, &tmp_infix_p);
                    //printf("infix = %.*s\n", len, tmp_infix_p);
                    if (memcmp(tmp_infix_p, _key+index, len) !=0)
                    {
                        printf("Wrong infix, key not found\n");
                        return 1;
                    }
                    //printf("precore?\n");
                    infix_get_childnode(tt, row_i, current_node_id);
                    //print_result_id(current_node_id);
                    index += len;
                }
                break;
            }
            default:
            {
                printf("ERROR: unknown table type %u\n", table_type);
                exit(EXIT_FAILURE);
                break;
            }
        }
    }
    while (index<keylen);


    //printf("Checking final node\n");
    
    if (index==keylen)
    {
        decode_id(current_node_id, &table_type, &table_i, &row_i);
        if (table_type==PASSTHROUGH_TABLE)
        {
            *result = passthrough_lookup(tt, row_i, current_node_id);
            return 0;
        }
        else if (table_type==TERMINATOR_TABLE)
        {
            *result = terminator_get_result(tt, row_i);
            return 0;
        }
        else if (table_type==SUFFIX_TABLE)
        {
            *result = suffix_get_result(tt, row_i);
            return 0;
        }
        else
        {
            printf("Non terminating table type %u\n", table_type);
            return 1;
        }
    }
    
    return 1;
}


void ttrie__dprint(struct ttrie__obj *tt)
{
    unsigned char table_type;
    unsigned int table_i;
    unsigned long long row_i;
    decode_id(tt->root_id, &table_type, &table_i, &row_i);
    printf("|ROOT NODE=|typ='%u', ti='%u', ri='%llu'| \n", table_type, table_i, row_i);
    printall_choice_tables(tt);
    printall_terminators(tt);
    printall_passthrough(tt);
    printall_suffix(tt);
    printall_infix(tt);
}


////////////////////////////////////////////////
////////////////////////////////////////////////


int ttrie__iter(struct ttrie__obj *tt)
{
    return 0;
}

int ttrie__next(struct ttrie__obj *tt, char *key, unsigned long long *result)
{
    return 0;
}
