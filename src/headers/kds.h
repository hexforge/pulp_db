#ifndef _KDS_H_
#define _KDS_H_ 

#include "mmbuf.h"
#include "rtrie.h"
#include "dpref.h"
#include "ttrie.h"
#include "cpref.h"

#define STATE_RAW 0
#define STATE_DIRTY 1
#define STATE_CONVERTED 2

#define KDS_MAPPING_UNKNOWN 0
#define KDS_MAPPING_1_TO_1 2
#define KDS_MAPPING_1_TO_MANY 4

#define KDS_META_SUFFIX ".meta"
#define KDS_DPREF_SUFFIX ".dpref"
#define KDS_CPREF_SUFFIX ".cpref"
#define KDS_TTRIE_SUFFIX ".ttrie"
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Plan of attack.
// Do stage one first......................................... DONE
// Stage two ttrie.  We need iter method for this............. DONE
// Need to perist global meta data............................ 
//     fieldname, Number of keys, total number of messages. one_to_one, one_to_many.
// Stage 3, cref needs an interface put infront of it.........
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

struct keyds
{
    char state;

    // These are stored in .meta
    char mode;
    char mapping_type;                     
    unsigned long long total_messages;               // These are duplicated with what is stored below. Is this needed? remove
    unsigned long long num_keys;                     // These are duplicated with what is stored below. Is this needed? remove
 
    // These are calculated at run time.
    char *field_name;
    char *folder_path;
    char *meta_file_path;
    char *dpref_file_path;
    char *cpref_file_path;
    char *ttrie_file_path;

    // WRITE: Stage one.
    struct rtrie__view rtrie;
    struct dpref__obj dref;
    struct dpref__buffer *buffers;  //pointer to array of buffers.
    unsigned int last_i;            // Last_i used in buffers
    
    // WRITE: Stage two.
    // READ: 
    struct mmbuf_obj *meta_file;
    struct ttrie__obj ttrie;
    struct cpref__obj cref;
};

/**
 * General methods
**/
int kds__open(struct keyds *kds, char *folder_path, char *fieldname, char mode);
int kds__close(struct keyds *kds);

/* WRITTING
   A field_name coresponds to an rtrie and a dpref. e.g. wSrcTime
   Each field_value corresponds to an int. e.g. 12:00:12.123 : 12
   each int coreresponds to a element in an array of dpref buffers 12 : <dpref_buffer: msg_refs 1, 8, 13>

   On pulp__optimize
   for each field rtrie convert it into ttrie
       iterate each element of the ttrie.
           convert the dpref to something like cpref. depending on one to one, percentage of population etc.
           Convert the index to a initial offset kinda value.
       persist the ttrie.
 */

/** 
 *  Write methods
**/
int kds__append(struct keyds *kds, void *value, int val_len, unsigned long long msg_num);
int kds__optimize_write(struct keyds *kds);

/* READING

   BLAH
 */

/**
 *  Read methods
**/
unsigned long long kds__len(struct keyds *k);                // This should be a passthrough to rtrie in w mode, in read mode can just return this. OR should we just pass through as well?
unsigned long long kds__total_refs(struct keyds *k);         // This should be a passthrough to dref in w mode. in read mode can just return the value. OR shold we just pass through as well?
int kds__get_key_i(struct keyds *k, unsigned long long i, void *key, int*key_len);  
int kds__contains(struct keyds *k, void *key, int key_len);

struct query_context
{
    signed int refs_type;
    void *self;                                                      //<--- arbitrary data probably cpref
    struct keyds *kds_data;
    signed long long (*len)(struct query_context*);  
    signed long long (*geti)(struct query_context*, signed long long);
    signed long long (*next)(struct query_context*);   
    signed long long (*prev)(struct query_context*);
    signed long long (*ge)(struct query_context*, signed long long);
    signed long long (*le)(struct query_context*, signed long long);
};

void kds__setup_query(struct query_context *q, struct keyds *k);
int kds__lookup(struct query_context *q, char *key, int n);

// Once we have looked up a key, we can use the query context and call its methods.
/*
q->len(q)
q->geti(q, i)
q->next(q)
q->prev(q)
q->ge(q, ref)
q->le(q, ref)
*/

void kds__teardown_query(struct query_context *q);


#endif

