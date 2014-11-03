#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "pref.h"

static signed long long new_page(struct pref__obj *const p, char **page_pp);
static signed long long new_page(struct pref__obj *const p, char **page_pp)
{
    *page_pp = mmbuf__alloc(p->ref_file, PAGE_SIZE);
    signed long long old_offset = p->offset;
    p->offset += PAGE_SIZE;
    return (old_offset);
}

static int flush_buffer(struct pref__obj *p, struct pref__buffer *buf);
static int flush_buffer(struct pref__obj *p, struct pref__buffer *buf)
{
    if (buf->refs_index < 0)
        return 1;
    
    if (buf->refs_index<NUM_REF_PER_PAGE)
    {
        for (int i=buf->refs_index; i<NUM_REF_PER_PAGE; ++i)
        {
            buf->refs[i] = -1;
        }
    }
    
    char *new_page_p;
    signed long long new_offset = new_page(p, &new_page_p);
    
    buf->refs[NEXT_PAGE_INDEX] = new_offset;
    buf->refs = (signed long long *) new_page_p;
    buf->current_page_offset = new_offset;
    buf->refs_index = 0;
    return 0;
}

static int final_flush_buffer(struct pref__obj *p, struct pref__buffer *buf);
static int final_flush_buffer(struct pref__obj *p, struct pref__buffer *buf)
{
    if (buf->refs_index < 0)
        return 1;

    if (buf->refs_index<NUM_REF_PER_PAGE)
    {
        for (int i=buf->refs_index; i<NUM_REF_PER_PAGE; ++i)
        {
            buf->refs[i] = -1;
        }
    }

    buf->refs[NEXT_PAGE_INDEX] = -1;
    buf->current_page_offset = -1;
    buf->refs_index = 0;
    return 0;
}

static int load_page(const struct pref__obj *p, struct pref__buffer *const buf, const signed long long index);
static int load_page(const struct pref__obj *p, struct pref__buffer *const buf, const signed long long index)
{
    mmbuf__get_data(p->ref_file, (void **) &buf->refs, index, PAGE_SIZE);
    return 0;
}

static int load_first_page(const struct pref__obj *p, struct pref__buffer *const buf);
static int load_first_page(const struct pref__obj *p, struct pref__buffer *const buf)
{
    int rc = load_page(p, buf, buf->first_page_offset);
    buf->global_index = 0;
    return rc;
}

static int load_next_page(const struct pref__obj *p, struct pref__buffer *const buf);
static int load_next_page(const struct pref__obj *p, struct pref__buffer *const buf)
{
    int rc;
    if (buf->refs[NEXT_PAGE_INDEX] == -1)
    {
        rc = 1;
    }
    else
    {
        rc = load_page(p, buf, buf->refs[NEXT_PAGE_INDEX]);
        buf->global_index += NEXT_PAGE_INDEX;
    }
    return rc;
}

static int pref__setup_rbuffer(const struct pref__obj *p, struct pref__buffer *const buf, const signed long long offset);
static int pref__setup_rbuffer(const struct pref__obj *p, struct pref__buffer *const buf, const signed long long offset)
{
    buf->refs_index = 0;  // We never use this here
    buf->refs = ( (signed long long *) malloc(sizeof(signed long long [SIZE_OF_ARRAY])) );

    int error = 0;
    if (p->mode=='r')
    {
        buf->first_page_offset = offset;
        buf->current_page_offset = offset;
    }
    else
    {
        error = -1;
    }
    load_first_page(p, buf);

    return error;    // zero on success else positive error code.
}

static int pref__setup_wbuffer(struct pref__obj *const p, struct pref__buffer *const buf);
static int pref__setup_wbuffer(struct pref__obj *const p, struct pref__buffer *const buf)
{
    buf->refs_index = 0;
    buf->global_index = -1;
    buf->refs = ( (signed long long *) malloc(sizeof(signed long long [SIZE_OF_ARRAY])) );

    int error = 0;
    if (p->mode=='w')
    {
        char *new_page_p;
        buf->first_page_offset = new_page(p, &new_page_p);
        buf->refs = (signed long long *) new_page_p;
        buf->current_page_offset = buf->first_page_offset;
    }
    else
    {
        error = -1;
    }
    return error;    // zero on success else positive error code.
}

/*---
 GLOBAL METHODS
---*/
int pref__open(struct pref__obj *const p, const char *ref_file_path, const char mode)
{
    p->mode = mode;
    p->ref_file = malloc(sizeof(struct mmbuf_obj));
    p->offset = 0;
    strcpy(p->ref_file_path, ref_file_path);    

    char mode_with_hint[3];
    if (mode=='r')
        strcpy(mode_with_hint, "r");   //'rr' Read random, 'rs' sometimes want to read sequential!!!!!!!!
    else if (mode=='w')
        strcpy(mode_with_hint, "ws");   // Write sequential
    else
    {
        printf("Bad mode %c \n", mode);
        exit(EXIT_FAILURE);
    }
    
    mmbuf__setup(p->ref_file, ref_file_path, mode_with_hint);
    return 0;
}

int pref__close(const struct pref__obj * p)
{
    mmbuf__teardown(p->ref_file);
    free(p->ref_file);
    return 0;
}

// zero on success else positive error code.
int pref__setup_buffer(struct pref__obj *const p, struct pref__buffer *const buf, ...)  
{
    if (p->mode=='w')
    {
        return pref__setup_wbuffer(p, buf);
    }
    else if (p->mode=='r')
    {
        va_list ap;
        va_start(ap, buf);
        signed long long offset =  va_arg(ap, signed long long);
        va_end(ap);
        return pref__setup_rbuffer(p, buf, offset);
    }
    else
    {
        return -1;
    }
}

int pref__teardown_buffer(struct pref__obj *const p, struct pref__buffer *const buf)  
{
    if (buf->refs_index > 0)
    {
        //printf("Flushing buffer\n");
        final_flush_buffer(p, buf);
    }
    int error = 0;
    return error;   // zero on success else positive error code.
}


/*---
 WRITE METHODS
---*/
signed long long pref__append(struct pref__obj *const p, struct pref__buffer *const buf, const signed long long ref)
{

    buf->refs[buf->refs_index] = ref;
    buf->refs_index += 1;
    buf->global_index += 1;

    if (buf->refs_index >= NUM_REF_PER_PAGE)
    {
        flush_buffer(p, buf);
    }
    return 0;
}


/*---
 READ METHODS
---*/
signed long long pref__geti(const struct pref__obj *p, struct pref__buffer *const buf, const unsigned long long index)
{
    // If index is negative we are in trouble.
    bool not_found = true;
    while (not_found)
    {
        if (index < buf->global_index)
        {
            load_first_page(p, buf);
        }
        else if ( (index >= buf->global_index ) && (index < (buf->global_index+NUM_REF_PER_PAGE)) )
        {
            return buf->refs[index - buf->global_index];
        }
        else if (index >= buf->global_index+NUM_REF_PER_PAGE)
        {
            int error = load_next_page(p, buf);
            if (error)
                return -1;
        }
    }
    return -1;
}
