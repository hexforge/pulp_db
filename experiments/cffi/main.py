from cffi import FFI
ffi = FFI()

printf_str = """
int printf(const char *format, ...);
"""

ffi.cdef(printf_str)

C = ffi.dlopen(None)
arg = ffi.new("char []", b"world")
C.printf(b"mmmmmmm %s!\n", arg)