#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "dpref.h"

static signed long long new_page(struct dpref__obj *const p);
static signed long long new_page(struct dpref__obj *const p)
{
    return mmbuf__pos_alloc(p->ref_file, DPREF_PAGE_SIZE);
}

static int flush_buffer(struct dpref__obj *p, struct dpref__buffer *buf, const bool final);
static int flush_buffer(struct dpref__obj *p, struct dpref__buffer *buf, const bool final)
{
    if (buf->refs_index < 0)
        return 1;
    
    signed long long *refs_page_array = (signed long long *) mmbuf__pos_tmpptr(p->ref_file, buf->current_page_offset);

    if (buf->refs_index < (int)DPREF_NUM_REF_PER_PAGE)
    {
        for (unsigned long i=buf->refs_index; i<(int)DPREF_NUM_REF_PER_PAGE; ++i)
            refs_page_array[i] = -1;
    }
    
    if (final==true)
    {
        refs_page_array[DPREF_NEXT_PAGE_INDEX] = -1;
        buf->refs_index = -1;
    }
    else
    {
        signed long long new_offset = new_page(p);

        // Need to recalulate this as new page might have invalidated the pointer.
        refs_page_array = (signed long long *) mmbuf__pos_tmpptr(p->ref_file, buf->current_page_offset);
        refs_page_array[DPREF_NEXT_PAGE_INDEX] = new_offset;
        
        buf->current_page_offset = new_offset;
        buf->refs_index = 0;
    }
    return 0;
}

static int load_page(const struct dpref__obj *p, struct dpref__buffer *const buf, const signed long long index);
static int load_page(const struct dpref__obj *p, struct dpref__buffer *const buf, const signed long long index)
{
    buf->current_page_offset = index;
    return 0;
}

static int load_first_page(const struct dpref__obj *p, struct dpref__buffer *const buf);
static int load_first_page(const struct dpref__obj *p, struct dpref__buffer *const buf)
{
    int rc = load_page(p, buf, buf->first_page_offset);
    buf->global_index = 0;
    return rc;
}

static int load_next_page(const struct dpref__obj *p, struct dpref__buffer *const buf);
static int load_next_page(const struct dpref__obj *p, struct dpref__buffer *const buf)
{
    int rc;
    signed long long *refs_page_array = (signed long long *) ((unsigned char *) p->ref_file->map + buf->current_page_offset);
    if (refs_page_array[DPREF_NEXT_PAGE_INDEX] == -1)
    {
        rc = 1;
    }
    else
    {
        rc = load_page(p, buf, refs_page_array[DPREF_NEXT_PAGE_INDEX]);
        buf->global_index += DPREF_NUM_REF_PER_PAGE;
    }
    return rc;
}

static int dpref__setup_rbuffer(const struct dpref__obj *p, struct dpref__buffer *const buf, const signed long long offset);
static int dpref__setup_rbuffer(const struct dpref__obj *p, struct dpref__buffer *const buf, const signed long long offset)
{
    buf->refs_index = 0;  // We never use this here
      //( (signed long long *) malloc(sizeof(signed long long [DPREF_SIZE_OF_ARRAY])) );

    int error = 0;
    assert(p->mode=='r');
    buf->first_page_offset = offset;
    buf->current_page_offset = buf->first_page_offset;
    load_first_page(p, buf);

    return error;    // zero on success else positive error code.
}

static int dpref__setup_wbuffer(struct dpref__obj *const p, struct dpref__buffer *const buf);
static int dpref__setup_wbuffer(struct dpref__obj *const p, struct dpref__buffer *const buf)
{
    buf->refs_index = 0;
    buf->global_index = 0;
    int error = 0;
    assert(p->mode=='w');
    buf->first_page_offset = new_page(p);
    buf->current_page_offset = buf->first_page_offset;
    return error;    // zero on success else positive error code.
}

/*---
 GLOBAL METHODS
---*/
int dpref__open(struct dpref__obj *const p, const char *ref_file_path, const char mode)
{
    p->mode = mode;
    p->ref_file = malloc(sizeof(struct mmbuf__obj));
    p->num_msgs = 0;
    strcpy(p->ref_file_path, ref_file_path);    

    char mode_with_hint[3];
    if (mode=='r')
        strcpy(mode_with_hint, "rr");   //'rr' Read random, 'rs' sometimes want to read sequential!!!!!!!!
    else if (mode=='w')
        strcpy(mode_with_hint, "wr");   // Write sequential
    else
    {
        printf("Bad mode %c \n", mode);
        exit(EXIT_FAILURE);
    }
    
    mmbuf__setup(p->ref_file, ref_file_path, mode_with_hint);
    return 0;
}

int dpref__close(struct dpref__obj * p, bool del)
{
    if (p->ref_file)
    {
        mmbuf__teardown(p->ref_file);
        free(p->ref_file);
    }
    if (del==true)
        remove(p->ref_file_path);
    p->ref_file = NULL;
    return 0;
}

unsigned long long dpref__len(const struct dpref__obj *p)
{
    return p->num_msgs;
}

// zero on success else positive error code.
int dpref__setup_buffer(struct dpref__obj *const p, struct dpref__buffer *const buf, ...)  
{
    buf->state = DPREF_BUFFER_OPEN;
    if (p->mode=='w')
    {
        return dpref__setup_wbuffer(p, buf);
    }
    else if (p->mode=='r')
    {
        va_list ap;
        va_start(ap, buf);
        signed long long offset =  va_arg(ap, signed long long);
        va_end(ap);
        return dpref__setup_rbuffer(p, buf, offset);
    }
    else
    {
        return -1;
    }
}

int dpref__teardown_buffer(struct dpref__obj *const p, struct dpref__buffer *const buf)  
{
    if (buf->state == DPREF_BUFFER_OPEN)
    {
        if (buf->refs_index > 0)
        {
            flush_buffer(p, buf, true);
        }
    }
    buf->state = DPREF_BUFFER_CLOSED;
    int error = 0;
    return error;                        // zero on success else positive error code.
}


/*---
 WRITE METHODS
---*/
signed long long dpref__append(struct dpref__obj *const p, struct dpref__buffer *const buf, const signed long long ref)
{
    signed long long *refs_page_array = (signed long long *) ((unsigned char *) p->ref_file->map + buf->current_page_offset);
    refs_page_array[buf->refs_index] = ref;
    buf->refs_index++;
    buf->global_index++;
    p->num_msgs ++;
    //printf("setting dpref %lld\n", p->num_msgs);

    if (buf->refs_index >= (int)DPREF_NUM_REF_PER_PAGE)
    {
        flush_buffer(p, buf, false);
    }
    return 0;
}


/*---
 READ METHODS
---*/
signed long long dpref__geti(const struct dpref__obj *p, struct dpref__buffer *const buf, const unsigned long long index)
{
    // If index is negative we are in trouble.
    bool not_found = true;
    while (not_found)
    {
        /*
        if (index < buf->global_index)
        {
            load_first_page(p, buf);
        }
        else 
        */
        if ( (index >= buf->global_index ) && (index < (buf->global_index+DPREF_NUM_REF_PER_PAGE)) )
        {
            signed long long *refs_page_array = (signed long long *) ((unsigned char *) p->ref_file->map + buf->current_page_offset);
            return refs_page_array[index - buf->global_index];
        }
        else if (index >= buf->global_index+DPREF_NUM_REF_PER_PAGE)
        {
            int error = load_next_page(p, buf);
            if (error)
                return -1;
        }
    }
    return -1;
}
