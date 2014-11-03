print("moo")
from cffi import FFI
ffi = FFI()

ffi.cdef("""void (*pydo)(void); \n """)

def pydo():
    print("erm")

thing = ffi.verify("""void (*pydo)(void);""")

cb = ffi.callback("void()", pydo)
thing.pydo = cb

print("oooooo")
