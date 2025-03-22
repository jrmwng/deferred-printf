# Deferred Printf

## Overview
The Deferred Printf project implements a deferred output mechanism in C++. This allows for the accumulation of output data that can be printed later, providing flexibility in how and when output is displayed.

## Project Structure
```
deferred-printf
├── src
│   └── deferred_printf.cpp
├── include
│   └── deferred_printf.h
├── CMakeLists.txt
└── README.md
```

## Files Description

- **src/deferred_printf.cpp**: Implements the functionality of the deferred printf feature. It includes the logic for handling deferred output and defines helper functions.

- **include/deferred_printf.h**: Declares the interface for the deferred printf functionality. It exports functions and possibly classes related to deferred printing.

- **CMakeLists.txt**: Configuration file for CMake. It specifies the project name, version, and the source files to be compiled.

## Setup Instructions
1. Clone the repository:
   ```
   git clone <repository-url>
   ```

2. Navigate to the project directory:
   ```
   cd deferred-printf
   ```

3. Create a build directory and navigate into it:
   ```
   mkdir build
   cd build
   ```

4. Run CMake to configure the project:
   ```
   cmake ..
   ```

5. Build the project:
   ```
   make
   ```

## Usage
After building the project, you can use the deferred printf functionality by including the `deferred_printf.h` header in your source files and using the provided functions.

Example with `vprintf`:
```cpp
#include "deferred_printf.h"

int main() {
    jrmwng::deferred_printf<> dp;
    dp("Hello, ");
    dp("world!");
    dp.apply(&vprintf);
    return 0;
}
```

Example with `vfprintf`:
```cpp
#include "deferred_printf.h"
#include <cstdio>

int main() {
    jrmwng::deferred_printf<> dp;
    dp("Hello, ");
    dp("world!");
    FILE *file = fopen("output.txt", "w");
    if (file) {
        dp.apply(&vfprintf, file);
        fclose(file);
    }
    return 0;
}
```

Example with `vsprintf`:
```cpp
#include "deferred_printf.h"
#include <cstdio>

int main() {
    jrmwng::deferred_printf<> dp;
    dp("Hello, ");
    dp("world!");
    char buffer[100];
    dp.apply(&vsprintf, buffer);
    printf("%s\n", buffer);
    return 0;
}
```

Example with custom buffer size (e.g., 1MB):
```cpp
#include "deferred_printf.h"

int main() {
    jrmwng::deferred_printf<1024 * 1024> dp;
    dp("Hello, ");
    dp("world!");

    // Apply the deferred printf with a custom buffer size
    dp.apply(&vprintf);
    return 0;
}
```

Test case to obtain the required buffer size, allocate a buffer with the size, and then fill the buffer from the logger:
```cpp
#include "deferred_printf.h"
#include <vector>

int main() {
    jrmwng::deferred_printf<> dp;
    dp("Hello, ");
    dp("world!");

    // Obtain the required buffer size
    int size = dp.apply<char*, size_t>(&vsnprintf, nullptr, 0);

    // Allocate a buffer with the required size
    std::vector<char> buffer(size + 1);

    // Fill the buffer from the logger
    dp.apply(&vsprintf, buffer.data());

    printf("%s\n", buffer.data());
    return 0;
}
```

## Running Tests
To run the tests, use CTest after building the project:

```sh
cd build
ctest
```

## Contributing
Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.