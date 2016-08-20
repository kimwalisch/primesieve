Build manpage & API documentation
=================================

In order to build the primesieve manpage and the html API
documentation you need to have installed the ```help2man``` and
```doxygen``` programs. Run the commands below from the parent
directory.

```bash
$ ./configure --with-help2man
$ make man
$ make doxygen
```
