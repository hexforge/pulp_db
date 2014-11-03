import os
from cffi import FFI
ffi = FFI()

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
so_path = os.path.abspath(os.path.join(folder_path, '../libs'))
headers_path = os.path.abspath(os.path.join(folder_path, '../headers'))
c_file_path = os.path.abspath(os.path.join(folder_path, '../pulp_db/kds'))

KDS_SO = ffi.verify("""#include "kds.h" """, 
                      libraries=["kds"], 
                      library_dirs=[so_path],
                      runtime_library_dirs=[so_path],
                      include_dirs=[headers_path, c_file_path],
                      extra_compile_args=["-std=c99"])


from collections import namedtuple
dataset = namedtuple("dataset", ["name", "pts"])

DATA = []
d1 = dataset(name="one".encode('ascii'), pts=[x for x in range(1,11)])
DATA.append(d1)
d2 = dataset(name="two".encode('ascii'), pts=[x for x in range(4,7)])
DATA.append(d2)

if not os.path.exists('data/kds'):
    os.mkdir('data/kds')

def test_write():
    k = ffi.new("struct keyds *")
    KDS_SO.kds__open(k, "data/kds".encode('ascii'), "wSrcTime".encode('ascii'), "w".encode('ascii'))

    for data in DATA:
        print("Appending data", data.pts, len(data.pts))
        for element in data.pts:
            KDS_SO.kds__append(k, data.name, len(data.name), element);

    print(1)
    KDS_SO.kds__optimize_write(k);
    print(2)
    KDS_SO.kds__close(k);

def test_read():
    k = ffi.new("struct keyds *")
    KDS_SO.kds__open(k, "data/kds".encode('ascii'), "wSrcTime".encode('ascii'), "r".encode('ascii'))

    len_keys = KDS_SO.kds__len(k)
    print("len=", len_keys)

    total_refs = KDS_SO.kds__total_refs(k)
    print("total_refs=", total_refs)

    key = ffi.new("char[256]")
    klen = ffi.new("int *")

    for i in range(2):
        rc = KDS_SO.kds__get_key_i(k, i, key, klen)
        assert(rc==0)
        print("key={} klen={}".format(key, klen))

    i = 2
    rc = KDS_SO.kds__get_key_i(k, i, key, klen)
    assert(rc==1)

    rc = KDS_SO.kds__contains(k, d1.name, len(d1.name))
    assert(rc)

    rc = KDS_SO.kds__contains(k, d2.name, len(d2.name))
    assert(rc)

    rc = KDS_SO.kds__contains(k, "not_there".encode('ascii'), len("not_there".encode('ascii')))
    assert(rc==0)

    q = ffi.new("struct query_context *")

    KDS_SO.kds__setup_query(q, k)

    rc = KDS_SO.kds__lookup(q, d1.name, len(d1.name))
    assert (rc==0)

    print("qlen=", q.len(q))
    print("5th=", q.geti(q, 5))
    print("next=", q.next(q))
    print("next=", q.next(q))
    print("next=", q.next(q))
    print("next=", q.next(q))
    print("next=", q.next(q))
    print("next=", q.next(q))
    print("next=", q.prev(q))
    print("ge=", q.ge(q, 3))
    print("ge=", q.ge(q, 12)) 
    print("le=", q.le(q, 12))
    print("le=", q.le(q, -9))

    rc = KDS_SO.kds__lookup(q, d2.name, len(d2.name))
    assert (rc==0)

    rc = KDS_SO.kds__lookup(q, "not_there".encode('ascii'), len("not_there".encode('ascii')))
    assert (rc==1)

    KDS_SO.kds__teardown_query(q)
    KDS_SO.kds__close(k)

if __name__ == "__main__":
    test_write()
    test_read()
