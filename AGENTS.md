# Project instructions

These instructions apply throughout the primesieve repository.

## Project overview

primesieve is a command-line program and C/C++ library for generating primes and prime k-tuplets up to 2^64. It uses the segmented sieve of Eratosthenes with wheel factorization and Tomás Oliveira e Silva's bucket sieve, and sizes its sieve array according to the CPU's L1 and L2 cache sizes. primesieve is multi-threaded using `std::async` (not OpenMP) and uses SIMD algorithms, e.g. for pre-sieving and for converting sieve bits into primes, with runtime dispatch to the fastest instruction set supported by the CPU.

Read `README.md` once at the beginning of each new session before making code changes to understand the project's capabilities, usage, and algorithm overview. Consult relevant sections again as needed. Use the build and verification instructions in this `AGENTS.md` when developing.

## Required reading

primesieve has no mandatory papers; its documentation explains the design. Read `src/README.md` (overview of the C++ classes) before editing code in `src/`, `doc/ALGORITHMS.md` before editing the sieving algorithms (`src/Erat*`, `src/PreSieve*`, `src/Wheel.hpp`, `src/Bucket.hpp`, `src/MemoryPool.*`, `src/SievingPrimes.*`), `doc/C_API.md` and `doc/CPP_API.md` before changing the public API, and `doc/BUILD.md` and `test/README.md` (Debug builds with assertions, sanitizers) before changing the build system or tests.

Read each relevant document once per agent session/chat, when work on the corresponding area first begins and before the first code edit. For example, work on `src/EratMedium.cpp` requires reading both `src/README.md` and `doc/ALGORITHMS.md`. Reuse that reading for subsequent edits in the same session; do not reread a document for every edit or message. When work moves to another area, read any required documents that have not yet been read in the session. In a new session/chat, read the relevant documents again before editing.

When a change makes one of these documents incorrect, update the document as part of the change.

## Vendored libraries

Exclude this vendored library from changes to the primesieve codebase, including requests to change all occurrences of a pattern:

- calculator: `include/primesieve/calculator.hpp`

Fixes to it belong in its upstream repository, https://github.com/kimwalisch/calculator.

primecount (https://github.com/kimwalisch/primecount) in turn vendors the complete primesieve tree, including this file, in `lib/primesieve` and builds it using `add_subdirectory()`. Keep such subproject builds working: settings that would affect the parent project, such as the default build type, are only applied if `PRIMESIEVE_IS_TOP_PROJECT` is ON. Do not edit primecount's copy; fixes belong in the primesieve repository.

## Preserve user changes

Preserve existing user changes, including staged changes. Do not revert, overwrite, or recreate code the user intentionally changed or removed unless the requested task requires it.

Do not run the scripts in `scripts/` unless the user asks: the `build_*` scripts delete build directories and download files, `benchmark-vs-prev-release.sh` checks out the previous release in the working tree, and `update_version.sh` changes the version numbers.

## Compatibility

Keep primesieve's implementation compatible with C++14 and preserve existing platform support. C++14 is only a requirement for building primesieve itself: applications using its public C++ headers (`include/primesieve.hpp` and the headers it includes) and linking against the library must continue to work with C++11 or later. Keep public headers compatible with C++11 and avoid propagating a C++14 requirement to library consumers through build or package metadata. The C headers `include/primesieve.h` and `include/primesieve/iterator.h` must remain valid C (CI compiles the C tests with `-pedantic`) and usable from C++. Installed headers may only include each other and standard headers. Do not introduce newer language requirements, dependencies, or public API changes unless the task calls for them.

primesieve supports numbers < 2^64. The shared library's SOVERSION is the major version, so do not change the public ABI, e.g. the layout of `primesieve_iterator`, unless the task requires it. Keep primesieve working on x86 and x64 (including 32-bit), ARM64, 32-bit ARM and other CPUs (portable `_default` algorithms), Emscripten/WebAssembly, Linux, macOS, and Windows with MinGW-w64 GCC, MSVC and clang-cl. `cmake/libatomic.cmake` handles old 32-bit systems that need libatomic.

## Coding conventions

primesieve has no official coding style guide that can be enforced by a tool. Infer the coding style from the file being edited and follow its existing formatting. If the file is too small or lacks examples of the code construct being written, inspect one or two other source files to determine how to format it.

Keep changes limited to the requested task. Avoid unrelated refactoring, renaming, formatting, or whitespace changes. Preserve the existing file encoding and line endings.

- Do not break a variable initialization immediately after `=` except in rare cases, such as lookup tables whose brace-enclosed initializer starts on the next line.
- Prefer a multi-line `if` condition over introducing a boolean variable used only for that condition.
- Naming: camelCase for functions, methods and local variables (`fillNextPrimes()`, `bytesToCopy`), PascalCase for classes and their files (`EratSmall.cpp`), a trailing underscore for class member variables (`sieveIdx_`, but not in plain structs such as `CmdOptions`), and UPPER_CASE for macros (except `if_likely()` and `if_unlikely()`), enumerators and `config.hpp` constants. snake_case is only established in the public C/C++ API (`count_primes()`, `primesieve_*()`), the std-like code in `Vector.hpp`, CPU feature detection (`has_avx2()`) and a few other names such as `max_n` in `nthPrime.cpp`. SIMD variants use the suffixes `_x86_avx512`, `_x86_avx2`, `_x86_sse2`, `_arm_sve`, `_arm_neon` and `_default`. Never rename existing identifiers to fit these rules.
- Put file-local functions and constants into an anonymous namespace instead of making them `static`.
- In `src/`, include the headers from `include/primesieve/` as `<primesieve/...>`, e.g. `<primesieve/cpu_arch_macros.hpp>`, not `<cpu_arch_macros.hpp>`; these headers include each other with quotes (`"macros.hpp"`).
- Split compound preprocessor conditions in `#if` and `#elif` directives across multiple lines, with one condition per line. Use `\` line continuations and align the continued conditions. Preserve logical operators and parentheses.
- SIMD code is selected by `include/primesieve/cpu_arch_macros.hpp`. Its native macros `ENABLE_AVX512_VBMI2`, `ENABLE_AVX2_BMI2`, `ENABLE_AVX512_BW`, `ENABLE_AVX2` and `ENABLE_ARM_SVE` (as well as `ENABLE_SSE2` and `ENABLE_ARM_NEON`, which have no multiarch counterpart) follow the compiler flags. For runtime dispatch, the build system defines `ENABLE_MULTIARCH_AVX512_VBMI2`, `ENABLE_MULTIARCH_AVX512_BW`, `ENABLE_MULTIARCH_AVX2`, `ENABLE_MULTIARCH_ARM_SVE` and `ENABLE_MULTIARCH_x86_POPCNT` (lower-case `x86`) if their `cmake/multiarch_*.cmake` probes succeed. `cpu_arch_macros.hpp` adds `ENABLE_MULTIARCH_AVX2_BMI2`, which `PrimeGenerator.hpp` uses because its AVX2 algorithm needs runtime dispatch under different conditions than the one in `PreSieve.cpp` (`ENABLE_MULTIARCH_AVX2`), and it undefines multiarch macros made redundant by native ones. Hence files using these macros must include it, directly or via e.g. `PrimeGenerator.hpp`; only `ENABLE_MULTIARCH_x86_POPCNT` (`popcnt.hpp`) does not depend on it.
- Native builds such as `-march=native` should use the SIMD implementation directly without runtime CPU-feature checks. `PreSieve.cpp` and `PrimeGenerator.hpp`/`.cpp` select the fastest native algorithm using an `#if`/`#elif` chain that defines `*_DEFAULT_HEADER` and ends with the portable `_default` algorithm, include `<primesieve/cpu_supports_<isa>.hpp>` and the kernel header in separate `#if defined(ENABLE_MULTIARCH_<ISA>)` blocks, and dispatch by testing the `cpu_supports_*` flags, fastest first. Where native and multiarch cases share one `#if` chain (`popcnt.hpp`, `cpuInfo()` in `src/app/main.cpp`), put the native case first and the multiarch case in `#elif`.
- Annotate multiarch SIMD functions, on both declaration and definition, with `PRIMESIEVE_TARGET_ATTRIBUTE(isa)` inside `#if defined(ENABLE_MULTIARCH_<ISA>)`. It is a target attribute with GCC/Clang (including clang-cl) and `NOINLINE` with native MSVC, which must not inline SIMD code into scalar code (see `macros.hpp`). Give SIMD helpers the same annotation and `isa` string as their callers (e.g. `sieveWordsToPrimes()`), never call them from scalar code, and do not make them `ALWAYS_INLINE`. Never pass SIMD vector types into or out of a dispatched function.
- Runtime dispatch for a new instruction set needs, as for the AVX2 `PrimeGenerator` algorithm:
  1. A `cmake/multiarch_<isa>.cmake` probe compiling real project code (`cpu_arch_macros.hpp`, `macros.hpp`, `src/arch/x86/cpuid.cpp` or `src/arch/arm/sve.cpp`), included in `CMakeLists.txt`.
  2. `has_<isa>()` in `src/arch/`, checking CPU and OS support, and `include/primesieve/cpu_supports_<isa>.hpp`.
  3. Updates to `cpu_arch_macros.hpp`, the kernel header and the dispatch code.
  4. The `--cpu-info` output in `src/app/main.cpp`.
  5. `-DENABLE_MULTIARCH_<ISA>` in `scripts/build_clang_*.ps1` and `scripts/build_mingw64_*.sh`, which build the release binaries without CMake.
  6. A `multiarch_<isa>:INTERNAL=1` check in `.github/workflows/ci.yml`.
- Split overly complicated expressions, especially nested `std::min()`/`std::max()` calls combined with table lookups, into simpler intermediate calculations.
- Do not use `UINT64_C(...)` or similar integer-constant macros.
- Add command-line options as described at the top of `src/app/main.cpp`, and document new or changed options in `src/app/help.cpp`, in the man page source `doc/primesieve.txt` and, if listed there, in `README.md`. Regenerating `doc/primesieve.1` (`-DBUILD_MANPAGE=ON`) requires `a2x`; if it is unavailable, tell the user.

## Integer types and casts

primesieve supports numbers < 2^64 and uses 64-bit integer arithmetic. Generally keep signedness consistent within a function: use signed or unsigned integers as appropriate and avoid unnecessary mixing of the two. Add a cast only when it is needed, e.g. to avoid a warning or to narrow a value; narrowing assignments keep their explicit cast.

Prefer suffixless integer literals and implicit conversions when they preserve behavior and compile without warnings on supported compilers, including in typed initializers and hexadecimal mask arguments. Retain casts or suffixes needed for overload/template selection, signedness, or intermediate arithmetic width, e.g. `1ull << 32` (not `uint64_t(1) << 32`). A wider destination does not widen earlier operations.

Keep `~0ull` for the unsigned 64-bit all-ones value (`0xffffffffffffffff`); `~0` computes a signed `int` complement. Also keep unsigned suffixes on decimal constants above the signed 64-bit maximum and `L` in `__cplusplus` version checks. Preserve numeric and comment alignment in tables after removing suffixes.

When a cast is necessary, follow the convention used in the surrounding function or file. Do not introduce named C++ casts such as `static_cast<uint64_t>(x)`, `reinterpret_cast`, or similar forms. primesieve uses both C-style casts such as `(uint64_t) x`, which are more than three times as common, and function-style casts such as `uint64_t(x)`. Function-style casts are C++-only, so in C files use C-style casts. Preserve nearby pointer-cast style as well, for example `(uint8_t*) sieve.data()`.

## Internal utilities and the C++ standard library

Prefer primesieve's internal utilities over C++ standard library equivalents. Check the internal headers in `include/primesieve/` for existing functionality before using the standard library. Generally limit standard library use to low-level operations for which primesieve has no equivalent; threads, atomics and printing with `std::cout` are examples of allowed uses. Use primesieve's `Vector` and `Array` from `Vector.hpp` instead of `std::vector` and `std::array`; `std::vector` is only used with the public API, e.g. in the tests and examples.

- `pmath.hpp` provides `inBetween(lower, value, upper)`, `ceilDiv()`, `isPow2()`, `floorPow2()`, `ilog2()`, `isqrt()`, `checkedAdd()`, `checkedSub()`, `maxPrimeGap()` and `primeCountUpper()`. Use `inBetween()` instead of `std::clamp()`; note that the lower bound is the first argument and the value being clamped is the second.
- In scalar code, use `popcnt64()` from `popcnt.hpp` and `ctz64()` from `ctz.hpp`. x86 SIMD kernels whose target includes `popcnt` use `_mm_popcnt_u64()`.
- `macros.hpp` provides `ALWAYS_INLINE`, `NOINLINE`, `if_likely()`, `if_unlikely()`, `ASSERT()` (only active with `ENABLE_ASSERT`, which CMake defines for Debug builds), `FALLTHROUGH`, `UNREACHABLE`, `MAYBE_UNUSED`, `INDETERMINATE` (for large stack variables initialized later), `NO_UNROLL_LOOP` and `NO_VECTORIZE_LOOP`.
- There are no internal `min()`/`max()` helpers. Use `std::min()` and `std::max()` with an explicit cast when the argument types differ, e.g. `std::min(bytesToCopy, uint64_t(preSieveTables[0].size() - pos0))` in `PreSieve.cpp`.
- Report errors by throwing `primesieve_error`. The C API (`src/api-c.cpp`, `src/iterator-c.cpp`) must not let exceptions escape: it catches them, prints the message to `std::cerr`, sets `errno` to `EDOM` and, depending on the function, returns `PRIMESIEVE_ERROR` or a null pointer.

## Code comments

- Comment sparsely. Add a comment without being asked only when it helps a human reader understand an important, non-obvious aspect of the code, such as an algorithmic choice or a correctness constraint. Avoid comments that merely restate the code.
- When modifying code with an existing comment, update the comment if the code changes make it incorrect.
- When the user asks to copy a code section, copy its comments along with the code.
- When writing a multi-line comment, check the width of nearby comments and use a similar wrapping width.
- Do not end single-line comments with a period, but do end multi-line comments with a period.
- Keep comments short and compact. There is no fixed line count; use nearby comments as a guide to the expected length, especially when there are many examples nearby.
- Comments documenting functions and classes use `///`, comments inside functions use `//`. Multi-line `///` comments above functions usually end with a bare `///` line.
- All `*.cpp`, `*.c`, `*.hpp`, and `*.h` files outside `examples/` contain a top-level comment describing the file and providing license and copyright information (`/// @file` headers, `/** ... */` in the public C headers), usually with a single-year `Copyright (C) <year> Kim Walisch, <kim.walisch@gmail.com>` line. Whenever updating one of these files, update the copyright year in that comment to the current year. If the copyright uses a year range, preserve the starting year and update the ending year. Keep the copyright lines of other contributors (e.g. in `PreSieve.cpp`) unchanged.
- When creating a new `*.cpp`, `*.c`, `*.hpp`, or `*.h` file, add the standard top-level file description, license, and copyright comment using existing project files as a template and the current copyright year. This standard header is required regardless of the preference for sparse comments.

## Refactoring

After a significant code change, perform a dedicated refactoring and cleanup pass on the newly added or modified code before considering the task complete. A change is considered significant if the total number of newly added and modified existing lines is at least 30.

During this pass, inspect a few closely related source files, up to a maximum of 5, and analyze their coding conventions. Prefer the most relevant nearby or analogous implementations. Use those files to match the project's existing coding style, formatting, naming, comments, and code structure as closely as possible.

Do not refactor code outside the newly added or modified code unless it is necessary for the requested change. Do not refactor merely to satisfy this requirement. If the implementation is already simple and consistent with the surrounding code, leave it unchanged. Avoid unrelated cleanup outside the scope of the task.

When working on performance-critical code, such as the `crossOff*()` functions in `EratSmall.cpp` (manually unrolled), `EratMedium.cpp` and `EratBig.cpp`, the `PreSieve*` and `PrimeGenerator*` kernels, `CountPrintPrimes.cpp` and `popcount.cpp`, avoid refactoring that could deteriorate performance merely to reduce code duplication or the number of lines of code. In hot inner loops, prefer keeping performance-critical code inline rather than extracting it into functions or abstractions that the compiler might fail to inline. Performance takes precedence over reducing code size or duplication in such cases. Hot loops access arrays through raw pointers from `.data()` rather than `Vector` element access, and copy member variables they update into local variables. Keep lookup tables at namespace scope instead of in local `const` arrays, which add stack-protection instructions with GCC's `-fstack-protector-strong`. Measure performance claims with the benchmarks below.

## Benchmarking

Judge whether a change improves performance using these three single-threaded benchmarks. Run them from the build directory of a Release build (primesieve's default build type) configured with `-DBUILD_EXAMPLES=ON`, e.g. by running `cmake . -DBUILD_EXAMPLES=ON` in an existing build directory:

1. `./primesieve 1e11 -t1 -q --time` counts the primes ≤ 10^11 using one thread. It prints `Seconds: ...` and the count, which must be `4118054813`.
2. `./examples/cpp/primesieve_iterator` iterates forward over the primes ≤ 10^10 using `primesieve::iterator::next_prime()`.
3. `./examples/cpp/prev_prime` iterates backward over the primes ≤ 10^10 using `primesieve::iterator::prev_prime()`.

The two example programs benchmark `primesieve::iterator`, the most important feature of libprimesieve. Each prints `Sum of primes <= 10000000000: 2220822432581729238` and `Seconds: ...`. Run them without arguments; their optional limit argument is parsed with `std::atoll`, so it must be a plain integer such as `10000000000` (`1e10` would be read as 1).

To compare a change, build the unmodified code with the same configuration without touching the user's uncommitted changes, e.g. in a `git worktree` of the base commit. Run each benchmark several times, alternating between the two builds, and compare the minimum times; differences of about 1% are noise. No other CPU-intensive programs, including leftover benchmark runs, may run at the same time.

## ChangeLog

Whenever making a notable change in a file, add a short description, ideally one line, to the root `ChangeLog` under the next primesieve version. That is the topmost version in `ChangeLog` unless a `vX.Y` git tag exists for it; in that case, ask the user. Follow the existing format: version headings read `Changes in version X.Y, DD/MM/YYYY`, underlined with `=` characters of the same length, and entries read `* file.cpp: Description.`, usually added at the top of the list. Update the date in that version's heading to the current date using `DD/MM/YYYY` format (`11/10/2026` is 11 October 2026). Never change version numbers; the user updates them using `scripts/update_version.sh` (see `doc/RELEASE.md`).

The user may revise or remove ChangeLog entries, including entries they consider insufficiently notable. Respect these editorial decisions: preserve their revisions and do not restore removed entries unless explicitly asked.

## Building on Windows

Use MSYS2 MinGW-w64 GCC as the primary Windows compiler. The commands below use the MINGW64 environment in MSYS2's default location, `C:\msys64\mingw64\bin`. If MSYS2 is installed elsewhere, or you use another environment such as UCRT64 (used by CI), replace that path in all commands; the environment must provide `gcc`, `g++`, `cmake` and `mingw32-make`. Do not mix MSYS2 environments, and do not install or update packages.

Prepend that directory to `PATH` in every new shell used to configure, build, run tests, or run benchmarks. This makes both the build tools and their runtime DLLs available; without it, `g++` can fail with exit code 1 and no error message. Set the compiler and make program explicitly as shown below to avoid tool discovery delays.

The following PowerShell commands were verified on the maintainer's PC using the `Unix Makefiles` generator from `doc/BUILD.md`, CMake 4.4.3, and GCC 16.2.0. Run each block in a new shell from the repository root. `build-agents` is an ignored build directory; reuse it when its compiler and generator match, or choose a fresh build directory when they differ.

```powershell
$env:Path = 'C:\msys64\mingw64\bin;' + $env:Path
New-Item -ItemType Directory -Force -Path build-agents | Out-Null
Set-Location build-agents
cmake .. -G 'Unix Makefiles' -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe '-DCMAKE_CXX_FLAGS=-Wall -Wextra -pedantic -Werror' '-DCMAKE_C_FLAGS=-Wall -Wextra -pedantic -Werror' -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }
foreach ($probe in 'multiarch_x86_popcnt', 'multiarch_avx2', 'multiarch_avx512_bw', 'multiarch_avx512_vbmi2') { if (-not (Select-String -Path CMakeCache.txt -Pattern "^${probe}:INTERNAL=1$" -Quiet)) { throw "CMake probe $probe failed" } }
cmake --build . --parallel 14
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
ctest
if ($LASTEXITCODE -ne 0) { throw 'Unit tests failed' }
.\primesieve.exe --test
if ($LASTEXITCODE -ne 0) { throw 'primesieve --test failed' }
$count = .\primesieve.exe 1e10 -q
if ($LASTEXITCODE -ne 0 -or $count -ne '455052511') { throw "Wrong prime count: $count" }
$nth = .\primesieve.exe 1e9 --nth-prime -q
if ($LASTEXITCODE -ne 0 -or $nth -ne '22801763489') { throw "Wrong nth prime: $nth" }
"Known answers OK: primesieve 1e10 = $count, primesieve 1e9 --nth-prime = $nth"
```

The build defaults to `Release`. The parallel build above uses the maintainer's 14 logical processors; adjust the job count on other machines. For the Debug pass (Required verification, step 6), run the same commands with `build-agents-debug` as the build directory, `-DCMAKE_BUILD_TYPE=Debug` added, and `-O1` prepended to both sets of flags.

This configuration uses GNU's linker, with `CMAKE_LINKER` resolving to `C:/msys64/mingw64/bin/ld.exe`. Microsoft Visual Studio's `link.exe` is not required. Do not add a Visual Studio linker path or force `link.exe` for this MinGW build.

`.\primesieve.exe --cpu-info` shows which of the instruction sets used for runtime dispatch (AVX2, AVX-512, ARM SVE) the CPU supports. Kernels for instruction sets that the CPU lacks, e.g. AVX-512 on many x64 CPUs, can only be compiled, and x64 compilers skip the ARM code, which CI's ARM64 Linux jobs run. Report which code paths you could not run.

After changes to MSVC-specific code (`_MSC_VER` branches, `PRIMESIEVE_TARGET_ATTRIBUTE`, `popcnt.hpp`, runtime dispatch), also build with MSVC and with clang-cl, which defines both `_MSC_VER` and `__clang__`. The block below was verified with Visual Studio Community 2026 (MSVC 14.51) and LLVM 22.1.5 in their default locations. Importing the developer shell module, unlike running `Launch-VsDevShell.ps1`, works with PowerShell's default execution policy; a `'vswhere.exe' is not recognized` message is harmless. With MSVC-style compilers, CMake's Ninja generator defaults to a Debug build, hence the explicit `CMAKE_BUILD_TYPE`. MSVC's helper processes `vctip.exe` and `mspdbsrv.exe` may keep running for several minutes after the build; tools that wait for all child processes wait for them as well.

```powershell
Import-Module 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll'
Enter-VsDevShell -VsInstallPath 'C:\Program Files\Microsoft Visual Studio\18\Community' -SkipAutomaticLocation -DevCmdArguments '-arch=amd64 -host_arch=amd64'
$env:Path = 'C:\Program Files\LLVM\bin;' + $env:Path
foreach ($compiler in 'cl', 'clang-cl') {
  New-Item -ItemType Directory -Force -Path "build-agents-$compiler" | Out-Null
  Push-Location "build-agents-$compiler"
  cmake .. -G Ninja "-DCMAKE_CXX_COMPILER=$compiler" "-DCMAKE_C_COMPILER=$compiler" -DCMAKE_BUILD_TYPE=Release '-DCMAKE_CXX_FLAGS=/W3 /WX /EHsc' '-DCMAKE_C_FLAGS=/W3 /WX' -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
  if ($LASTEXITCODE -ne 0) { throw "${compiler}: CMake configuration failed" }
  cmake --build . --parallel 14
  if ($LASTEXITCODE -ne 0) { throw "${compiler}: build failed" }
  ctest
  if ($LASTEXITCODE -ne 0) { throw "${compiler}: unit tests failed" }
  .\primesieve.exe --test
  if ($LASTEXITCODE -ne 0) { throw "${compiler}: primesieve --test failed" }
  Pop-Location
}
```

To run the benchmarks on Windows, prepend `C:\msys64\mingw64\bin` to `PATH` and run the commands from the Benchmarking section unchanged in the `build-agents` directory.

## Required verification

Always build and run all of the following checks after code changes, using a separate build directory:

1. Configure with tests and examples enabled and warnings treated as errors for both C++ and C:

   ```sh
   cmake .. -DCMAKE_CXX_FLAGS="-Wall -Wextra -pedantic -Werror" -DCMAKE_C_FLAGS="-Wall -Wextra -pedantic -Werror" -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
   ```

   On Windows, use the explicit tool paths and generator in the PowerShell recipe above.

2. Build using `cmake --build . --parallel`.
3. Run `ctest` from the build directory. If a test fails, inspect its output using `ctest --output-on-failure`.
4. Run `./primesieve --test` to run primesieve's correctness tests.
5. Verify that `./primesieve 1e10 -q` prints exactly `455052511` and `./primesieve 1e9 --nth-prime -q` prints exactly `22801763489`.
6. After changes to `src/` or `include/`, repeat these checks in a Debug build in another build directory, adding `-DCMAKE_BUILD_TYPE=Debug` and `-O1` to both sets of flags as in `test/README.md`. Debug builds enable `ASSERT()`, including bounds checks in `Vector` and `Array`.
7. After changes to CMake files or runtime dispatch code, verify that `CMakeCache.txt` contains `multiarch_<isa>:INTERNAL=1` for `x86_popcnt`, `avx2`, `avx512_bw` and `avx512_vbmi2` on x64, or for `arm_sve` on ARM64 Linux with GCC 14 or later or Clang, as CI does.
8. After changes to printing code, compare the MD5 sums of the `--print` output with those in `.github/workflows/ci.yml`. On Windows, convert the output's CRLF line endings to LF before hashing.
9. After changes to MSVC-specific code, also run the MSVC and clang-cl block above.

In PowerShell, use `.\primesieve.exe` as shown above. Check every command's exit status and report any failures or checks that could not be completed.

See `doc/BUILD.md` for general build instructions and `test/README.md` for additional testing options.

## Windows Python process cleanup

- Do not use the Microsoft Store `python.exe` launcher for long-running or potentially cancelled Python commands. Resolve and invoke the real Python interpreter executable directly.
- Record the process IDs created by every Python command. If the command is cancelled, times out, or is otherwise terminated early, stop its entire process tree immediately and verify that it is gone.
- Never finish a turn after invoking Python without checking for Python processes created during that turn and stopping any that are still running.
- Do not terminate a running Python tool cell without performing the process-tree cleanup and verification in the same turn.
