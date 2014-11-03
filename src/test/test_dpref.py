import os
from collections import namedtuple
import random 

from cffi import FFI
ffi = FFI()

DPREF_STRUCT = """
struct dpref__obj 
{
    char mode;
    struct mmbuf_obj *ref_file;
    char ref_file_path[256];
    unsigned long long num_msgs;
};

struct dpref__buffer
{
    signed int refs_index;
    unsigned long long global_index;
    signed long long first_page_offset;
    signed long long current_page_offset;
};

"""

DPREF_API = """
int dpref__open(struct dpref__obj *const p, const char *ref_file_path, const char mode);
int dpref__close(struct dpref__obj *p, bool del);
int dpref__setup_buffer(struct dpref__obj *const p, struct dpref__buffer *const pref_b, ...);
int dpref__teardown_buffer(struct dpref__obj *const p, struct dpref__buffer *const pref_b);
signed long long dpref__append(struct dpref__obj *const p, struct dpref__buffer *const buf, const signed long long ref);
signed long long dpref__geti(const struct dpref__obj *p, struct dpref__buffer *const buf, const unsigned long long index);
"""

ffi.cdef(DPREF_STRUCT)
ffi.cdef(DPREF_API)

folder_path = os.path.dirname(__file__)
so_path = os.path.abspath(os.path.join(folder_path, '../libs'))
headers_path = os.path.abspath(os.path.join(folder_path, '../headers'))
c_file_path = os.path.abspath(os.path.join(folder_path, '../pulp_db/kds'))

DPREF_SO = ffi.verify("""#include "dpref.c"

                      """, 
                      libraries=["dpref"], 
                      library_dirs=[so_path],
                      runtime_library_dirs=[so_path],
                      include_dirs=[headers_path, c_file_path],
                      extra_compile_args=["-std=c99"])


if not os.path.exists('data/dpref'):
    os.makedirs('data/dpref')

test_file_name = ffi.new("char[]", "data/dpref/test_pref.tmp".encode('ascii'))

def test_dpref():
    #---
    # WRITING
    #---

    out_obj = ffi.new("struct dpref__obj*")
    out_buf = ffi.new("struct dpref__buffer*")
    mode = ffi.cast("char", "w".encode('ascii'))
    print("{0:-<80}".format("Opneding dpref in write mode"))
    rc = DPREF_SO.dpref__open(out_obj, test_file_name, mode)
    assert rc == 0

    rc = DPREF_SO.dpref__setup_buffer(out_obj, out_buf)
    assert rc == 0

    print("{0:-<80}".format("Appending values"))
    NUM_ELEMENTS = 20000
    for i in range(NUM_ELEMENTS):
        #print("appending value", i)
        DPREF_SO.dpref__append(out_obj, out_buf, i)


    print("{0:-<80}".format("Closing dpref"))
    rc = DPREF_SO.dpref__teardown_buffer(out_obj, out_buf)
    assert rc == 0

    error = DPREF_SO.dpref__close(out_obj, 0)
    assert rc == 0

    #---
    # READING
    #---
    in_obj = ffi.new("struct dpref__obj*")
    in_buf = ffi.new("struct dpref__buffer*")
    mode = ffi.cast("char", "r".encode('ascii'))

    print("{0:-<80}".format("Opening dpref in read mode"))
    rc = DPREF_SO.dpref__open(in_obj, test_file_name, mode)
    assert rc == 0

    offset = ffi.cast("int", 0)
    rc = DPREF_SO.dpref__setup_buffer(in_obj, in_buf, offset)
    assert rc == 0

    print("{0:-<80}".format("Going to first index"))
    count = ffi.cast("int", 0)
    ref = DPREF_SO.dpref__geti(in_obj, in_buf, count)
    
    print("{0:-<80}".format("Iterativing over the pref in file"))
    i = 0
    while (1):
        last_ref = int(ref)
        print("Element {}, reference={}".format(int(count), int(ref)))
        i += 1
        count = ffi.cast("int", i)
        ref = DPREF_SO.dpref__geti(in_obj, in_buf, count)
        if ref == -1:
            assert i >= NUM_ELEMENTS
            break
        #print("ref last_ref", ref, last_ref)
        #assert ref == last_ref + 1

    print("{0:-<80}".format("Closing dpref"))
    rc = DPREF_SO.dpref__teardown_buffer(in_obj, in_buf)
    assert rc == 0

    rc = DPREF_SO.dpref__close(in_obj, 1)
    assert rc == 0
