// Nicer interface for abstracting many underlying things
//pointer_to_struct_methods = setup()
//p->get()
//p->set()
//p->close()

// one to one with all elements covered
// less than 256 elements.    char map with index.

// two elements all
// one element some.

// all - special

//Writing
// We have a set of keys, with meta data.
// We choice what ref stores.
// We have a key.
// We get the meta data.
// We get an offset of the dis file.
// We add to the correct ref store.
// We return a magic number.

//Reading
// We get a magic number.
// We use this to create a context.
// We expose a generic api to the outside world.


struct ref_context
{
    int refs_type;
    void *self;                                     //<--- arbitary data
    signed long long *next(struct ref_context*);   
    signed long long *prev(struct ref_context*);
    signed long long *get(struct ref_context*, signed long long);
    signed long long *ge(struct ref_context*, signed long long);
    signed long long *le(struct ref_context*, signed long long);
    void *close(struct ref_context*)
};

struct ref_context *get_refs(unsigned long long refs_id);
struct ref_context *get_refs(unsigned long long refs_id)
{
    // 
    // get the type
    // get the ref
    // set self
};


//wrappers. From ref_context to cpref calls

signed long long wrapped_cpref__next(struct ref_context*);   
signed long long wrapped_cpref__prev(struct ref_context*);
signed long long wrapped_cpref__get(struct ref_context*, signed long long);
signed long long wrapped_cpref__ge_ref(struct ref_context*, signed long long);
signed long long wrapped_cpref__le_ref(struct ref_context*, signed long long);



q = query_context
setup(q, kds)

q->geti(q, i)
q->prev(q)
q->next(q)
q->ge(q, ref)
q->le(q, ref)



teardown(q)