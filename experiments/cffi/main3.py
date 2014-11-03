from cffi import FFI
ffi = FFI()

add_str = """
int add(int x, int y);
"""

ffi.cdef(add_str)   # Many????

C = ffi.dlopen("foobar.so")
C.add(10, 12)
