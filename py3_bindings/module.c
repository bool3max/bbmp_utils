#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "bbmp_helper.h"
#include "bbmp_parser.h"

/* static PyObject *func_createimage(PyObject *self, PyObject *args, PyObject *kwargs) */
/* { */
/*     // arbitrary call */
/*     bbmp_vertflip(NULL); */

/*     static char *kw_options[] = {"resolution", "width", "height", "name", NULL}; */

/*     const char *name; */
/*     int res, wid, hei; */

/*     if(!PyArg_ParseTupleAndKeywords(args, kwargs, "iiis", kw_options, &res, &wid, &hei, &name)) { */
/*         // cannot parse tuple -- the function raises an appropriate python exception and therefore returns 0, which we catch here */
/*         fprintf(stderr, "lib: failed parsing tuple...\n"); */
/*         // return a NULL pointer indicating failure */
/*         return NULL; */
/*     } */
    
/*     fprintf(stdout, "lib: got: resolution: %d\n\twidth: %d\n\theight: %d\n\tname: %s\n", res, wid, hei, name); */

/*     return PyLong_FromLong(33); // PyLong_FromLong returns a pointer to an instance of a python integer object */
/* } */

static PyObject *e_parse_metadata(PyObject *self, PyObject *args) {
    /* Taking a form of a python byte object as an argument, read it, parse its metadata using bbmp_utils' bbmp_parse_bmp_metadata function and return
     * a python dictionary representing the bbmp_Metadata structure
    */

    Py_buffer buf;

    if(!PyArg_ParseTuple(args, "y*", &buf)) {
        // interpreter raises exception, we return NULL to indicate failure
        return NULL;
    }
    
    bbmp_Metadata meta = {0};

    bbmp_parse_bmp_metadata(buf.buf, &meta);
    bbmp_debug_bmp_metadata(&meta);

    PyBuffer_Release(&buf);

    // construct a python dictionary object and fill it with the data from the bbmp_Metadata structure
    PyObject *dict = Py_BuildValue("{s:s, s:I, s:H, s:H, s:I, s:I, s:i, s:i, s:H, s:H, s:I, s:I, s:i, s:i, s:I, s:I, s:I, s:H, s:I, s:I, s:I, s:I}",
                                    "header_iden",
                                    meta.header_iden,
                                    "filesize",
                                    meta.filesize,
                                    "res1",
                                    meta.res1,
                                    "res2",
                                    meta.res2,
                                    "pixelarray_off",
                                    meta.pixelarray_off,
                                    "dib_size",
                                    meta.dib_size,
                                    "pixelarray_width",
                                    meta.pixelarray_width,
                                    "pixelarray_height",
                                    meta.pixelarray_height,
                                    "panes_num",
                                    meta.panes_num,
                                    "bpp",
                                    meta.bpp,
                                    "compression_method",
                                    meta.compression_method,
                                    "pixelarray_size",
                                    meta.pixelarray_size,
                                    "ppm_horiz",
                                    meta.ppm_horiz,
                                    "ppm_vert",
                                    meta.ppm_vert,
                                    "colors_num",
                                    meta.colors_num,
                                    "colors_important_num",
                                    meta.colors_important_num,
                                    "Bpr",
                                    meta.Bpr,
                                    "Bpp",
                                    meta.Bpp,
                                    "Bpr_np",
                                    meta.Bpr_np,
                                    "resolution",
                                    meta.resolution,
                                    "padding",
                                    meta.padding,
                                    "pixelarray_size_np",
                                    meta.pixelarray_size_np);
    if(!dict) return NULL; // interpreter raises an exception, we return NULL

    return dict;
}

// method table (describe all the methods exposed by this particular extension module)
static PyMethodDef module_methods[] = {
    /* {"create_image", (PyCFunction) func_createimage, METH_VARARGS | METH_KEYWORDS, "test"}, */ 
    {"parse_metadata", (PyCFunction) e_parse_metadata, METH_VARARGS, "Parse the metadata out of a BMP file"},
    {NULL, NULL, 0, NULL} // sentinel
};

static PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "bbmp_utils", // module name
    "A library for efficient BMP file manipulation", // module description
    -1, // per-interpreter state (?)
    module_methods // the methods that the module exposes
};

PyMODINIT_FUNC PyInit_bbmp_utils(void) {
    return PyModule_Create(&module); // create a new module using the structure we've defined at file scope
}
