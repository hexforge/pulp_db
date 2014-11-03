#include "varray.h"

//# how to make the underlying abstracted out?

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

static int choice_resize_tables(struct ttrie__obj *tt, unsigned int new_nitems);
static int choice_resize_tables(struct ttrie__obj *tt, unsigned int new_nitems)
{
    tt->choice_tables = realloc(tt->choice_tables, sizeof(struct ttrie__choice_table[new_nitems]));
    return 0;
}

static int choice_resize_table(struct ttrie__choice_table *ct, unsigned long long new_nrows);
static int choice_resize_table(struct ttrie__choice_table *ct, unsigned long long new_nrows)
{
    int width;
    if ((width = choice_table_width(ct)) == 0)
    {
        printf("ERROR: choice table not inited\n");
        exit(EXIT_FAILURE);
    }

    ct->ids = realloc(ct->ids, sizeof(unsigned char [new_nrows*width*5 +1]));   // This looks wrong
    return 0;
}


static void choice_insert_table(struct ttrie__obj *tt, unsigned char *table_str, unsigned int pos, bool auto_resize);
static void choice_insert_table(struct ttrie__obj *tt, unsigned char *table_str, unsigned int pos, bool auto_resize)
{
    if (auto_resize && power_of_two_or_zero(tt->last_choice_i + 1))
    {
        if (tt->choice_tables == NULL)
        {
            choice_resize_tables(tt, 1);
            choice_insert_table(tt, table_str, 0, 0);
            tt->last_choice_i = 0;
            return;
        }
        else
        {
            unsigned int old_nitems = tt->last_choice_i + 1;
            unsigned int new_nitems = old_nitems << 1;
            choice_resize_tables(tt, new_nitems);
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
        bool new_entry = false;
        struct rtrie__node *n = rtrie__add(tt->table_names_index, table_str+1, table_str[0] + 1, &new_entry);        // [0] is size -2.   
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

        //printf("Existing table len='%u' name='%.*s'  depth=%d\n", name_len, name_len, (char *)table_name, choice_table_row_count(tt->choice_tables +p));
        //unsigned int width = choice_table_width(tt->choice_tables + p);
        //printf("Found existing table at pos='%u', for name='%.*s', n_elements='%u'\n", p, width, tt->choice_tables[p].name+1, width);
    }
}

