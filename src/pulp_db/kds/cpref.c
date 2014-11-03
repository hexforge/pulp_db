#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "cpref.h"
#include "mmbuf.h"

static void print_stream_details(struct cpref__stream *stream);
static void print_stream_details(struct cpref__stream *stream)
{
    printf("mode=%c \n", stream->mode);
    printf("num_blocks=%d \n", stream->num_blocks);
    printf("meta_start_offset=%lld \n", stream->meta_start_offset);
    printf("meta_block_i=%d \n", stream->meta_block_i);
    printf("ref_start_offset=%lld \n", stream->ref_start_offset);
    printf("ref_block_i=%d \n", stream->ref_block_i);
    printf("nmsgs=%lld \n", stream->nmsgs);
    printf("low_ref=%lld \n", stream->low_ref);
    printf("high_ref=%lld \n", stream->high_ref);
    printf("current_i=%lld \n", stream->current_i);                   // Next free ref
    printf("current_block_ref_low=%lld \n", stream->current_block_ref_low);
    printf("current_block_ref_high=%lld \n", stream->current_block_ref_high);
}

static signed int calcuate_num_blocks(unsigned long long total_msgs);
static signed int calcuate_num_blocks(unsigned long long total_msgs)
{
    signed int num_meta_blocks = (total_msgs / CPREF_SIZE_OF_ARRAY);
    if (total_msgs%CPREF_SIZE_OF_ARRAY > 0)
        num_meta_blocks += 1;

    return num_meta_blocks;
}

static unsigned long long calculate_stream_data_size(unsigned long long total_msgs);
static unsigned long long calculate_stream_data_size(unsigned long long total_msgs)
{
    return sizeof(struct cpref__stream);
}

static unsigned long long calculate_ref_size(unsigned long long total_msgs);
static unsigned long long calculate_ref_size(unsigned long long total_msgs)
{
    return sizeof(unsigned long long) * total_msgs;
}

static unsigned long long calculate_meta_size(unsigned long long total_msgs);
static unsigned long long calculate_meta_size(unsigned long long total_msgs)
{
    signed int num_meta_blocks = calcuate_num_blocks(total_msgs);
    return (num_meta_blocks * sizeof(struct cpref__meta_block));
}

static unsigned long long calculate_required_space(unsigned long long total_msgs);
static unsigned long long calculate_required_space(unsigned long long total_msgs)
{
    unsigned long long space_required = 0;
    space_required += calculate_stream_data_size(total_msgs);
    space_required += calculate_meta_size(total_msgs);
    space_required += calculate_ref_size(total_msgs);
    return space_required;
}

static struct cpref__meta_block *get_meta_block(struct cpref__stream *stream, signed int block_id);
static struct cpref__meta_block *get_meta_block(struct cpref__stream *stream, signed int block_id)
{
    struct cpref__meta_block *block;
    assert (block_id < stream->num_blocks);
    block = ((struct cpref__meta_block*) (stream->allocated_area + stream->meta_start_offset)) + block_id;
    stream->meta_block_i = block_id;
    stream->current_block_ref_low = block->min;
    stream->current_block_ref_high = block->max;
    return block;
}

static signed int search_metas_for_i(struct cpref__stream *stream, unsigned long long i, signed int low_block_id, signed int high_block_id);
static signed int search_metas_for_i(struct cpref__stream *stream, unsigned long long i, signed int low_block_id, signed int high_block_id)
{
    if (i < stream->nmsgs)
        return i & ~(CPREF_SIZE_OF_ARRAY-1); // Subelements are position within the array. This is a power of two
    return -1;
}

static signed int check_buffered_block_for_ref(struct cpref__stream *stream, signed long long ref);
static signed int check_buffered_block_for_ref(struct cpref__stream *stream, signed long long ref)
{
    if (stream->current_block_ref_low == -1)
        return -1;

    if (stream->current_block_ref_high == -1)
        return -1;

    if (ref <= stream->current_block_ref_high && ref >= stream->current_block_ref_low)
        return stream->meta_block_i;
    return -1;
}

static signed int search_metas_for_ref(struct cpref__stream *stream, signed long long ref, signed int low_block_id, signed int high_block_id);
static signed int search_metas_for_ref(struct cpref__stream *stream, signed long long ref, signed int low_block_id, signed int high_block_id)
{
    if (low_block_id == -1)
    {
        low_block_id = 0;
    }

    if (high_block_id == -1)
    {
        high_block_id = stream->num_blocks -1;
    }
    //printf("low %d high %d \n", low_block_id, high_block_id);

    signed int pivot_id = 0;
    struct cpref__meta_block *block = NULL;

    if (low_block_id > high_block_id)
    {
        //printf("bad call to search_metas_for_ref low>high \n");
        exit(EXIT_FAILURE); 
    }

    while (low_block_id <= high_block_id)
    {
        pivot_id = (high_block_id + low_block_id)/2;

        //printf("pivot %d low %d high %d \n", pivot_id, low_block_id, high_block_id);
        block = get_meta_block(stream, pivot_id);
        //printf("block pointer %p\n", block);

        //printf("min=%lld max=%lld for ref=%lld\n", block->min, block->max, ref);
        if (ref < block->min)
        {
            high_block_id = pivot_id -1;
        }
        else if (ref > block->max)
        {
            low_block_id = pivot_id + 1;
        }
        else
        {
            return pivot_id;
        }   
    }
    return -1;
}


//Think about binary search
//when not found end up lower or high depends on number of iterations?
// xo(xx) or (xox) (xx)ox
// 0123, or 1234 always points to 2nd location from left.
// Better than binary search

static signed long long binsearch_refblock_ge(signed long long *refs, signed long long ref);
static signed long long binsearch_refblock_ge(signed long long *refs, signed long long ref)
{
    signed int low = 0;
    signed int high = CPREF_SIZE_OF_ARRAY; 
    signed long long p_ref = -1;
    while (low < high)
    {
        signed int p = (high+low)/2;
        p_ref = refs[p];
        //printf("fkeys %c\n", c);
        
        if (ref < p_ref)
        {
            high = p;
        }
        else if (ref > p_ref)
        {
            p_ref = refs[p+1];  // This is inefficient better doing a post check
            low = p + 1;
        }
        else
        {
            break;
        }
    }

    if (ref > p_ref)
    {
        return -1;
    }

    return p_ref;
}

static signed long long linearsearch_refblock_ge(signed long long *refs, signed long long high_ref, signed long long ref);
static signed long long linearsearch_refblock_ge(signed long long *refs, signed long long high_ref, signed long long ref)
{
    signed long long i_ref = -1;
    for (unsigned long i=0; i<CPREF_SIZE_OF_ARRAY; ++i)
    {
        i_ref = refs[i];
        if (i_ref == high_ref)
        {
            break;
        }

        if (i_ref >= ref)
        {
            return i_ref;
        }
    }

    if (i_ref >= ref)
    {
        return i_ref;
    }
    return -1;
}

static signed long long search_refblock_ge(struct cpref__stream *stream, signed int block_id, signed long long ref);
static signed long long search_refblock_ge(struct cpref__stream *stream, signed int block_id, signed long long ref)
{
    signed long long offset = CPREF_PAGE_SIZE * block_id;
    signed long long *refs = (signed long long *) (stream->allocated_area + stream->ref_start_offset + offset);

    if (block_id == stream->num_blocks-1)           // Last block:  Must linear search last block as may be incomplete.
    {
        return linearsearch_refblock_ge(refs, stream->high_ref, ref);
    }
    else
    {
        return binsearch_refblock_ge(refs, ref);
    }
}

static signed long long binsearch_refblock_le(signed long long *refs, signed long long ref);
static signed long long binsearch_refblock_le(signed long long *refs, signed long long ref)
{
    signed int low = 0;
    signed int high = CPREF_SIZE_OF_ARRAY; 
    signed long long p_ref;
    
    while (low < high)
    {
        signed int p = (high+low)/2;
        p_ref = refs[p];
        //printf("ref %lld\n", p_ref);
        
        if (ref < p_ref)
        {
            p_ref = refs[p-1]; // This is inefficient better doing a post check, 
            high = p;
        }
        else if (ref > p_ref)
        {
            low = p + 1;
        }
        else
        {
            break;
        }
    }

    if (ref < p_ref)
    {
        return -1;
    }

    return p_ref;
}

static signed long long linearsearch_refblock_le(signed long long *refs, signed long long high_ref, signed long long ref);
static signed long long linearsearch_refblock_le(signed long long *refs, signed long long high_ref, signed long long ref)
{
    //printf("In linear le\n");
    signed long long i_ref = -1;
    for (unsigned long i=0; i<CPREF_SIZE_OF_ARRAY; ++i)
    {
        
        i_ref = refs[i];
        //printf("00 %d %lld %lld\n", i, i_ref, high_ref);

        if (i_ref > ref)
        {
            if (i==0)
            {
                //printf("In here2\n");
                return -1;
            }
            else
            {
                //printf("In here3\n");
                i--;
                i_ref = refs[i];
            }
            break;
        }
        else if (i_ref == ref)
        {
            return i_ref;
        }

        if (i_ref == high_ref)
        {
            break;
        }
    }

    //printf("In here4 %lld\n", i_ref);
    if (i_ref <= ref)
    {
        return i_ref;
    }
    return -1;
}

static signed long long search_refblock_le(struct cpref__stream *stream, signed int block_id, signed long long ref);
static signed long long search_refblock_le(struct cpref__stream *stream, signed int block_id, signed long long ref)
{
    signed long long offset = CPREF_PAGE_SIZE * block_id;
    signed long long *refs = (signed long long *) (stream->allocated_area + stream->ref_start_offset + offset);

    if (block_id == stream->num_blocks-1)           // Last block:  Must linear search last block as may be incomplete.
    {
        //printf("12 %d %lld\n", block_id, stream->high_ref);
        assert (stream->high_ref!=-1);
        return linearsearch_refblock_le(refs, stream->high_ref, ref);
    }
    else
    {
        //printf("13 %d\n", block_id);
        return binsearch_refblock_le(refs, ref);
    }
}

static void meta_block_append(struct cpref__stream *stream, signed long long low_ref, signed long long high_ref);
static void meta_block_append(struct cpref__stream *stream, signed long long low_ref, signed long long high_ref)
{
    struct cpref__meta_block *meta_block = (struct cpref__meta_block *) (stream->allocated_area + stream->meta_start_offset) + stream->meta_block_i;       ///////////////??~~~~
    (stream->meta_block_i)++;
    meta_block->min = low_ref;
    meta_block->max = high_ref;
    stream->ref_block_i += 1;
    //printf("Appending to stream block_low=%lld  block_high=%lld \n", stream->current_block_ref_low, stream->current_block_ref_high);
}

/*---
 GLOBAL METHODS
---*/
int cpref__open(struct cpref__obj *cp, const char *ref_file_path, const char mode)
{
    cp->mode = mode;
    cp->ref_file = malloc(sizeof(struct mmbuf_obj));

    cp->size = 0;
    strcpy(cp->ref_file_path, ref_file_path);    

    char mode_with_hint[3];
    if (mode=='r')
    {
        strcpy(mode_with_hint, "r");   //'rr' Read random, 'rs' sometimes want to read sequential!!!!!!!!
    }
    else if (mode=='w')
    {
        strcpy(mode_with_hint, "ws");   // Write sequential
    }
    else
    {
        printf("Bad mode %c \n", mode);
        exit(EXIT_FAILURE);
    }

    mmbuf__setup(cp->ref_file, ref_file_path, mode_with_hint);
    return 0;
}

int cpref__close(struct cpref__obj *cp)
{
    mmbuf__teardown(cp->ref_file);
    free(cp->ref_file);
    cp->ref_file = NULL;
    return 0;
}

/*
static signed long long cpref__setup_stream(struct cpref__stream *stream, struct cpref__obj *cp, unsigned long long x)
{
    stream->mode = cp->mode;
    if (stream->mode=='w')
    {
    }
    else if (stream->mode=='r')
    {
    }
    else ()
    {
        assert(2==1);
    }
}
*/

signed long long cpref__setup_write_stream(struct cpref__stream *stream, struct cpref__obj *cp, unsigned long long total_msgs)
{
	stream->mode = cp->mode;
	assert(stream->mode=='w');

    unsigned long long space_required = calculate_required_space(total_msgs);
    //printf("LOOK AT ME %llu %lld \n", space_required, total_msgs);

    signed long long offset = cp->size;
    cp->size += space_required;
    stream->allocated_area = mmbuf__alloc(cp->ref_file, space_required);
    
    stream->num_blocks = calcuate_num_blocks(total_msgs);

    stream->ref_start_offset = calculate_stream_data_size(total_msgs) + calculate_meta_size(total_msgs);
    stream->ref_block_i = 0;

    stream->meta_start_offset = calculate_stream_data_size(total_msgs);
    stream->meta_block_i = 0;

    stream->nmsgs = total_msgs;
    stream->low_ref = -1;
    stream->high_ref = -1;

    stream->current_i = -1;
    stream->current_block_ref_low = -1;
    stream->current_block_ref_high = -1;

    //print_stream_details(stream);
    return offset;
}

void cpref__setup_read_stream(struct cpref__stream *stream, struct cpref__obj *cp, unsigned long long offset)
{
    stream->mode = cp->mode;
    assert(stream->mode=='r');
    stream->allocated_area = cp->ref_file->map + offset;
    struct cpref__stream *tmp_stream = (struct cpref__stream *) stream->allocated_area;
    stream->num_blocks = tmp_stream->num_blocks;
    stream->meta_start_offset = tmp_stream->meta_start_offset;
    stream->meta_block_i = 0;
    stream->ref_start_offset  = tmp_stream->ref_start_offset;
    stream->ref_block_i = 0;
    stream->nmsgs = tmp_stream->nmsgs;
    stream->low_ref = tmp_stream->low_ref;
    stream->high_ref = tmp_stream->high_ref;
    stream->current_i = -1;
    stream->current_block_ref_low = -1;
    stream->current_block_ref_high = -1;
}


signed long long last_written_ref(struct cpref__stream *stream)
{
    signed long long value;
    if (stream->current_i >= 0)
    {
        signed long long *p = (signed long long *)(stream->allocated_area + stream->ref_start_offset) + stream->current_i;
        value = *p;
    }
    else
    {
        value = -1;
    }
    return value;
}

void cpref__close_stream(struct cpref__stream *stream)
{
    if (stream->mode == 'w')
    {
        // We may not have written the high and low yet.
        if (((stream->current_i + 1)&(CPREF_SIZE_OF_ARRAY-1)) != 0)   // Every x messages
        {
            stream->current_block_ref_high = last_written_ref(stream);
            meta_block_append(stream, stream->current_block_ref_low, stream->current_block_ref_high);
        }

        //printf("I am here\n");
        stream->low_ref = cpref__get(stream, 0);
        stream->high_ref = cpref__get(stream, stream->nmsgs-1);
        //printf("erm=%lld\n", cpref__get(stream, stream->nmsgs-1));
        assert(stream->low_ref != -1);
        assert(stream->high_ref != -1);

        struct cpref__stream *tmp_stream = (struct cpref__stream *) stream->allocated_area;

        tmp_stream->allocated_area = 0;
        tmp_stream->mode = '\0';
        tmp_stream->num_blocks = stream->num_blocks;
        tmp_stream->meta_start_offset = stream->meta_start_offset;
        tmp_stream->meta_block_i = 0;
        tmp_stream->ref_start_offset = stream->ref_start_offset;
        tmp_stream->ref_block_i = 0;
        tmp_stream->nmsgs = stream->nmsgs;
        tmp_stream->low_ref = stream->low_ref;
        tmp_stream->high_ref = stream->high_ref;
        tmp_stream->current_i = -1;
        tmp_stream->current_block_ref_low = -1;
        tmp_stream->current_block_ref_high = -1;
    }
}

void cpref__write_stream(struct cpref__stream *stream, signed long long *refs, int n)
{
    assert(stream->mode == 'w');
    int i;
    signed long long value = -1;

    signed long long *p_to_first = ((signed long long *)(stream->allocated_area + stream->ref_start_offset));

    for (i=0; i<n; ++i)
    {
        value = refs[i];
        stream->current_i++;
        *(p_to_first + stream->current_i) = value;
        //printf("value %lld to position %lld \n", value, stream->current_i);
        
        // If the next one will be at the start of a new block
        if (((stream->current_i + 1)&(CPREF_SIZE_OF_ARRAY-1)) == 0)   // Every x messages    
        {
            stream->current_block_ref_high = last_written_ref(stream);
            meta_block_append(stream, stream->current_block_ref_low, stream->current_block_ref_high);
        }
        else if (((stream->current_i + 1)&(CPREF_SIZE_OF_ARRAY-1)) == 1)   // Every x messages
        {
            stream->current_block_ref_low = value;   
        }
    }
    //unsigned long long *xs = ((unsigned long long *)(stream->allocated_area + stream->ref_start_offset));
    //for (int i=0; i<=stream->current_i; ++i)
    //{
    //    printf("0Checking i=%d ref=%lld \n", i, *(xs + i));
    //}



    // Check the writing
    //unsigned long long *x = ((unsigned long long *)(stream->allocated_area + stream->ref_start_offset));
    //for (int i=0; i<=stream->current_i; ++i)
    //{
    //    printf("1Checking i=%d ref=%lld \n", i, *(x + i));
    //}
}

signed long long cpref__len(struct cpref__stream *stream)
{
    //printf("<<<<<<<<<<<<<<<<<<<<<\n");
    //print_stream_details(stream);
    //printf(">>>>>>>>>>>>>>>>>>>>>\n");
    return stream->nmsgs;
}

signed long long cpref__get(struct cpref__stream *stream, unsigned long long i)
{
    assert (i >= 0);
    assert (i <= stream->nmsgs);
    stream->current_i = i;
    return *((unsigned long long *) (stream->allocated_area + stream->ref_start_offset) + i);         ///////////////??~~~~
}

signed long long cpref__next(struct cpref__stream *stream)
{
    if (stream->current_i > (signed long long ) (stream->nmsgs -1)) 
        return -1;
    else if (stream->current_i == (signed long long ) (stream->nmsgs -1)) 
    {
        stream->current_i++;
        return -1;
    }

    stream->current_i++;
    return cpref__get(stream, stream->current_i);
}

signed long long cpref__prev(struct cpref__stream *stream)
{
    if (stream->current_i < 0)
        return -1;
    else if (stream->current_i == 0)
    {
        stream->current_i--;
        return -1;
    }

    stream->current_i--;
    return cpref__get(stream, stream->current_i);
}

signed long long cpref__ge_ref(struct cpref__stream *stream, signed long long ref)
{
    signed int block_id;
    block_id = -1 ;//check_buffered_block_for_ref(stream, ref);
    
    // Search around metas
    if (block_id == -1)
    {
        block_id = search_metas_for_ref(stream, ref, -1, -1);      // Can limit the scope here no need to use -1
    }

    // Search around where we left in the above search.  To cover the gaps between meta blocks
    if (block_id == -1)
    {
        signed int near_block = MAX(0, stream->meta_block_i-1);
        signed int far_block = MIN(stream->meta_block_i+1, stream->num_blocks-1);
        for (signed int i=near_block; i<=far_block; ++i)
        {
            struct cpref__meta_block *block = get_meta_block(stream, i);
            if (block->min > ref)
                return block->min;
        }
        return -1;
    }
    else
    {
        return search_refblock_ge(stream, block_id, ref);
    }
}

signed long long cpref__le_ref(struct cpref__stream *stream, signed long long ref)
{
    signed int block_id;

    // Check buffered blocks
    block_id = check_buffered_block_for_ref(stream, ref);

    // Search around metas
    if (block_id == -1)
    {
        block_id = search_metas_for_ref(stream, ref, -1, -1);   // Can limit the scope here no need to use -1
    }

    // Search around where we left in the above search.  To cover the gaps between meta blocks
    if (block_id == -1)
    {
        signed int near_block = MAX(0, stream->meta_block_i-1);
        signed int far_block = MIN(stream->meta_block_i+1, stream->num_blocks-1);
        for (signed int i=far_block; i>=near_block; --i)
        {
            struct cpref__meta_block *block = get_meta_block(stream, i);
            if (block->max < ref)
                return block->max;
        }
        return -1;
    }
    else
    {
        return search_refblock_le(stream, block_id, ref);
    }
}
