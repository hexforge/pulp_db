import os

from cffi import FFI
ffi = FFI()

BITPACK_STRUCTS = """
#define MAX_MSG_IN_CLUMP ...

struct bitpack__clump {
    long long offsets[...];
    unsigned int msg_sizes[...];
};
"""

BITPACK_API = """
int bitpack__decode(char *source, struct bitpack__clump *target, int little_endian);
int bitpack__encode(char *target, struct bitpack__clump *source, int little_endian);
int bitpack__print_struct(struct bitpack__clump *clump);
"""

#HEX_STRUCTS = """ """

HEX_API = """
int hex__dump(char *data, unsigned int n);
"""


folder_path = os.path.dirname(__file__)
so_path = os.path.abspath(os.path.join(folder_path, '../libs'))
headers_path = os.path.abspath(os.path.join(folder_path, '../headers'))
c_file_path = os.path.abspath(os.path.join(folder_path, '../pulp_db/mds'))

ffi.cdef(BITPACK_STRUCTS)
ffi.cdef(BITPACK_API)
#ffi.cdef(HEX_STRUCTS)
ffi.cdef(HEX_API)

BITPACK_C = ffi.verify("""#include "bitpack.c"
                           #include "hex.c" """, 
                      #libraries=["mds"], 
                      #library_dirs=[so_path],
                      #runtime_library_dirs=[so_path],
                      include_dirs=[headers_path, c_file_path],
                      extra_compile_args=["-std=c99", "-Wno-unused-function"])


def check_bitpack(offset):
    expected_data = []
    
    size = 128
    block = ffi.new("char [128]")
    print("Blank data------------------------")
    BITPACK_C.hex__dump(block, 128)

    data = ffi.new("struct bitpack__clump *")
    for i in range(BITPACK_C.MAX_MSG_IN_CLUMP):
        data[0].offsets[i] = offset
        data[0].msg_sizes[i] = i + 3
        expected_data.append((offset, i+3))
        offset += i + 3 

    #print("Struct to write------------------------")
    #BITPACK_C.bitpack__print_struct(data)

    print("Struct packed------------------------")
    BITPACK_C.bitpack__encode(block, data, 1)
    BITPACK_C.hex__dump(block, 128)

    print("Struct unpacked------------------------")
    result = ffi.new("struct bitpack__clump*")
    BITPACK_C.bitpack__decode(block, result, 1)

    for i in range(BITPACK_C.MAX_MSG_IN_CLUMP):
        #print(result[0].offsets[i])
        #print(result[0].msg_sizes[i])
        #print("actual", result[0].offsets[i])
        #print("expected", expected_data[i][0])
        assert(result[0].offsets[i] == expected_data[i][0])
        assert(result[0].msg_sizes[i] == expected_data[i][1])
    #BITPACK_C.bitpack__print_struct(result)
    return 0;


def test_bitpack_zero():
    check_bitpack(offset=0)

def test_bitpack_mid():
    check_bitpack(offset=1113545)

def test_bitpack_large():
    check_bitpack(offset=1029511627776)
