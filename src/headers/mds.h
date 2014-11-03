#ifndef mdsH
#define mdsH

#define PAGE_SIZE 4096

#include "bitpack.h"
#include "mmbuf.h"

struct mds_obj {
    char mode;
    unsigned short num_in_clump;
    long long current_block_index;
    long long msg_count;
    struct bitpack__clump *index;
    struct mmbuf_obj *data_file;
    struct mmbuf_obj *index_file;
};

int mds__setup(struct mds_obj *m, char *dirpath, char mode);
int mds__teardown(struct mds_obj *m);
signed long long mds__append(struct mds_obj *m, char *data, int length);
unsigned int mds__get(struct mds_obj *m, long long index, char **result);
long long mds__len(struct mds_obj *m);

#endif       //mdsH


// When someone uses this they either want to 

// open a db
// put some data into it.    All they want is rid of the data.  Each call will have   ##  db_stuff + data
// Get some data out of it.  Want to get a message out of it.  db and number.   Need them to assign storage?
// Need to iter in this case we need buffer last calculation.
