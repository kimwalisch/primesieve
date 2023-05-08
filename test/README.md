# primesieve testing

Run the commands below from the primesieve root directory.

```bash
cmake . -DBUILD_TESTS=ON
cmake --build . --parallel
ctest
```

# Test in debug mode

When hacking on primesieve's source code, it is best to run its test suite
in debug mode i.e. ```-DCMAKE_BUILD_TYPE=Debug``` because this enables
extensive (but slow) runtime assertions. In case an assertion is triggered,
the file name and line number where the error occurred will be printed to
the screen. This helps to quickly identify newly introduced bugs.

```bash
# Run commands from primesieve root directory
cmake . -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O1 -Wall -Wextra -pedantic" -DCMAKE_C_FLAGS="-O1 -Wall -Wextra -pedantic"
cmake --build . --parallel
ctest --output-on-failure
```

# Test using GCC/Clang sanitizers

Running primesieve's test suite with sanitizers enabled is also very useful
as this helps find undefined behavior bugs and data races.

```bash
# Run commands from primesieve root directory
cmake . -DBUILD_TESTS=ON -DCMAKE_CXX_FLAGS="-g -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -Wall -Wextra -pedantic" -DCMAKE_C_FLAGS="-g -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -Wall -Wextra -pedantic"
cmake --build . --parallel
ctest --output-on-failure
```
