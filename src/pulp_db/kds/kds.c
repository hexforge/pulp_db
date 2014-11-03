#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "kds.h"

// A field_name coresponds to an rtrie and a dpref.
// Each field_value corresponds to an int.
// each in coreresponds to a element in an array of dpref buffers

// On pulp__optimize
// for each field rtrie convert it into ttrie
//     iterate each element of the ttrie.
//         convert the dpref to something like cpref. depending on one to one, percentage of population etc.
//         Convert the index to a initial offset kinda value.
//     persist the ttrie.


static bool power_of_two_or_zero(unsigned int i);
static bool power_of_two_or_zero(unsigned int i)
{
    // Including zero as we want to expand then
    if ((i & (i - 1)) == 0)   // See exercise 2.9 K&R
        return true;
    else
        return false;
}

// Doubling buffer.  This code should be refactored to take account of this.  It used all over the place.
static unsigned int new_dref_buffer(struct keyds *kds);
static unsigned int new_dref_buffer(struct keyds *kds)
{

    if (kds->buffers == NULL)
    {
        kds->buffers = malloc(sizeof(struct dpref__buffer));
        kds->last_i = 0;
        dpref__setup_buffer(&kds->dref, kds->buffers + kds->last_i);
        return kds->last_i;
    }
    else if (power_of_two_or_zero(kds->last_i + 1))
    {
        unsigned int new_nitems = (kds->last_i + 1) << 1;
        kds->buffers = realloc(kds->buffers, sizeof(struct dpref__buffer[new_nitems]));
        //printf("doubling buffer to %u\n", new_nitems);
        //printf("%lu\n", sizeof(bool)); 
    }
    
    kds->last_i++;
    dpref__setup_buffer(&kds->dref, kds->buffers + kds->last_i);
    return kds->last_i;
}

static void flush_dpref_buffers(struct keyds *kds);
static void flush_dpref_buffers(struct keyds *kds)
{
    for (unsigned int i=0; i<=kds->last_i; ++i)
        dpref__teardown_buffer(&kds->dref, kds->buffers + i);
}

static void teardown_buffers(struct keyds *kds);
static void teardown_buffers(struct keyds *kds)
{
    if (kds->buffers != NULL)
    {
        flush_dpref_buffers(kds);
        free(kds->buffers);
        kds->buffers = NULL;
    }
}

static signed long long convert_dpref_to_cpref(struct cpref__obj *cpref, struct dpref__obj *dpref, unsigned int offset, unsigned long long num_msgs, unsigned long long total_num_msgs);
static signed long long convert_dpref_to_cpref(struct cpref__obj *cpref, struct dpref__obj *dpref, unsigned int offset, unsigned long long num_msgs, unsigned long long total_num_msgs)
{
    struct dpref__buffer buf;     //Could be static 
    struct cpref__stream stream;  //Could be static 
    dpref__setup_buffer(dpref, &buf, offset);
    signed long long cpref_offset = cpref__setup_write_stream(&stream, cpref, num_msgs);

    static signed long long refs[128];
    static unsigned int i;
    int j;
    for (i=0; i<num_msgs; ++i)
    {
        j = i & (128-1);

        refs[j] = dpref__geti(dpref, &buf, i);    // This is inefficent.
        //printf("HOW AM I HERE??? j=%d, ref=%lld \n", j, refs[j]);
        if (refs[j] == -1)
        {
            printf("ERRRORORORORRO\n");
            exit(EXIT_FAILURE);
            //j--;
            //break;
        }

        //printf("Writing %d of %llu msgs to c_stream from d_stream starting at %u\n", j, num_msgs, offset);
        if (j == 128-1)
        {
            //printf("Flushing stream\n");
            cpref__write_stream(&stream, refs, 128);
            //i = 0;
        }
    }
    if (j!=128-1)
    {
        //printf("Flushing stream\n");
        cpref__write_stream(&stream, refs, j+1);
    }

    cpref__close_stream(&stream);
    dpref__teardown_buffer(dpref, &buf);
    return cpref_offset;
}

void write_global_data(struct keyds *kds);
void write_global_data(struct keyds *kds)
{
    int offset = 0;
    //printf("kds->total_messages %lld \n", kds->total_messages);
    mmbuf__append(kds->meta_file, &(kds->total_messages), sizeof(kds->total_messages));
    offset += sizeof(kds->total_messages);
    
    mmbuf__append(kds->meta_file, &(kds->mapping_type), sizeof(kds->mapping_type));
    offset += sizeof(kds->mapping_type);
    
    //printf("kds->num_keys %lld \n", kds->num_keys);
    mmbuf__append(kds->meta_file, &(kds->num_keys), sizeof(kds->num_keys));
    offset += sizeof(kds->num_keys);
    
    return;
}

void read_global_data(struct keyds *kds);
void read_global_data(struct keyds *kds)
{
    unsigned char *read_from = NULL;
    int offset = 0;
    
    mmbuf__get_data(kds->meta_file, (void **) &read_from, offset, sizeof(kds->total_messages));
    memcpy(&(kds->total_messages), read_from, sizeof(kds->total_messages));
    offset += sizeof(kds->total_messages);
    //printf("kds->total_messages %lld \n", kds->total_messages);

    mmbuf__get_data(kds->meta_file, (void **) &read_from, offset, sizeof(kds->mapping_type));
    memcpy(&(kds->mapping_type), read_from, sizeof(kds->mapping_type));
    offset += sizeof(kds->mapping_type);

    mmbuf__get_data(kds->meta_file, (void **) &read_from, offset, sizeof(kds->num_keys));
    memcpy(&(kds->num_keys), read_from, sizeof(kds->num_keys));
    offset += sizeof(kds->num_keys);
    //printf("kds->num_keys %lld \n", kds->num_keys);
    
    return;
}

char *make_file_path(char *fold_path, char *field_name, char *suffix);
char *make_file_path(char *fold_path, char *field_name, char *suffix)
{
    int fold_path_len = strlen(fold_path);
    int fld_name_len = strlen(field_name);
    int suffix_len = strlen(suffix);

    char *result = malloc(fold_path_len + fld_name_len + suffix_len + 1);
    strncpy(result, fold_path, fold_path_len + 1);
    assert(result[fold_path_len]=='\0');

    strncat(result, field_name, fld_name_len + 1);
    assert(result[fld_name_len+fold_path_len]=='\0');

    strncat(result, suffix, suffix_len + 1);
    //printf("result='%s'\n", result);
    assert(result[fld_name_len+fold_path_len+suffix_len]=='\0');
    return result;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// Write methods
int kds__open(struct keyds *kds, char *folder_path, char *fieldname, char mode)
{
    kds->mode = mode;

    int fld_name_len = strlen(fieldname);
    int fold_path_len = strlen(folder_path);

    kds->field_name = malloc(fld_name_len + 1);
    strncpy(kds->field_name, fieldname, fld_name_len + 1);
    //printf("Field_name='%s'\n", kds->field_name);

    kds->folder_path = malloc(fold_path_len + 2);
    strncpy(kds->folder_path, folder_path, fold_path_len + 1);
    //printf("Folder_path='%s'\n", kds->folder_path);
    strcat(kds->folder_path, "/");

    kds->meta_file_path = make_file_path(kds->folder_path, kds->field_name, KDS_META_SUFFIX);
    kds->dpref_file_path = make_file_path(kds->folder_path, kds->field_name, KDS_DPREF_SUFFIX);
    kds->cpref_file_path = make_file_path(kds->folder_path, kds->field_name, KDS_CPREF_SUFFIX);
    kds->ttrie_file_path = make_file_path(kds->folder_path, kds->field_name, KDS_TTRIE_SUFFIX);

    kds->mapping_type = KDS_MAPPING_UNKNOWN;
    kds->total_messages = 0;
    kds->buffers = NULL;  //pointer to array of buffers.
    kds->last_i = 0;

    kds->meta_file = malloc(sizeof(struct mmbuf_obj));
    if (kds->mode=='w')
    {
        kds->state = STATE_RAW;
        rtrie__open(&kds->rtrie);
        dpref__open(&kds->dref, kds->dpref_file_path, kds->mode);

        mmbuf__setup(kds->meta_file, kds->meta_file_path, "w");

    }
    else if (kds->mode=='r')
    {
        kds->state = STATE_CONVERTED;
        ttrie__open(&kds->ttrie, kds->mode);
        ttrie__read(&kds->ttrie, kds->ttrie_file_path);
        //printf("hellolottruie %lld\n", kds->ttrie.len);
        cpref__open(&kds->cref, kds->cpref_file_path, kds->mode);

        mmbuf__setup(kds->meta_file, kds->meta_file_path, "r");
        read_global_data(kds);
    }
    else
    {
        printf("Huston we have an error invalid mode '%c' \n", mode);
        exit(EXIT_FAILURE);
    }

    return 0;
}

int kds__close(struct keyds *kds)
{
    if (kds->mode=='w')
    {
        switch (kds->state)
        {
            case STATE_RAW:
            {
                rtrie__close(&kds->rtrie);
                dpref__close(&kds->dref, false);
                break;
            }
            case STATE_DIRTY:
            {
                printf("problems, mixed state\n");
                break;
            }
            case STATE_CONVERTED:
            {
                cpref__close(&kds->cref);
                ttrie__close(&kds->ttrie);
                break;
            }
            default:
            {
                printf("ERROR: Unknown state\n");
                exit(EXIT_FAILURE);
            }
        }
    teardown_buffers(kds);
    }
    else
    {
        ttrie__close(&kds->ttrie);
        cpref__close(&kds->cref);
    }
    
    free(kds->field_name);
    kds->field_name = NULL;
    free(kds->folder_path);
    kds->folder_path =  NULL;
    free(kds->meta_file_path);
    kds->meta_file_path = NULL;
    free(kds->dpref_file_path);
    kds->dpref_file_path = NULL;
    free(kds->cpref_file_path);
    kds->cpref_file_path = NULL;
    free(kds->ttrie_file_path);
    kds->ttrie_file_path = NULL;

    mmbuf__teardown(kds->meta_file);
    return 0;
}

int kds__append(struct keyds *kds, void *value, int len, unsigned long long msg_num)
{
    bool new_entry;
    struct rtrie__node *n = rtrie__add(&kds->rtrie, value, len, &new_entry);
    //printf("Unsigned %u\n", new_entry);
    //printf("appending key msg_num %.*s %llu\n", len, (char *)value, msg_num);
    unsigned int i = 0;
    if (new_entry)
    {
        //printf("new dref buffer\n");
        i = new_dref_buffer(kds);
        n->result = i;
    }
    else
    {
        //printf("setting msg_num\n");
        i = n->result;
    }
    struct dpref__buffer *buffer = kds->buffers + i;
    dpref__append(&kds->dref, buffer, msg_num);


    //////////////////////////////
    kds->mapping_type = KDS_MAPPING_1_TO_MANY;       // Just a hack for now.
    //////////////////////////////

    return 0;
}

int kds__optimize_write(struct keyds *kds)
{
    kds->total_messages = dpref__len(&kds->dref);
    kds->num_keys = rtrie__len(&kds->rtrie);

    kds->state = STATE_DIRTY;
    write_global_data(kds);

    // CONVERT RTRIE TO TTRIE
    //printf("beginning ttrie convert\n");
    ttrie__open(&kds->ttrie, 'w');
    //printf("moo\n");
 
    //////////////////// Iteration
    /*
    rtrie__iter(&kds->rtrie);
    unsigned char xactual_key[STACK_SIZE];
    int actual_keylen;
    while (1)
    {
        struct rtrie__node *n = rtrie__next(&kds->rtrie);
        if (n==NULL)
            break;
        rtrie__fullkey(&kds->rtrie, xactual_key, &actual_keylen);
        xactual_key[actual_keylen] = '\0';
        printf("BLAH BLAH BLAH %s %u \n", xactual_key, n->result);
    }
    */
    ////

    ttrie__convert(&kds->ttrie, &kds->rtrie);
    //printf("Finished convert\n");
    rtrie__close(&kds->rtrie);
    
    //printf("Closed rtrie\n");
    
    flush_dpref_buffers(kds);         //Closing buffers, keeping them around to get meat data.

    dpref__close(&kds->dref, false);
    
    // CONVERT DPREF TO CPREF
    dpref__open(&kds->dref, kds->dpref_file_path, 'r');
    cpref__open(&kds->cref, kds->cpref_file_path, 'w');
    
    struct ttrie__iterator iter;
    ttrie__iter(&iter, &kds->ttrie);

    int actual_key_len;
    char actual_key[256];
    unsigned long long *stored_value;
    while (true)
    {
        // Need to do cpref
        int rc = ttrie__next_node(&iter, actual_key, &actual_key_len, &stored_value);
        if (rc != 0)
        {
            printf("Finished iterating over \n");
            break;
        }

        //printf("Got ttrie key='%.*s' with value='%llu'\n", actual_key_len, actual_key, *stored_value);
        unsigned long long offset = kds->buffers[*stored_value].first_page_offset;
        unsigned long long num_msgs = kds->buffers[*stored_value].global_index;
        //printf("CCCCCCCCCCCCCCCCCCCCCC %llu %llu \n", num_msgs, kds->total_messages);
        *stored_value = convert_dpref_to_cpref(&kds->cref, &kds->dref, offset, num_msgs, kds->total_messages);
    }

    dpref__close(&kds->dref, true);
    
    ttrie__write(&kds->ttrie, kds->ttrie_file_path);
    kds->state = STATE_CONVERTED;
    return 0;
}

unsigned long long kds__len(struct keyds *k)
{
    if (k->mode=='w' && k->state==STATE_RAW)
        return rtrie__len(&k->rtrie);
    else
        return ttrie__len(&k->ttrie);
}

unsigned long long kds__total_refs(struct keyds *k)
{
    if (k->mode=='w' && k->state==STATE_RAW)
        return k->dref.num_msgs;
    else
        return k->total_messages;
}

int kds__contains(struct keyds *k, void *key, int key_len)                              // You supply the key, we return true or false.
{
    // 1 if contains 0 if not.
    if (k->state==STATE_DIRTY)
        return 1;
    else if (k->mode=='w' && k->state==STATE_RAW)
        return rtrie__get(&k->rtrie, key, key_len)!=NULL;
    else
    {
        unsigned long long result;
        return ttrie__get(&k->ttrie, key, key_len, &result)==0;
    }
}

int kds__get_key_i(struct keyds *k, unsigned long long i, void *key, int *key_len)     // You supply the space we try to fill it.
{
    if (k->state==STATE_DIRTY)
        return 1;
    else if (k->mode=='w' && k->state==STATE_RAW)
    {
        rtrie__iter(&k->rtrie);
        struct rtrie__node *n;
        unsigned long long j;
        for (j=0; j<=i; ++j)
        {
            n = rtrie__next(&k->rtrie);
        }
        if (j==i)
        {
            rtrie__fullkey(&k->rtrie, key, key_len);
            return 0;
        }
        else
            return 1;
    }
    else
    {   
        struct ttrie__iterator tmp_iter;
        //printf("address of ttrie %p \n", &k->ttrie);
        ttrie__iter(&tmp_iter, &k->ttrie);
        unsigned long long *value;
        unsigned long long j;
        for (j=0; j<=i; ++j)
        {
            int rc = ttrie__next_node(&tmp_iter, key, key_len, &value);
            if (rc==1)
                break;
        }
        //printf("kds__get_key_i, key= %.*s\n", *key_len, key);
        ttrie__close_iter(&tmp_iter);
        if (j-1==i)
        {
            //printf("kds__get_key_i: j,i  %llu %llu\n", j ,i);
            return 0;
        }
        else
            return 1; 
    }
    return 1;
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//Wrappers

//  We have wrappers to allow for the fact that cpref is not the only way todo this. 
//  In other senoira more efficent ways should be used.  But we need an external common interface.

/*
q->len(q)
q->geti(q, i)
q->next(q)
q->prev(q)
q->ge(q, ref)
q->le(q, ref)
*/

//////////////////////////////////////////////////////////////

signed long long len_cpref_wrapper(struct query_context *q)
{
    return cpref__len(q->self);   
}

signed long long next_cpref_wrapper(struct query_context *q)
{
    return cpref__next(q->self);   
}

signed long long prev_cpref_wrapper(struct query_context *q)
{
    return cpref__prev(q->self);
}

signed long long geti_cpref_wrapper(struct query_context *q, signed long long i)
{
    return cpref__get(q->self, i);
}

signed long long ge_cpref_wrapper(struct query_context *q, signed long long ref)
{
    return cpref__ge_ref(q->self, ref);
}

signed long long le_cpref_wrapper(struct query_context *q, signed long long ref)
{
    return cpref__le_ref(q->self, ref);
}
//////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

signed long long null_query(struct query_context *q)
{
    printf("WARNING: PREVIOUS QUERY FAILED, dirty query context.\n");
    return -1;
}

signed long long null_query1(struct query_context *q, signed long long x)
{
    return null_query(q);
}

// Should I define null callbacks instead????
void kds__setup_query(struct query_context *q, struct keyds *k)
{
    if (k->mode!='r')
    {
        printf("Only currently support querying in read mode");
        exit(EXIT_FAILURE);
    }
    q->refs_type = -1; 
    q->self = NULL;
    q->len = NULL;
    q->geti = NULL;
    q->next = NULL;
    q->prev = NULL;
    q->ge = NULL;
    q->le = NULL;

    //printf("!!!!!!!!!!!!!!!!!!!ttrie address = %p \n", &(q->kds_data->ttrie));
    q->kds_data = k;
    //printf("!!!!!!!!!!!!!!!!!!!ttrie address = %p \n", &(q->kds_data->ttrie));
}

int kds__lookup(struct query_context *q, char *key, int n)
{
    unsigned long long result;
    //printf("ttrie address = %p \n", &(q->kds_data->ttrie));
    int rc = ttrie__get(&(q->kds_data->ttrie), key, n, &result);
    if (rc!=0)
    {
        //printf("Not found: key='%.*s'\n", n, key);
        q->len = null_query;
        q->geti = null_query1;
        q->next = null_query;
        q->prev = null_query;
        q->ge = null_query1;
        q->le = null_query1;
        return 1;
    }
    
    //Assume cpref for now
    struct cpref__stream *cstream = malloc(sizeof(struct cpref__stream));
    //printf("*****kds__lookup ttrie result %lld \n", result);
    cpref__setup_read_stream(cstream, &q->kds_data->cref, result);
    //printf("*****kds__lookup %lld \n", cpref__len(cstream));
    q->self = cstream;

    q->len = len_cpref_wrapper;
    q->geti = geti_cpref_wrapper;
    q->next = next_cpref_wrapper;
    q->prev = prev_cpref_wrapper;
    q->ge = ge_cpref_wrapper;
    q->le = le_cpref_wrapper;
    return 0;
}

void kds__teardown_query(struct query_context *q)
{
    if (q->self != NULL)
    {
        cpref__close_stream(q->self);
        free(q->self);
        q->self = NULL;
    }
    q->refs_type = -1;
    q->self = NULL;
    q->len = NULL;
    q->geti = NULL;
    q->next = NULL;
    q->prev = NULL;
    q->ge = NULL;
    q->le = NULL;
}
