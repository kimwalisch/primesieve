Hacking on primesieve
=====================

Benchmark your code changes:
```sh
$ time ./primesieve 1e13 --offset=1e11
```

Fix all warnings:
```sh
$ make clean
$ make CXXFLAGS="-Wall -Wextra -Werror -Wno-long-long -pedantic -O2"
```

Run integration tests:
```sh
$ make check
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
* Increase version number in [Doxyfile](Doxyfile)
* Increase version number in [configure.ac](configure.ac) in ```AC_INIT```
* [Increase Libtool version](http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html) number in [configure.ac](configure.ac) in ```AC_SUBST```
* Update current year in [src/apps/console/help.cpp](src/apps/console/help.cpp)
* Update current year in [src/apps/gui/src/PrimeSieveGUI_const.hpp](src/apps/gui/src/PrimeSieveGUI_const.hpp)

### Release process

* Run tests using ```make check```
* Increase version number (see <a href="#versioning">Versioning</a>)
* Update [ChangeLog](ChangeLog)
* Tag the new release in git
* Build statically linked primesieve binaries and upload them to [https://bintray.com/kimwalisch/primesieve](https://bintray.com/kimwalisch/primesieve)
* Create new release tarball using ```make dist``` and upload it to [https://bintray.com/kimwalisch/primesieve](https://bintray.com/kimwalisch/primesieve)
* Update http://primesieve.org
* Update http://primesieve.org/api
