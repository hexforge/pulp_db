
#include <Python.h>


void (*pydo)(void);

int main()
{
    Py_Initialize();
    
    PyObject *pModule = PyImport_Import(PyUnicode_FromString("callback"));
    if (pydo)
    {
        printf("Going gogogogo\n");   
        pydo();
    }
    else
    {
        printf("pydo null\n");
    }
    printf("%p\n", pModule);
    Py_Finalize();
    return 0;
}
