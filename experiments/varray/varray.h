#ifndef _varray_H_
#define _varray_H_

// Doubles in size at correct moment.
// Provides a pointer that is consistant.

struct varray__obj
{
    unsigned long long size;
    //?? type information
};

void varray__new(struct varray__obj *va);      Could set sizeof here once and for all.
void varray__append(struct varray__obj *va, thing?????????); varidic marco capture sizeof somehow.

// Would love this to be in a context that disallowed other.
void varray__alloc(struct varray__obj *va), like append but instead returna pointer where the thing could be put.  This pointer must be used before the next append realloc problem.

thing varray__pop(struct varray__obj *va, );  // Requires copy or free?????????
void varray__len(struct varray__obj *va);
void varray__geti(struct varray__obj *va);
void varray__contains(struct varray__obj *va);
void varray__del(struct varray__obj *va);

#endif //_varray_H_