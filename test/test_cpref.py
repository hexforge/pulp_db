import os
from collections import namedtuple
import random

from cffi import FFI
ffi = FFI()

CPREF_STRUCT = """
struct cpref__obj 
{
    char mode;
    struct mmbuf_obj *ref_file;
    char ref_file_path[256];
    unsigned long long size;
};

struct cpref__meta_block
{
    signed long long min;
    signed long long max;
};

struct cpref__stream 
{
    unsigned char mode;
    unsigned char *allocated_area;
    signed int num_blocks;

    unsigned long long meta_start_offset;
    signed int meta_block_i;

    signed long long ref_start_offset;
    signed int ref_block_i;

    unsigned long long nmsgs;
    signed long long low_ref;
    signed long long high_ref;

    signed long long current_i;
    signed long long current_block_ref_low;
    signed long long current_block_ref_high;
};
"""

CPREF_API = """
void print_stream_details(struct cpref__stream *stream);

int cpref__open(struct cpref__obj *cp, const char *ref_file_path, const char mode);
int cpref__close(struct cpref__obj *cp);
signed long long cpref__setup_write_stream(struct cpref__stream *stream, struct cpref__obj *cp, unsigned long long total_msgs);
void cpref__setup_read_stream(struct cpref__stream *stream, struct cpref__obj *cp, unsigned long long offset);
void cpref__close_stream(struct cpref__stream *stream);

/*---
 WRITE METHODS
---*/
void cpref__write_stream(struct cpref__stream *stream, signed long long *refs, int n);

/*---
 READ METHODS
---*/
signed long long cpref__next(struct cpref__stream *stream);   
signed long long cpref__prev(struct cpref__stream *stream);
signed long long cpref__get(struct cpref__stream *stream, unsigned long long i);
signed long long cpref__ge_ref(struct cpref__stream *stream, signed long long ref);
signed long long cpref__le_ref(struct cpref__stream *stream, signed long long ref);
"""

ffi.cdef(CPREF_STRUCT)
ffi.cdef(CPREF_API)


folder_path = os.path.dirname(__file__)
so_path = os.path.abspath(os.path.join(folder_path, '../build/libs'))
headers_path = os.path.abspath(os.path.join(folder_path, '../build/headers'))
c_file_path = os.path.abspath(os.path.join(folder_path, '../src/pulp_db/kds'))

# How do we unit test static functions. We cheat and include thus not static.
CPREF_SO = ffi.verify("""#include "cpref.c"
                      """, 
                      libraries=["ttrie"], 
                      library_dirs=[so_path],
                      runtime_library_dirs=[so_path],
                      include_dirs=[c_file_path, headers_path],
                      extra_compile_args=["-std=c99", "-Wno-unused-function"])

W_L = 2000
PY_DATA = [x for x in range(W_L)]
C_DATA = ffi.new("signed long long [{}]".format(len(PY_DATA)), PY_DATA)
R_L = 1800

if not os.path.exists('data/cpref'):
    os.makedirs('data/cpref')

def cpref_write(file_path, data, length):
    cp_write = ffi.new("struct cpref__obj *c")
    mode_write = ffi.cast("char", "w".encode('ascii'))
    rc = CPREF_SO.cpref__open(cp_write, file_path, mode_write)
    assert rc == 0

    sp_write = ffi.new("struct cpref__stream *")
    CPREF_SO.cpref__setup_write_stream(sp_write, cp_write, length)
    print("{0:-<80}".format("writing_stream"))
    CPREF_SO.cpref__write_stream(sp_write, data, length)
    CPREF_SO.print_stream_details(sp_write)
    
    print("{0:-<80}".format("closing cpref write"))
    cpref_close(sp_write, cp_write)

def cpref_close(sp_write, cp_write):
    CPREF_SO.cpref__close_stream(sp_write)
    rc = CPREF_SO.cpref__close(cp_write)
    assert rc == 0

def cpref_read(file_path):
    cp_read = ffi.new("struct cpref__obj *c")
    mode_read = ffi.cast("char", "r".encode('ascii'))

    rc = CPREF_SO.cpref__open(cp_read, file_path, mode_read)
    assert rc == 0

    sp_read = ffi.new("struct cpref__stream *")
    offset = ffi.cast("int", 0)

    CPREF_SO.cpref__setup_read_stream(sp_read, cp_read, offset)
    return sp_read, cp_read

def test_cpref_read_write():
    file_path = ffi.new("char []", "data/cpref/pref_basic.tmp".encode("ascii"))
    print("{0:-<80}".format("testing test_cpref_write"))
    cpref_write(file_path, C_DATA, W_L)

    print("{0:-<80}".format("testing test_cpref_read"))
    print("-------------------test_read\n")
    sp_read, cp_read = cpref_read(file_path)

    CPREF_SO.print_stream_details(sp_read)

    print("{0:-<80}".format("testing read lookup at {}".format(R_L)))
    for x in range(R_L+1):
        result = CPREF_SO.cpref__get(sp_read, x)
        print("i={} result of lookup actual={}, expected={}".format(x, result, x))
        assert result == x

    print("{0:-<80}".format("testing next"))
    expected = R_L
    while True:
        expected += 1
        result = CPREF_SO.cpref__next(sp_read)
        if expected >= W_L:
            print("No cpref next expected {}".format(expected))
            assert result == -1
            expected -= 1
            break
        else:
            #print("Next result {} expected {}".format(result, expected))
            assert result == expected

    # Maybe not correct behavour!!!!!!! next->1998, next->1999, next->-1, prev->1998 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    print("{0:-<80}".format("testing prev"))
    while True:
        
        result = CPREF_SO.cpref__prev(sp_read)

        print("Prev result {} expected {}".format(result, expected))
        assert result == expected

        if expected < 0:
            break
        
        expected -= 1

    print("{0:-<80}".format("closing cpref read"))
    cpref_close(sp_read, cp_read)


ODD_W_L = 1000
ODD_C_W_L = ffi.cast("int", ODD_W_L)
ODD_PY_DATA = [x*2+1 for x in range(ODD_W_L)]
ODD_C_DATA = ffi.new("signed long long [{}]".format(len(ODD_PY_DATA)), ODD_PY_DATA)
ODD_R_L = 1000
ODD_C_R_L = ffi.cast("int", ODD_R_L)


def test_cpref_ge():
    file_path = ffi.new("char []", "data/cpref/pref_ge.tmp".encode("ascii"))
    cpref_write(file_path, ODD_C_DATA, ODD_C_W_L)

    print("{0:-<80}".format("testing test_cpref_ge"))
    print("-------------------test_read\n")
    sp_read, cp_read = cpref_read(file_path)

    CPREF_SO.print_stream_details(sp_read)

    x = 0
    max_ref = ODD_PY_DATA[-1]
    while True:
        c_x = ffi.cast("int", x)

        result = CPREF_SO.cpref__ge_ref(sp_read, c_x)
        print("result = {} for lookup of x={}".format(int(result), x))
        if x > max_ref:
            print("Out of range", x)
            assert result == -1
        if result == -1:
            print("Got -1 result")
            break

        if (x%2 == 0):
            assert result == x+1
        else:
            assert result == x

        x += 1

    print("{0:-<80}".format("closing cpref ge read"))
    cpref_close(sp_read, cp_read)

def test_cpref_le():
    file_path = ffi.new("char []", "data/cpref/pref_le.tmp".encode("ascii"))
    cpref_write(file_path, ODD_C_DATA, ODD_C_W_L)

    print("{0:-<80}".format("testing test_cpref_le"))
    print("-------------------test_read\n")
    sp_read, cp_read = cpref_read(file_path)

    CPREF_SO.print_stream_details(sp_read)

    x = 0
    min_ref = ODD_PY_DATA[0]
    max_ref = ODD_PY_DATA[-1]
    while True:
        c_x = ffi.cast("int", x)

        result = CPREF_SO.cpref__le_ref(sp_read, c_x)
        print("result = {} for lookup of x={}".format(int(result), x))
        if x < min_ref:
            assert result == -1
            x += 1
            continue
        elif x > max_ref:
            if x > max_ref + 1:
                break

        if (x%2 == 0):
            assert result == x-1
        else:
            assert result == x

        x += 1

    print("{0:-<80}".format("closing cpref ge read"))
    cpref_close(sp_read, cp_read)
