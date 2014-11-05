import os
from cffi import FFI
ffi = FFI()

# This is just a placeholder for the c implementation.
# Some of it is thinking of the c and is not very pythony.
# We are thinkling alot about chunking the data.
# Stored separately to be more continuous. Meta gets queried.
# overall_meta
# meta1
# meta2
# meta3
# meta4
# meta5

# encoded_numbers1    #<--- Might be quite expensive to decode.
# encoded_numbers2    #<--- Continous
# encoded_numbers3
# encoded_numbers4
# encoded_numbers5

#meta_data["min"]
#meta_data["max"]
#meta_data["len"]
#meta_data["start_i"]

# Should never  need to worry about Order of expression 
# auto optimise gallops symmetrically.
# Say results are [1,2,3,5,......] vs [99999999].  
# Just need to merge galop through the longer.  
# Data is sorted can therefor do a bit of jump predictability :)

############################################################################################

###############################################################################################

KDS_STRUCTS_CDEF = """
struct keyds
{
    char state;
    char mode;
    char mapping_type;                     
    unsigned long long total_messages;
    unsigned long long num_keys;
    char *field_name;
    char *folder_path;
    char *meta_file_path;
    char *dpref_file_path;
    char *cpref_file_path;
    char *ttrie_file_path;
    ...;
};


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

"""

KDS_API_CDEF = """
int kds__open(struct keyds *kds, char *folder_path, char *fieldname, char mode);
int kds__close(struct keyds *kds);
int kds__append(struct keyds *kds, void *value, int val_len, unsigned long long msg_num);
int kds__optimize_write(struct keyds *kds);
unsigned long long kds__len(struct keyds *k);                // This should be a passthrough to rtrie in w mode, in read mode can just return this. OR should we just pass through as well?
unsigned long long kds__total_refs(struct keyds *k);         // This should be a passthrough to dref in w mode. in read mode can just return the value. OR shold we just pass through as well?
int kds__get_key_i(struct keyds *k, unsigned long long i, void *key, int*key_len);  
int kds__contains(struct keyds *k, void *key, int key_len);
void kds__setup_query(struct query_context *q, struct keyds *k);
int kds__lookup(struct query_context *q, char *key, int n);
void kds__teardown_query(struct query_context *q);
"""

ffi.cdef(KDS_STRUCTS_CDEF)
ffi.cdef(KDS_API_CDEF)

folder_path = os.path.dirname(__file__)
if not folder_path:
    folder_path = os.getcwd()
common_path = os.path.abspath(os.path.join(folder_path, "../../common"))
KDS_SO = ffi.verify("""#include "kds.h" """, 
                      libraries=["kds"], 
                      library_dirs=[folder_path],
                      runtime_library_dirs=[folder_path],
                      include_dirs=[folder_path, common_path],
                      extra_compile_args=["-std=c99"])

class KeyTable(object):
    def __init__(self, dirname, fieldname, mode, dumper=None, loader=None):
        self.dirname = dirname
        self.fieldname = fieldname
        #self.path = os.path.join(dirname, '{}.db'.format(self.field))
        self.mode = mode
        foo = ffi.new("struct keyds *k")
        self.c_kds_data = ffi.gc(foo, KDS_SO.kds__close)
        self.flushed = False
        self.dumper = dumper
        self.loader = loader

    def __enter__(self):
        path = self.dirname.encode('ascii')
        mode = self.mode.encode('ascii')
        fieldname = self.fieldname.encode('ascii')
        if self.mode == 'w':
            rc = KDS_SO.kds__open(self.c_kds_data, path, fieldname, mode)
            assert rc == 0
        elif self.mode == 'r':
            rc = KDS_SO.kds__open(self.c_kds_data, path, fieldname, mode)
            assert rc == 0
        else:
            raise NotImplementedError("Unknown mode {}".format(self.mode))
        return self
    
    def __exit__(self, exc_type, exc_value, traceback):
        self.flush()

    def flush(self):
        if self.flushed == False:
            self.flushed = True
            if self.mode == 'w':
                KDS_SO.kds__optimize_write(self.c_kds_data)

    def append(self, key, msg_num):
        if self.dumper is not None:
            key = self.dumper(key)
        assert(isinstance(key, type("".encode('ascii'))))
        KDS_SO.kds__append(self.c_kds_data, key, len(key), msg_num);

    def __iter__(self):
        i = 0
        while i<len(self):
            #print("yileding a key out of iter")
            yield self.getkeyi(i)
            i += 1

    def __len__(self):
        return int(KDS_SO.kds__len(self.c_kds_data))

    def __contains__(self, key):
        if self.dumper is not None:
            key = self.dumper(key)
        assert(isinstance(key, type("".encode('ascii'))))
        return KDS_SO.kds__contains(self.c_kds_data, key, len(key)) == 1;

    def __getitem__(self, key):
        #import pdb
        #pdb.set_trace()
        if self.dumper is not None:
            key = self.dumper(key)
        assert(isinstance(key, type("".encode('ascii'))))
        return Query(self.c_kds_data, key)

    def getkeyi(self, i):
        raw = ffi.new("unsigned char [256]")
        raw_len = ffi.new("int *")
        rc = KDS_SO.kds__get_key_i(self.c_kds_data, i, raw, raw_len)
        if rc != 0:
            raise IndexError("Key at i={} not found".format(i))
        key = ffi.buffer(raw, raw_len[0])[:]
        if self.loader is not None:
            key = self.loader(key)
        return key

    def keys(self):
        return (k for k in self)

class Query(object):
    def __init__(self, kds_data, key):
        self.key = key
        self.c_kds_data = kds_data
        self.query_context = None
        
        tmp = ffi.new("struct query_context *")
        self.query_context = ffi.gc(tmp, KDS_SO.kds__teardown_query)
        KDS_SO.kds__setup_query(self.query_context, self.c_kds_data)
        KDS_SO.kds__lookup(self.query_context, self.key, len(self.key))
    
    def __repr__(self):
        return "<Query object for key={}>".format(self.key)

    def __len__(self):
        assert(self.query_context)
        return int(self.query_context.len(self.query_context))

    def __getitem__(self, i):
        assert(self.query_context)
        return int(self.query_context.geti(self.query_context, i))
    
    def next(self):
        assert(self.query_context)
        #print("What am I")
        return int(self.query_context.next(self.query_context))

    def prev(self):
        assert(self.query_context)
        return int(self.query_context.prev(self.query_context))

    def ge(self, ref):
        assert(self.query_context)
        return int(self.query_context.ge(self.query_context, ref))

    def le(self, ref):
        assert(self.query_context)
        return int(self.query_context.le(self.query_context, ref))


# Really this is all just number series.  
# Comparisons and joins etc.
# Do do this better than conventional db we need meta.
# Can do lots here have consitancy.
