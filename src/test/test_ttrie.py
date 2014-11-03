import os
from collections import namedtuple
import random

from cffi import FFI
ffi = FFI()

TTRIE_STRUCT = """
#define STACK_SIZE ...
#define CHOICE_TABLE ...
#define TERMINATOR_TABLE ...
#define PASSTHROUGH_TABLE ...
#define SUFFIX_TABLE ...
#define INFIX_TABLE ...

struct ttrie__choice_table
{
    unsigned char *name;
    unsigned char *ids;
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

struct ttrie__infix
{
    unsigned char (*ids)[5];
    unsigned char *infix_lens;
    unsigned char **infixs;
    unsigned long long last_i;
};

struct ttrie__obj
{
    unsigned long long len;
    char mode;
    unsigned char root_id[5];
    bool index_table_names;
    struct rtrie__view *table_names_index;

    unsigned int last_choice_i;              
    struct ttrie__choice_table *choice_tables;

    struct ttrie__terminator term_table;
    struct ttrie__passthrough pass_table;

    struct ttrie__suffix suffix_table;
    struct ttrie__infix infix_table;
};

struct ttrie__frame
{
    unsigned char table_type;
    unsigned char id[5];
    signed char child_index;                       //<--- Only needed for child nodes.
    unsigned char key[...];
    int key_len;
};

struct ttrie__iterator                             // This is a view on the data.  Using this shows the data can go away.
{
    struct ttrie__obj *view;
    struct ttrie__frame *frames;
    signed int stack_index;                             
};
"""

TTRIE_API = """
const int INTSIZE;
void free(void *);

static void encode_id(unsigned char table_type, unsigned int table_i, unsigned long long row_i, unsigned char id[5]);
static void decode_id(const unsigned char id[5], unsigned char *table_type, unsigned int *table_i, unsigned long long *row_i);
static bool power_of_two_or_zero(unsigned int i);

static unsigned char *generate_lenstr(void *str, int str_len);
static void *generate_raw(unsigned char *len_str, int *len_p);
static int num_digits_lenstr(const unsigned char *len_str);
static void print_result_id(unsigned char result_id[5]);

static void choice_print_table_names(struct ttrie__obj *tt);
static void choice_printall_tables(struct ttrie__obj *tt);
static unsigned int choice_find_add_table(struct ttrie__obj *tt, void *table_name, unsigned int name_len);
static int choice_new_row(struct ttrie__obj *tt, char *table_name, unsigned char name_len, unsigned char (*row_id)[5], unsigned char result_id[5]);

static int terminator_append(struct ttrie__obj *tt, unsigned long long value, unsigned char node_id[5]);
static unsigned long long terminator_get_result(struct ttrie__obj *tt, unsigned long long row_i);

static int passthrough_append(struct ttrie__obj *tt, unsigned char child_id[5], unsigned long long value, unsigned char node_id[5]);
static unsigned long long passthrough_lookup(struct ttrie__obj *tt, unsigned long long row_i, unsigned char id[5]);

static int suffix_append(struct ttrie__obj *tt, unsigned char *suffix, unsigned char n, unsigned long long value, unsigned char node_id[5]);
static unsigned char suffix_get_len(struct ttrie__obj *tt, unsigned long long row_i);
static void suffix_get_suffix(struct ttrie__obj *tt, unsigned long long row_i, unsigned char **suffix);
static unsigned long long suffix_get_result(struct ttrie__obj *tt, unsigned long long row_i);

static int infix_append(struct ttrie__obj *tt, unsigned char *infix, int n, unsigned char child_node_id[5], unsigned char node_id[5]);
static unsigned char infix_get_len(struct ttrie__obj *tt, unsigned long long row_i);
static void infix_get_infix(struct ttrie__obj *tt, unsigned long long row_i, unsigned char **infix);
static void infix_get_childnode(struct ttrie__obj *tt, unsigned long long row_i, unsigned char id[5]);

int ttrie__open(struct ttrie__obj *tt, char mode);
int ttrie__convert(struct ttrie__obj *tt, struct rtrie__view *t);
int ttrie__write(struct ttrie__obj *tt, char *trie_file_name);
int ttrie__close(struct ttrie__obj *tt);
int ttrie__read(struct ttrie__obj *tt, char *trie_file_name); 
int ttrie__get(struct ttrie__obj *tt, void *key, int keylen, unsigned long long *result);
unsigned long long ttrie__len(struct ttrie__obj *tt);
void ttrie__dprint(struct ttrie__obj *tt);
void ttrie__unindex(struct ttrie__obj *tt);
int ttrie__iter(struct ttrie__iterator *iter, struct ttrie__obj *tt);
int ttrie__next_node(struct ttrie__iterator *iter, void *key, int *key_len, unsigned long long **value);
int ttrie__close_iter(struct ttrie__iterator *iter);
"""



RTRIE_STRUCT = """
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
ffi.cdef(TTRIE_STRUCT)
ffi.cdef(TTRIE_API)

folder_path = os.path.dirname(__file__)
so_path = os.path.abspath(os.path.join(folder_path, '../libs'))
headers_path = os.path.abspath(os.path.join(folder_path, '../headers'))
c_file_path = os.path.abspath(os.path.join(folder_path, '../pulp_db/kds'))

TTRIE_SO = ffi.verify("""#include "ttrie.c"
const int INTSIZE = sizeof(int);
                      """, 
                      libraries=["ttrie"], 
                      library_dirs=[so_path],
                      runtime_library_dirs=[so_path],
                      include_dirs=[headers_path, c_file_path],
                      extra_compile_args=["-std=c99", "-Wno-unused-function"])


if not os.path.exists('data/ttrie'):
    os.mkdir('data/ttrie')

# This will be moved to common.

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

##########################################
##########################################



def test_lenstr_with_str():
    print("{0:-<40}".format("testing test_lenstr_with_str"))
    raw = "hel\00lo\00".encode('ascii')
    s = ffi.new("unsigned char[]", raw)
    slen = ffi.cast("int", len(raw))
    converted_s = TTRIE_SO.generate_lenstr(s, slen)
    print(int(converted_s[0]) +1, len(raw))
    assert int(converted_s[0]) +1 == len(raw)

    print("{0:-<40}".format("num_digits_lenstr"))
    assert TTRIE_SO.num_digits_lenstr(converted_s)  == len(raw) + 1

    print("{0:-<40}".format("testing generate_raw"))
    xlen = ffi.new("int *")
    converted_back = TTRIE_SO.generate_raw(converted_s, xlen)
    print(xlen[0], len(raw))
    assert xlen[0] == len(raw)
    x = ffi.buffer(converted_back, len(raw))
    print("converted_back=", x[:])
    assert x[:] == raw
    TTRIE_SO.free(converted_s)
    TTRIE_SO.free(converted_back)

def test_lenstr_with_int():
    print("{0:-<40}".format("testing test_lenstr_with_int"))
    n = 42
    raw = ffi.new("int *", n)
    ilen = int(TTRIE_SO.INTSIZE)
    print(ilen)
    converted_i = TTRIE_SO.generate_lenstr(raw, int(ilen))
    print(int(converted_i[0]) +1, ilen)
    assert int(converted_i[0]) +1 == ilen

    print("{0:-<40}".format("num_digits_lenstr"))
    assert TTRIE_SO.num_digits_lenstr(converted_i)  == ilen + 1

    print("{0:-<40}".format("testing generate_raw"))
    xlen = ffi.new("int *");
    converted_back = TTRIE_SO.generate_raw(converted_i, xlen)
    print(xlen[0], ilen)
    assert xlen[0] == ilen
    x = ffi.cast("int *", converted_back)
    print("converted_back=", x[0])
    
    assert x[0] == n
    TTRIE_SO.free(converted_i)
    TTRIE_SO.free(converted_back)

def test_open_close():
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "r".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0

def add_choice_table(ttp, table_name):
    tn = ffi.new("unsigned char[]", table_name)
    tn_len = len(tn)
    table_number = TTRIE_SO.choice_find_add_table(ttp, tn, tn_len)
    
    res_table = ttp.choice_tables[table_number]
    l = ffi.new("int *")
    name = TTRIE_SO.generate_raw(res_table.name, l)
    x = ffi.buffer(name, len(table_name))
    print("converted_back=", x[:])
    assert x[:] == table_name
    TTRIE_SO.free(name)
    return table_number

def test_make_choice_tables_read():
    print("{0:-<40}".format("testing test_make_choice_tables_read"))
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "r".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    data = ["abc".encode('ascii'), 
            "abxy".encode('ascii'), 
            "a".encode('ascii'), 
            "abz\00d".encode('ascii')]
    for i, d in enumerate(data):
        tnum = add_choice_table(ttp, d)
        assert tnum == i
        assert ttp.last_choice_i == i

    TTRIE_SO.choice_print_table_names(ttp)

    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0

def test_make_choice_tables_write():
    print("{0:-<40}".format("testing test_make_choice_tables_read"))
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "w".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    data = ["abc".encode('ascii'), "cexy".encode('ascii'), "bexy".encode('ascii'), "a".encode('ascii'), "\00\00".encode('ascii')]
    for i, d in enumerate(data):
        tnum = add_choice_table(ttp, d)
        assert tnum == i
        assert ttp.last_choice_i == i

    TTRIE_SO.choice_print_table_names(ttp)

    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0


ROW_TABLE_DATA = [
    ("foo".encode('ascii'), ["00001".encode('ascii'), "00002".encode('ascii'), "00003".encode('ascii')]),
    ("ba".encode('ascii'), ["00004".encode('ascii'), "00005".encode('ascii')]),
    ("r".encode('ascii'), ["00006".encode('ascii')]),
    ("foo".encode('ascii'), ["00007".encode('ascii'), "00008".encode('ascii'), "00009".encode('ascii')]),          
]

def add_table_and_rows(ttp, table_name, row_ids):
    tn = ffi.new("unsigned char[]", table_name)
    tn_len = len(tn)
    row_ids = ffi.new("unsigned char[{}][5]".format(len(row_ids)), row_ids)
    #[print(chr(row_ids[j][i])) for j in range(len(row_ids)) for i in range(5)]

    result_id = ffi.new("unsigned char[5]")

    TTRIE_SO.choice_new_row(ttp, tn, tn_len, row_ids, result_id)
    TTRIE_SO.print_result_id(result_id)
    return result_id

def test_make_choice_table_write_append_row():
    print("{0:-<40}".format("testing test_make_choice_table_write_append_row"))
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "w".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    table_names = [x[0] for x in ROW_TABLE_DATA]
    table_indexs = {x:table_names.index(x) for x in  table_names}
    table_rows = {i: table_names[:i+1].count(x)-1 for i, x in enumerate(table_names)}

    for i, (tab_name, row_ids) in enumerate(ROW_TABLE_DATA):
        print("{0:-<40}".format("testing add_table_and_rows", tab_name, row_ids))
        result_id = add_table_and_rows(ttp, tab_name, row_ids)
        actual_table_type_p = ffi.new("unsigned char *")
        actual_table_i_p = ffi.new("unsigned int *")
        actual_row_i_p = ffi.new("unsigned long long *")
        TTRIE_SO.decode_id(result_id, actual_table_type_p, actual_table_i_p, actual_row_i_p)
        print(actual_table_type_p[0], actual_table_i_p[0], actual_row_i_p[0])
        assert(actual_table_type_p[0] == TTRIE_SO.CHOICE_TABLE)

        print("table_id:", actual_table_i_p[0], table_indexs[tab_name])
        assert(actual_table_i_p[0] == table_indexs[tab_name])

        print("table_rows:",actual_row_i_p[0], table_rows[i])
        assert(actual_row_i_p[0] == table_rows[i])

    
    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0

TERMINATOR_DATA = [222, 333, 444]

def test_new_terminator_node():
    print("{0:-<40}".format("testing test_new_terminator_node"))
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "w".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    result_id = ffi.new("unsigned char[5]")
    for i, d in enumerate(TERMINATOR_DATA):
        TTRIE_SO.terminator_append(ttp, d, result_id)

        actual_table_type_p = ffi.new("unsigned char *")
        actual_table_i_p = ffi.new("unsigned int *")
        actual_row_i_p = ffi.new("unsigned long long *")
        TTRIE_SO.decode_id(result_id, actual_table_type_p, actual_table_i_p, actual_row_i_p)
        
        print("table_type:", actual_table_type_p[0], TTRIE_SO.TERMINATOR_TABLE)
        assert(actual_table_type_p[0] == TTRIE_SO.TERMINATOR_TABLE)

        print("table_id:", actual_table_i_p[0], 0)
        assert(actual_table_i_p[0] == 0)

        print("table_rows:",actual_row_i_p[0], i)
        assert(actual_row_i_p[0] == i)
        
        result = TTRIE_SO.terminator_get_result(ttp, actual_row_i_p[0])
        assert result == d

    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0

PASSTHROUGH_DATA = [222, 333, 444]
def test_new_passthrough_node():
    print("{0:-<40}".format("testing test_new_passthrough_node"))
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "w".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    result_id = ffi.new("unsigned char[5]")

    for i, d in enumerate(PASSTHROUGH_DATA):
        child_id = ffi.new("unsigned char[5]", ("\0\0\0\0" + str(i)).encode('ascii'))

        TTRIE_SO.passthrough_append(ttp, child_id, d, result_id)

        actual_table_type_p = ffi.new("unsigned char *")
        actual_table_i_p = ffi.new("unsigned int *")
        actual_row_i_p = ffi.new("unsigned long long *")
        TTRIE_SO.decode_id(result_id, actual_table_type_p, actual_table_i_p, actual_row_i_p)
        
        print("table_type:", actual_table_type_p[0], TTRIE_SO.PASSTHROUGH_TABLE)
        assert(actual_table_type_p[0] == TTRIE_SO.PASSTHROUGH_TABLE)

        print("table_id:", actual_table_i_p[0], 0)
        assert(actual_table_i_p[0] == 0)

        print("table_rows:",actual_row_i_p[0], i)
        assert(actual_row_i_p[0] == i)

        result_child_id = ffi.new("unsigned char[5]")
        result = TTRIE_SO.passthrough_lookup(ttp, actual_row_i_p[0], result_child_id)
        assert result == d

        actual_child_id = ffi.buffer(result_child_id, 5)
        expected_child_id = ffi.buffer(child_id, 5)

        print("Checking child_ids", actual_child_id[:], expected_child_id[:])
        assert actual_child_id[:] == expected_child_id[:]

    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0

SUFFIX_VALUES = [222, 333, 444]
SUFFIX_STRINGS = ["aaa".encode('ascii'), "bbbb".encode('ascii'), "c".encode('ascii')]
def test_new_suffix_node():
    print("{0:-<40}".format("testing test_new_suffix_node"))
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "w".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    result_id = ffi.new("unsigned char[5]")
    for i, (value, suffix) in enumerate(zip(SUFFIX_VALUES, SUFFIX_STRINGS)):

        s = ffi.new("unsigned char[{}]".format(len(suffix)), suffix)
        slen = ffi.cast("int", len(suffix))

        TTRIE_SO.suffix_append(ttp, s, slen, value, result_id)

        actual_table_type_p = ffi.new("unsigned char *")
        actual_table_i_p = ffi.new("unsigned int *")
        actual_row_i_p = ffi.new("unsigned long long *")
        TTRIE_SO.decode_id(result_id, actual_table_type_p, actual_table_i_p, actual_row_i_p)
        
        print("table_type:", actual_table_type_p[0], TTRIE_SO.SUFFIX_TABLE)
        assert(actual_table_type_p[0] == TTRIE_SO.SUFFIX_TABLE)

        print("table_id:", actual_table_i_p[0], 0)
        assert(actual_table_i_p[0] == 0)

        print("table_rows:", actual_row_i_p[0], i)
        assert(actual_row_i_p[0] == i)


        result = TTRIE_SO.suffix_get_result(ttp, actual_row_i_p[0])
        print("result=", result, value)
        assert result == value

        l_value = TTRIE_SO.suffix_get_len(ttp, actual_row_i_p[0])
        print("suffix len:", l_value, len(suffix))
        assert l_value == len(suffix)
        
        actual_suffix_p = ffi.new("unsigned char**")
        TTRIE_SO.suffix_get_suffix(ttp, actual_row_i_p[0], actual_suffix_p)
        actual_suffix = ffi.buffer(actual_suffix_p[0], l_value)
        print("suffix:", actual_suffix[:], suffix)
        assert actual_suffix[:] == suffix

    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0

INFIX_VALUES = [222, 333, 444]
INFIX_STRINGS = ["aaa".encode('ascii'), "bbbb".encode('ascii'), "c".encode('ascii')]
def test_new_infix_node():
    print("{0:-<40}".format("testing test_new_infix_node"))
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "w".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    result_id = ffi.new("unsigned char[5]")
    for i, (value, infix) in enumerate(zip(INFIX_VALUES, INFIX_STRINGS)):
        child_id = ffi.new("unsigned char[5]", ("\0\0\0\0" + str(i)).encode('ascii'))

        inf = ffi.new("unsigned char[{}]".format(len(infix)), infix)
        inflen = ffi.cast("int", len(infix))

        TTRIE_SO.infix_append(ttp, inf, inflen, child_id, result_id)

        actual_table_type_p = ffi.new("unsigned char *")
        actual_table_i_p = ffi.new("unsigned int *")
        actual_row_i_p = ffi.new("unsigned long long *")
        TTRIE_SO.decode_id(result_id, actual_table_type_p, actual_table_i_p, actual_row_i_p)
        
        print("table_type:", actual_table_type_p[0], TTRIE_SO.INFIX_TABLE)
        assert(actual_table_type_p[0] == TTRIE_SO.INFIX_TABLE)

        print("table_id:", actual_table_i_p[0], 0)
        assert(actual_table_i_p[0] == 0)

        print("table_rows:", actual_row_i_p[0], i)
        assert(actual_row_i_p[0] == i)

        result_child_id = ffi.new("unsigned char[5]")
        result = TTRIE_SO.infix_get_childnode(ttp, actual_row_i_p[0], result_child_id)
        actual_child_id = ffi.buffer(result_child_id, 5)
        expected_child_id = ffi.buffer(child_id, 5)
        print("Checking child_ids", actual_child_id[:], expected_child_id[:])
        assert actual_child_id[:] == expected_child_id[:]


        l_value = TTRIE_SO.infix_get_len(ttp, actual_row_i_p[0])
        print("infix len:", l_value, len(infix))
        assert l_value == len(infix)
        
        actual_infix_p = ffi.new("unsigned char**")
        TTRIE_SO.infix_get_infix(ttp, actual_row_i_p[0], actual_infix_p)
        actual_infix = ffi.buffer(actual_infix_p[0], l_value)
        print("infix:", actual_infix[:], infix)
        assert actual_infix[:] == infix

    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0



CONVERT_DATA = [("abc".encode('ascii'), 12), 
                ("abcd".encode('ascii'), 55), 
                ("erm".encode('ascii'), 66),
                ("abc".encode('ascii'), 77),
                ("ab\x00c".encode('ascii'), 78),
                ("erq".encode('ascii'), 88),
                ("x123456789".encode('ascii'), 99),
                ]



def make_ttrie_though_convert(data):

    rt = ffi.new("struct rtrie__view *")
    n = ffi.new("struct rtrie__node *")
    data_results = {}

    print("{0:-<40}".format("opening rtrie"))
    rc = TTRIE_SO.rtrie__open(rt)
    assert rc == 0 

    print("{0:-<40}".format("Adding elements to rtrie"))
    for key, val in data:
        print("adding key={}, value={}".format(key, val))
        edata = ffi.new("char []", key)
        elen = ffi.cast("unsigned int", len(key))
        new_entry = ffi.new("bool *");
        n = TTRIE_SO.rtrie__add(rt, edata, elen, new_entry)
        assert n != ffi.NULL
        eres = ffi.cast("unsigned int", val)
        n.result = eres

    print("{0:-<40}".format("testing make_ttrie_though_convert"))
    ttp = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "w".encode('ascii'))

    print("{0:-<40}".format("testing ttrie open"))
    rc = TTRIE_SO.ttrie__open(ttp, mode_str)
    assert rc == 0

    print("{0:-<40}".format("Converting rtrie to ttrie open"))
    TTRIE_SO.ttrie__convert(ttp, rt);

    print("{0:-<40}".format("Closing rtrie"))
    rc = TTRIE_SO.rtrie__close(rt)
    assert rc == 0

    return ttp



def test_conversion():
    print("{0:-<40}".format("testing test_conversion"))
    ttp = make_ttrie_though_convert(CONVERT_DATA)

    TTRIE_SO.print_result_id(ttp.root_id);
    TTRIE_SO.ttrie__dprint(ttp);

    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0


    # Need more of a test here!!


EXTRA_SEARCH_STRINGS = ["not_there".encode('ascii'), "x123".encode('ascii'), "ab".encode('ascii'), "e".encode('ascii'), "x".encode('ascii'), "ab\00".encode('ascii')]


def get_check(ttp, data, extra_strings):
    known_keys = [x[0] for x in data]
    all_search_strings = known_keys + extra_strings
    random.shuffle(all_search_strings)
    key_dict = dict(data)

    
    result = ffi.new("unsigned long long*");
    for key in all_search_strings:
        print("looking for ", key)

        rkey = ffi.new("unsigned char[{}]".format(len(key)), key)
        rkey_len = ffi.cast("int", len(key))

        rc = TTRIE_SO.ttrie__get(ttp, rkey, rkey_len, result)
        if key in known_keys:
            #print("beep", rc ,result[0])
            assert rc == 0
            assert result[0] == key_dict[key]
        elif key in extra_strings:
            #print("bpp", rc, result[0])
            assert rc != 0

def test_get():
    print("{0:-<40}".format("testing test_get"))
    ttp = make_ttrie_though_convert(CONVERT_DATA)

    #TTRIE_SO.print_result_id(ttp.root_id);
    #TTRIE_SO.ttrie__dprint(ttp);

    print("{0:-<40}".format("Testing get"))
    get_check(ttp, CONVERT_DATA, EXTRA_SEARCH_STRINGS)  
    
    print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0


def check_iter(ttp, CONVERT_DATA):
    tt_iter = ffi.new("struct ttrie__iterator*")
    TTRIE_SO.ttrie__iter(tt_iter, ttp)


    expected_data = sorted(dict(CONVERT_DATA).items(), key=lambda x: x[0])
    value = ffi.new("unsigned long long **")
    klen = ffi.new("int *")
    key = ffi.new("char[256]")
    for expected_key, expected_val in expected_data:
        rc = TTRIE_SO.ttrie__next_node(tt_iter, key, klen, value)
        if rc != 0:
            print("ERROR early termination")
            assert False
            break

        actual_key = ffi.buffer(key, klen[0])[:]        
        actual_value = value[0][0]
        #print("Got value={}, len={}, key={}".format(actual_value, klen[0], actual_key))
        print("actual_key={} expected_key={}".format(actual_key, expected_key))    
        assert actual_key == expected_key
        print("actual_value={} expected_val={}".format(actual_value, expected_val))
        assert actual_value == expected_val
    
    assert(TTRIE_SO.ttrie__next_node(tt_iter, key, klen, value)!=0)
    TTRIE_SO.ttrie__close_iter(tt_iter);


def test_iter():
    #print("{0:-<40}".format("testing test_iter"))
    ttp = make_ttrie_though_convert(CONVERT_DATA)

    #TTRIE_SO.print_result_id(ttp.root_id);
    #TTRIE_SO.ttrie__dprint(ttp);

    print("{0:-<40}".format("Testing iter"))
    check_iter(ttp, CONVERT_DATA)  

    #print("{0:-<40}".format("testing ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp)
    assert rc == 0

def test_write_read():
    print("{0:-<40}".format("testing test_write_read"))
    ttp_write = make_ttrie_though_convert(CONVERT_DATA)

    print("{0:-<40}".format("finished convert"))
    file_path = ffi.new("char []", "data/ttrie/foobar.ttrie".encode('ascii'))

    print("{0:-<40}".format("current state pre write"))
    TTRIE_SO.print_result_id(ttp_write.root_id);
    TTRIE_SO.ttrie__dprint(ttp_write);

    print("{0:-<40}".format("writting to file"))
    rc = TTRIE_SO.ttrie__write(ttp_write, file_path)
    assert rc == 0

    expected_root_node = ffi.buffer(ttp_write.root_id, 5)
    expected_n_keys = int(ttp_write.len)

    print("{0:-<40}".format("ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp_write)
    assert rc == 0

    ttp_read = ffi.new("struct ttrie__obj *")
    mode_str = ffi.cast("char", "r".encode('ascii'))

    print("{0:-<40}".format("ttrie read open"))
    rc = TTRIE_SO.ttrie__open(ttp_read, mode_str)
    assert rc == 0

    print("{0:-<40}".format("ttrie reading from file"))
    rc = TTRIE_SO.ttrie__read(ttp_read, file_path)
    assert rc == 0
    print("{0:-<40}".format("ttrie finished reading from file"))

    print("{0:-<40}".format("current state post read"))
    TTRIE_SO.print_result_id(ttp_read.root_id);
    TTRIE_SO.ttrie__dprint(ttp_read);

    print("{0:-<40}".format("Testing get"))
    get_check(ttp_read, CONVERT_DATA, EXTRA_SEARCH_STRINGS)  

    actual_root_node = ffi.buffer(ttp_read.root_id, 5)
    actual_n_keys = int(ttp_read.len)

    assert expected_root_node[:] == actual_root_node[:]
    assert expected_n_keys == actual_n_keys

    print("{0:-<40}".format("ttrie close"))
    rc = TTRIE_SO.ttrie__close(ttp_read)
    assert rc == 0
