from collections import namedtuple

from cffi import FFI
ffi = FFI()

TTRIE_STRUCT = """
struct ttrie__choice_table
{
    unsigned char *name;                 // First element = last_i -1, not null terminated string!!!!!  len_minus_2
    unsigned char *ids;              // First element[0] is the number of following rows in the array. len_minus_2
};

struct ttrie__terminator
{
    unsigned long long *results;
    unsigned long long last_i;
};

// Not reusing suffixes.
struct ttrie__suffix
{
    unsigned long long *results;
    unsigned char *suffix_lens;
    unsigned char **suffixs;
    unsigned long long last_i;
};

struct ttrie__passthrough
{    
    unsigned char (*ids)[5]; 
    unsigned long long *results;
    unsigned long long last_i;
};

// Not reusing infixs
struct ttrie__infix
{
    unsigned char (*ids)[5];
    unsigned char *infix_lens;
    unsigned char **infixs;
    unsigned long long last_i;
};

struct ttrie__obj
{
    unsigned long long len;                 // Number of nodes in ttrie
    char mode;
    unsigned char root_id[5];                // Should be zero                

    bool index_table_names;
    struct trie__view *table_names_index;

    unsigned int last_choice_i;              
    struct ttrie__choice_table *choice_tables;

    struct ttrie__terminator term_table;
    struct ttrie__passthrough pass_table;

    struct ttrie__suffix suffix_table;
    struct ttrie__infix infix_table;
};
"""

MDS_API = """
static void encode_id(unsigned char table_type, unsigned int table_i, unsigned long long row_i, unsigned char id[5]);
static void decode_id(const unsigned char id[5], unsigned char *table_type, unsigned int *table_i, unsigned long long *row_i);
static bool power_of_two_or_zero(unsigned int i);
"""

ffi.cdef(TTRIE_STRUCT)
ffi.cdef(MDS_API)

#TTRIE_SO = ffi.dlopen("libttrie.so")

# How do we unit test static functions. We cheat and include thus not static.
TTRIE_SO = ffi.verify("""#include "tabletrie.c"

                      """, 
                      libraries=["ttrie"], 
                      library_dirs=["."],
                      include_dirs=[".", "../first_trie"],
                      extra_compile_args=["-std=c99"])

id_raw = namedtuple('id_raw', ['table_type', 'table_i', 'row_i'])
DATA = [id_raw(0, 1, 2),
        id_raw(0, 128, 2),
        id_raw(0, 100000, 2),
        id_raw(0, 1, 255),
        id_raw(1, 0, 1),
        id_raw(2, 0, 2),
        id_raw(3, 0, 4),
        id_raw(4, 0, 8),
        ]


def test_power_2():
    powers_of_two  = {2**x for x in range(10)}
    powers_of_two.add(0)
    numbers =  [x for x in range(600)]
    for x in numbers:
        c_num = ffi.cast("unsigned int", x)
        result = TTRIE_SO.power_of_two_or_zero(c_num)
        print(x, result)
        if x in powers_of_two:
            assert result != 0
        else:
            assert result == 0


def test_encode_decode():
    for x in DATA:
        yield execute_encode_decode_point, x.table_type, x.table_i, x.row_i

def execute_encode_decode_point(table_id, table_i, row_i):
    c_table_type = ffi.cast("unsigned char", table_id)
    c_table_id = ffi.cast("unsigned int", table_i)
    c_row_i = ffi.cast("unsigned long long", row_i)
    c_res_id = ffi.new('unsigned char [5]')
    print("encoding {c_table_type}, {c_table_id}, '{c_row_i}'".format(**locals()))
    TTRIE_SO.encode_id(c_table_type, c_table_id, c_row_i, c_res_id)
    
    actual_table_type_p = ffi.new("unsigned char *")
    actual_table_i_p = ffi.new("unsigned int *")
    actual_row_i_p = ffi.new("unsigned long long *")
    TTRIE_SO.decode_id(c_res_id, actual_table_type_p, actual_table_i_p, actual_row_i_p)
    
    actual_table_type = actual_table_type_p[0]
    actual_table_i = actual_table_i_p[0]
    actual_row_i = actual_row_i_p[0]
    print("decoded {} {} {}".format(actual_table_type, actual_table_i, actual_row_i))

    print("table_type: actual='{}', expected='{}'".format(actual_table_type, table_id))
    assert actual_table_type == table_id

    print("table_i: actual='{}', expected='{}'".format(actual_table_i, table_i))
    assert actual_table_i == table_i

    print("row_i: actual='{}', expected='{}'".format(actual_row_i, row_i))
    assert actual_row_i == row_i
