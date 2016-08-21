Release process
===============

Benchmark code changes:
```sh
$ ./primesieve 1e13 --dist=1e11
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

Increase the primesieve version and the primesieve libtool version before
each new release. The ```update_version.sh``` script automatically
updates the version (and release date) in all files.

```sh
cd scripts

# Usage example: update primesieve version to 1.2.3 and 
# update primesieve libtool version to 7:0:0
./update_version.sh 1.2.3 7:0:0
```
[This page](https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html)
explains libtool versioning.

### Release process

* Run tests using ```make check```
* Increase version number (see <a href="#versioning">Versioning</a>)
* Update [ChangeLog](ChangeLog)
* Tag the new release in git
* Build statically linked primesieve binaries and upload them to [https://bintray.com/kimwalisch/primesieve](https://bintray.com/kimwalisch/primesieve)
* Create new release tarball using ```make dist``` and upload it to [https://bintray.com/kimwalisch/primesieve](https://bintray.com/kimwalisch/primesieve)
* Update http://primesieve.org
* Update http://primesieve.org/api
