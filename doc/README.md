Build API documentation
=======================

To build the primesieve C/C++ API documentation in format html/PDF
you need to have installed ```doxygen```, ```doxygen-latex``` and
```graphviz (dot)```. Run the commands below from the parent
directory.

```bash
# Build C/C++ API documentation
cmake -DBUILD_DOC=ON .
make doc
```

Build man page
==============

To regenerate the primesieve man page you need to have installed
the ```help2man``` program. Run the commands below from the parent
directory.

```bash
# Build man page
cmake .
make
```
