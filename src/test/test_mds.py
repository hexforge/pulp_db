import os

from cffi import FFI
ffi = FFI()

MDS_STRUCTS = """
struct mds_obj {
    char mode;
    unsigned short num_in_clump;
    long long current_block_index;
    long long msg_count;
    ...;
};

"""

MDS_API = """
int mds__setup(struct mds_obj *m, char *dirpath, char mode);
int mds__teardown(struct mds_obj *m);
signed long long mds__append(struct mds_obj *m, char *data, int length);
unsigned int mds__get(struct mds_obj *m, long long index, char **result);
long long mds__len(struct mds_obj *m);
"""
folder_path = os.path.dirname(__file__)
so_path = os.path.abspath(os.path.join(folder_path, '../libs'))
headers_path = os.path.abspath(os.path.join(folder_path, '../headers'))
#c_file_path = os.path.abspath(os.path.join(folder_path, '../pulp_db/kds'))

ffi.cdef(MDS_STRUCTS)
ffi.cdef(MDS_API)


MDS_SO = ffi.verify("""#include "mds.h" """, 
                      libraries=["mds"], 
                      library_dirs=[so_path],
                      runtime_library_dirs=[so_path],
                      include_dirs=[headers_path],
                      extra_compile_args=["-std=c99", "-Wno-unused-function"])

db_path = "data/mds"
if not os.path.exists(db_path):
    os.mkdir(db_path)
db_path_b = db_path.encode("ascii")

DATA = []

def populate_db(m):
    
    print("Opening the db for writing")
    MDS_SO.mds__setup(m, db_path_b, b'w')

    print("Writing data in to the db")
    for i in range(100):
        string = "Hello".encode("ascii") + str(i).encode("ascii")
        DATA.append(string)
        d = ffi.new("char[]", string)
        print("Adding string", string)
        MDS_SO.mds__append(m, d, len(string))
    
    print("Closing the db")
    MDS_SO.mds__teardown(m)

READ_DB = None
def check_mds(i):
    m = READ_DB
    assert(READ_DB is not None)
    result_p = ffi.new("char **")
    length = MDS_SO.mds__get(m, i, result_p)
    assert (length!=0)
    data = ffi.buffer(result_p[0], length)
    print("Got message({})=={}".format(i, data[:]))
    assert data[:] == DATA[i]

def test_mds():
    global READ_DB
    m = ffi.new("struct mds_obj *")
    populate_db(m)
    READ_DB = m

    print("READING MODE ---------------------")
    print("Opening the db for reading")
    MDS_SO.mds__setup(m, db_path_b, b'r')

    print("CALCULATING DB LENGTH---------")
    db_len = MDS_SO.mds__len(m)

    print("db len =", db_len)

    print("ITERITING ---------------------")
    
    for i in range(db_len):
        yield check_mds, i

    print("Closing the db")
    MDS_SO.mds__teardown(m)
    return 0

if __name__ == "__main__":
    test_mds()
