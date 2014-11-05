#ifndef metaparseH
#define metaparseH

struct metaparse__pb
{
    struct mmbuf__obj *m;
    char *data;
    long long offset;
};

struct metaparse__msg
{
    char ip[16];
    char port[6];
    char time[18];
    unsigned int header_size;
    unsigned int payload_size;
    char *msg;
};

int metaparse__setup(struct metaparse__pb *m, const char *file_path, const char *mode);
int metaparse__teardown(struct metaparse__pb *m);
int metaparse__get_msg(struct metaparse__pb *m, struct metaparse__msg *pmsg);

#endif //metaparseH
