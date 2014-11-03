import os
import dbm

from cffi import FFI
ffi = FFI()

MDS_STRUCT = """
struct mds_obj {
    char mode;
    unsigned short num_in_clump;
    long long current_block_index;
    long long msg_count;
    struct bitpack__clump *index;
    struct mmbuf_obj *data_file;
    struct mmbuf_obj *index_file;
};
"""

MDS_API = """
int mds__setup(struct mds_obj *m, char *dirpath, char mode);
int mds__teardown(struct mds_obj *m);
signed long long mds__append(struct mds_obj *m, char *data, int length);
unsigned int mds__get(struct mds_obj *m, long long index, char **result);
long long mds__len(struct mds_obj *m);
"""

ffi.cdef(MDS_STRUCT)
ffi.cdef(MDS_API)

folder_path = os.path.dirname(__file__)
full_so_path = os.path.join(folder_path, "libmds.so")
MDS_SO = ffi.dlopen(full_so_path)

class MasterTable(object):
    def __init__(self, dirname, mode, dumper=None, loader=None):
        self.dirname = ffi.new("char[]", dirname.encode('ascii'))
        self.mode = mode.encode('ascii')
        self.data = ffi.new("struct mds_obj *")
        self.len = None
        self.dumper = dumper
        self.loader = loader

    def __enter__(self):
        MDS_SO.mds__setup(self.data, self.dirname, self.mode)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        MDS_SO.mds__teardown(self.data)
        self.data = None
        print("TEARDOWN master table")
    
    def __len__(self):
        if self.len is None:
            self.len = MDS_SO.mds__len(self.data)
        return self.len

    def __getitem__(self, index):
        if isinstance(index, int):
            pp_str = ffi.new("char **")
            # We want to give it a pointer to which we can then use the len
            length = MDS_SO.mds__get(self.data, index, pp_str)
            str_copy = ffi.buffer(pp_str[0], length)[:]
            if self.loader is not None:
                str_copy = self.loader(str_copy)
            return str_copy
        elif isinstance(index, slice):
            start, stop, step = index.indices(len(self))
            def sliceresults(self, start, stop, step):
                i = start
                while (i < stop):            
                    yield self[i]
                    i += step
            return sliceresults(self, start, stop, step)

    def __iter__(self):
        return (self[i] for i in range(len(self)))
    
    def append(self, msg):
        if self.dumper is not None:
            msg = self.dumper(msg)

        assert type(msg) == type("".encode('ascii'))
        size = len(msg)
        msg_num = MDS_SO.mds__append(self.data, msg, size)
        assert msg_num >= 0
        return msg_num

if __name__ == '__main__':
    # Little test
    with MasterTable("tmp", "w") as db:
        for x in range(10):
            db.append(("Foobar" + str(x+1) + '\n').encode('ascii'))

    with MasterTable("tmp", "r") as db:
        print("length=", len(db))
        for x in db:
            print(x)
