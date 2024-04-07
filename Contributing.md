# Contributing

When submitting a pull request, please run the formatter and test suite locally to ensure that the changes pass all checks.

### Build and run the main target in debug mode

Use the following command to build and run the executable target.

```
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
```

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```
cmake -B build
cmake --build build
cmake --build build --target test
```

To collect code coverage information, run CMake with the `-DENABLE_TEST_COVERAGE=1` option.

### Run the formatter

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, and _cmake-format_ to be installed on the current system.

```
cmake -B build

# view changes
cmake --build build --target format

# apply changes
cmake --build build --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.


#### Sanitizers

Sanitizers can be enabled by configuring CMake with `-DUSE_SANITIZER=<Address | Memory | MemoryWithOrigins | Undefined | Thread | Leak | 'Address;Undefined'>`.

#### Ccache

Ccache can be enabled by configuring with `-DUSE_CCACHE=<ON | OFF>`.
