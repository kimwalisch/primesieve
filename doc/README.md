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

primesieve includes an up to date man page at ```doc/primesieve.1```.
That man page has been generated from ```doc/primesieve.txt``` using
the ```a2x``` program from the ```asciidoc``` package. However when
packaging primesieve for e.g. a Linux distro it is recommended to
regenerate the man page.

```bash
# Build man page using a2x program (asciidoc package)
cmake -DBUILD_MANPAGE=ON .
make -j
```
