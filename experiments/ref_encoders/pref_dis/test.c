#include <stdio.h>

#include "pref.h"

static const char test_file_name[] = "test_pref.file";

int main()
{
    int error;
    printf("Starting pref test\n");

    /*---
    WRITING
    --*/
    struct pref__obj out_obj;
    struct pref__buffer out_buf;
    
    error = pref__open(&out_obj, test_file_name, 'w');
    if (error)
        printf("ERROR: Failed to open out file %d\n", error);

    error = pref__setup_buffer(&out_obj, &out_buf);  // offset only used in read mode;
    if (error)
        printf("ERROR: Failed to setup pref buffer (out)\n");

    for (int i=0; i<20; ++i)
    {
        pref__append(&out_obj, &out_buf, i);
        printf("Have appended value %d \n", i);
    }

    error = pref__teardown_buffer(&out_obj, &out_buf);
    if (error)
        printf("ERROR: Problem teardown pref buffer\n");

    error = pref__close(&out_obj);
    if (error)
        printf("ERROR: Failed to close pref out file\n");

    /*---
    READING
    ---*/
    struct pref__obj in_obj;
    struct pref__buffer in_buf;
    
    error = pref__open(&in_obj, test_file_name, 'r');
    if (error)
        printf("ERROR: Failed to open in file\n");

    error = pref__setup_buffer(&in_obj, &in_buf, 0);  // offset only used in read mode;
    if (error)
        printf("ERROR: Failed to setup pref buffer (in)\n");

    int count = 0;
    signed long long ref = pref__geti(&in_obj, &in_buf, count);
    

    printf("Iterativing over the pref in file\n");
    while (ref!=-1)
    {
        printf("Element %d, reference='%lld'\n", count, ref);
        count += 1;
        ref = pref__geti(&in_obj, &in_buf, count);    
    }

    error = pref__teardown_buffer(&in_obj, &in_buf);
    if (error)
        printf("ERROR: Failed to teardown pref buffer (in)\n");

    error = pref__close(&in_obj);
    if (error)
        printf("ERROR: Failed to close pref in file\n");

}