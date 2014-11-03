from cffi import FFI
ffi = FFI()

add_str = """
struct two_nums {
    int x;
    int y;
};
"""

add_str2 = """
int add_struct(struct two_nums *thing);
"""
ffi.cdef(add_str)
ffi.cdef(add_str2)

data = ffi.new("struct two_nums *") #, [1,2])
data.x = 2
data.y = 3

C = ffi.dlopen("foobar.so")
C.add_struct(data)