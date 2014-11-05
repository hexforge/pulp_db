import os
from collections import namedtuple
import random 

from cffi import FFI
ffi = FFI()

RTRIE_STRUCT = """
#define STACK_SIZE ...
#define RTRIE_NAME_SIZE ...

union suffix_or_children
{   
    void *link;
    unsigned char* suffix;
    struct rtrie__node* children;
};

struct rtrie__node
{                                                 
    unsigned char *keys;
    union suffix_or_children u;
    unsigned int result;
    unsigned char last_i;
    unsigned char suffix_len;    
    bool has_suffix;
    bool has_value;    
};

struct rtrie__frame
{
    signed short stack_child_index;
    struct rtrie__node *node;
};

struct rtrie__view
{
    unsigned long long len;
    struct rtrie__node *root;

    signed short stack_index;
    struct rtrie__frame stack[...];
};
"""

RTRIE_API = """
int rtrie__open(struct rtrie__view *t);
int rtrie__close(struct rtrie__view *t);       
struct rtrie__node *rtrie__add(struct rtrie__view *t, void *key, int len, bool *new_entry);
struct rtrie__node *rtrie__get(struct rtrie__view *t, void *key, int len);
unsigned long long rtrie__len(struct rtrie__view *t);
bool rtrie__node_has_value(struct rtrie__node *n);
bool rtrie__node_has_children(struct rtrie__node *n);
bool rtrie__node_has_suffix(struct rtrie__node *n);
void rtrie__iter(struct rtrie__view *t);
struct rtrie__node *rtrie__next(struct rtrie__view *t);
int rtrie__fullkey(struct rtrie__view *t, unsigned char *dest, int *len);
"""

ffi.cdef(RTRIE_STRUCT)
ffi.cdef(RTRIE_API)

folder_path = os.path.dirname(__file__)
so_path = os.path.abspath(os.path.join(folder_path, '../build/libs'))
c_file_path = os.path.abspath(os.path.join(folder_path, '../src/pulp_db/kds'))

RTRIE_SO = ffi.verify("""#include "rtrie.c"

                      """, 
                      libraries=["rtrie"], 
                      library_dirs=[so_path],
                      runtime_library_dirs=[so_path],
                      include_dirs=[c_file_path],
                      extra_compile_args=["-std=c99"])


DATA_SETS = []

data_array = ["abc".encode('ascii'), 
              "abcd".encode('ascii'), 
              "erm".encode('ascii'), 
              "abc".encode('ascii'), 
              "erq".encode('ascii'), 
              "123456789".encode('ascii'), 
              "1234xxxx".encode('ascii'), 
              "1234567890".encode('ascii')]
not_there = ["not_there".encode('ascii'), 
             "a".encode('ascii'), 
             "abce".encode('ascii'), 
             "123456789".encode('ascii')]
data_set1 = (data_array, not_there)
DATA_SETS.append(data_set1)

data_array = ["a\x00c".encode('ascii'), 
              "abcd".encode('ascii'), 
              "erm".encode('ascii'), 
              "abcd\x00".encode('ascii')]
not_there = ["erm\x00".encode('ascii'), 
            "ac".encode('ascii')]
data_set2 = (data_array, not_there)
DATA_SETS.append(data_set2)

def make_rtie(data_set):
    data_array, not_there = data_set;

    rt = ffi.new("struct rtrie__view *")
    n = ffi.new("struct rtrie__node *")
    data_results = {}

    print("{0:-<80}".format("opening rtrie"))
    rc = RTRIE_SO.rtrie__open(rt)
    assert rc == 0 
    
    print("{0:-<80}".format("Adding elements to rtrie"))
    for i, e in enumerate(data_array):
        print("adding {}".format(e))
        edata = ffi.new("char []", e)
        elen = ffi.cast("unsigned int", len(e))
        new_entry = ffi.new("bool *")
        n = RTRIE_SO.rtrie__add(rt, edata, elen, new_entry)
        assert n != ffi.NULL
        #eres = ffi.cast("unsigned int", i)
        n.result = i #eres
        data_results[e] = i
    
    print("{0:-<80}".format("Checking number of elements in rtrie"))
    len_tree = RTRIE_SO.rtrie__len(rt);
    print(int(len_tree), len(set(data_array)))
    assert int(len_tree) == len(set(data_array))

    print("{0:-<80}".format("Getting elements from rtrie"))
    data = (data_array + not_there)
    random.shuffle(data)
    for x in data:
        print("Trying to get {}".format(x))
        xdata = ffi.new("char []", x)
        xlen = ffi.cast("unsigned int", len(x))
        n = RTRIE_SO.rtrie__get(rt, xdata, xlen);
        if x in data_array:
            print("Checking is there")
            assert n != ffi.NULL
            assert int(n.result) == data_results[x]
        elif x in not_there:
            assert n == ffi.NULL
            print("Checking is not there")

    print("{0:-<80}".format("iterating rtrie"))
    RTRIE_SO.rtrie__iter(rt)
    klen = ffi.new("int*")
    fullk = ffi.new("unsigned char [256]")
    for i in  range(len_tree + 10):
        #print("Next ")
        n = RTRIE_SO.rtrie__next(rt);
        #print(n);
        if i <= len_tree - 1:
            assert n != ffi.NULL
            RTRIE_SO.rtrie__fullkey(rt, fullk, klen);
            key = ffi.buffer(fullk, klen[0])
            print("Got key {}".format(key[:]))
            #print(sorted(data_array))
            #print(key[:], sorted(set(data_set[0])))
            assert key[:] in data_array
        else:
            #print("beep")
            assert n == ffi.NULL

    print("{0:-<80}".format("Closing rtrie"))
    rc = RTRIE_SO.rtrie__close(rt)
    assert rc == 0

def test_rtrie():
    for data_set in DATA_SETS:
        yield make_rtie, data_set
