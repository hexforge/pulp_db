
#include <stdio.h>
#include "rtrie.h"

struct pulp_context
{
    char db_name[256];
    char mode;
    char *field_names;
    struct rtrie__view *rties;
    struct dpref__obj *drefs;
    struct dpref__buffer **buffers;  //array of pointers to buffer arrays
};


// Writing mode.
int pulp__open(struct pulp_context *p, char *db_name. char mode);
unsigned long long pulp__append(struct pulp_context *p, void *data. int n);
int pulp__index(struct pulp_context *p, char *fieldname, void *value, int n, unsigned long long msg_num);
int pulp__optimize(struct pulp_context *p);
int pulp__close(struct pulp_context *p);



