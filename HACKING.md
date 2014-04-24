Hacking on primesieve
=====================

### Hacking on Unix-like OSes

Clone or fork primesieve:
```sh
$ git clone https://github.com/kimwalisch/primesieve.git
```

In order to hack primesieve you need to have installed a C++ compiler
and the GNU Build System (a.k.a Autotools). To install the GNU Build
System install
[GNU Autoconf](http://www.gnu.org/software/autoconf/),
[GNU Automake](http://www.gnu.org/software/automake/) and
[GNU Libtool](http://www.gnu.org/software/libtool/) using your packet
manager.

Generate configure script (only once):
```sh
$ ./autogen.sh
```

Then build primecount using:
```sh
$ ./configure
$ make
```

### Where the source code lives

* The sieve of Eratosthenes implementation lives in [src/primesieve](src/primesieve)
* The sieve of Eratosthenes header files live in [include/primesieve](include/primesieve)
* The primesieve console application lives in [src/apps/console](src/apps/console)
* The primesieve GUI application (uses Qt framework) lives in [src/apps/gui](src/apps/gui)

### Adding a new source file

* Add new cpp file to [src/primesieve](src/primesieve)
* Add its header file to [include/primesieve](include/primesieve)
* Add source and header files to [Makefile.am](Makefile.am) and [Makefile.msvc](Makefile.msvc)
* Add source file to [src/apps/gui/primesieve.pro](src/apps/gui/primesieve.pro)

### Versioning

* Increase version number in [include/primesieve.hpp](include/primesieve.hpp)
* Increase version number in [include/primesieve.h](include/primesieve.h)
* Increase version number in [configure.ac](configure.ac) in ```AC_INIT```
* [Increase Libtool version](http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html) number in [configure.ac](configure.ac) in ```AC_SUBST```
* Update current year in [src/apps/console/help.cpp](src/apps/console/help.cpp)
* Update current year in [src/apps/gui/src/PrimeSieveGUI_const.hpp](src/apps/gui/src/PrimeSieveGUI_const.hpp)

### Release process

* Run tests using ```make check```
* Increase version number (see <a href="#versioning">Versioning</a>)
* Build statically linked primesieve binaries and upload them to [https://bintray.com](https://bintray.com)
* Update [ChangeLog](ChangeLog)
* Tag the new release in git
* Create a new release tarball using ```make dist``` and upload it to [https://bintray.com](https://bintray.com)
