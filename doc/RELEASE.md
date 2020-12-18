# How to release a new primesieve version

* Update the [ChangeLog](../ChangeLog) file.
* Run ```scripts/update_version.sh``` in the root directory of the primesieve git repo to update the version number.
* Run ```scripts\build_msvc.bat``` in the root directory of the primesieve git repo to build a primesieve release binary for Windows.
* Build the Doxygen API documentation using ```cmake . -DBUILD_DOC=ON && make doc``` and update the ```gh_pages``` branch.
* Finally go to the [GitHub website and do the release](https://github.com/kimwalisch/primesieve/releases). The release title should be primesieve-X.Y and the tag name should be vX.Y (e.g. primesieve-1.0 and v1.0).
