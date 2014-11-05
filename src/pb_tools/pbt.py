import os

from cffi import FFI
ffi = FFI()

str_metaparse__pb = """
struct metaparse__pb
{
    struct mmbuf__obj *m;
    unsigned char *data;
    long long offset;
};
"""

str_metaparse__msg = """
struct metaparse__msg
{
    char ip[16];
    char port[6];
    char time[18];
    unsigned int header_size;
    unsigned int payload_size;
    unsigned char *msg;
};
"""

str_meta_parse_calls = """
int metaparse__setup(struct metaparse__pb *m, const char *file_path, const char *mode);
int metaparse__teardown(const struct metaparse__pb *m);
int metaparse__get_msg(struct metaparse__pb *m, struct metaparse__msg *pmsg);
"""

this_dir = os.path.dirname(__file__)
so_path = os.path.abspath(os.path.join(this_dir, "meta.so"))
def metaparse(playback):
    ffi.cdef(str_metaparse__pb)
    ffi.cdef(str_metaparse__msg)
    ffi.cdef(str_meta_parse_calls)

    data = ffi.new("struct metaparse__pb *")
    msg = ffi.new("struct metaparse__msg *")
    filename = ffi.new("char[]", playback.encode('ascii'))
    mode = "rs".encode("ascii")

    C = ffi.dlopen(so_path)
    C.metaparse__setup(data, filename, mode)
    
    while True:
        rc = C.metaparse__get_msg(data, msg)
        if rc != 0:
            break
        new_msg = {}
        new_msg['ip'] = ffi.string(msg.ip)
        new_msg['port'] = ffi.string(msg.port)
        new_msg['time'] = ffi.string(msg.time)
        header_size = msg.header_size
        new_msg['header_size'] = header_size
        payload_size = msg.payload_size
        new_msg['payload_size'] = payload_size

        new_msg['msg'] =  ffi.buffer(msg.msg, header_size+payload_size)[:]
        new_msg['payload'] = new_msg['msg'][header_size:]
        new_msg['orig'] = msg

        yield new_msg
    
    C.metaparse__teardown(data)

def parse_pb(playback):
    for msg in metaparse(playback):
        yield msg

def get_decoder(spec):
    return

if __name__ == '__main__':
    for msg in parse_pb('example.pb'):
        print(msg)
