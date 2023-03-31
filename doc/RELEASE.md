# How to release a new primesieve version

* Update the [ChangeLog](../ChangeLog) file.
* Run ```scripts/update_version.sh``` in the root directory of the primesieve git repo to update the version number. This script takes the new version number as a parameter e.g.: ```scripts/./update_version.sh 1.2```
* Go to the [GitHub website and do the release](https://github.com/kimwalisch/primesieve/releases). The release title should be primesieve-X.Y and the tag name should be vX.Y (e.g. primesieve-1.0 and v1.0).
