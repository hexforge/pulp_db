#ifndef flineH
#define flineH

/*---
PUBLIC INTERFACE
---*/
int fline__setup(char *file_path);
int fline__close(void);
int fline__get_line(char *data, const int maxlen);

#endif //flineH