# HamArc

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/build-CMake-brightgreen.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A fault-tolerant, error-correcting file archiver implemented in modern C++ (C++23). **HamArc** packages multiple files into a single archive (`.haf` — *Hamming Archive File*) and employs **Hamming error-correction coding** to detect and repair data corruption caused by bit flips or transmission noise.

---

## Features

- **Hamming Error Correction**: Uses Hamming encoding over configurable block sizes to protect stored file contents and archive headers.
- **Archive Operations**:
  - **Create** (`-c`, `--create`): Pack one or more files into a new `.haf` archive.
  - **Extract** (`-x`, `--extract`): Unpack all files or select individual files from the archive.
  - **List** (`-l`, `--list`): Display contents and metadata of an archive.
  - **Append** (`-a`, `--append`): Append new files to an existing archive.
  - **Delete** (`-d`, `--delete`): Remove specific files from an archive.
  - **Concatenate** (`-A`, `--concatenate`): Merge multiple `.haf` archives into a single archive.
- **Configurable Hamming Block Size**: Supports custom block sizes via `--size` (default: 8 bits).
- **Custom Argument Parser**: Lightweight and robust CLI option parser built from scratch without external dependencies.
- **Unit & Integration Tests**: Comprehensive tests using GoogleTest.

---

## Architecture Overview

```
HamArc
├── app/
│   ├── archiver.h / archiver.cpp   # Core Archiver, Coder, and Decoder implementations
├── bin/
│   └── main.cpp                    # CLI entry point
├── lib/
│   ├── argparser.h / .cpp          # Command-line argument parser
│   └── type.h / .cpp               # CLI option types and validation
└── tests/
    └── test_archiver.cpp           # GoogleTest integration tests
```

- **Coder**: Streams input files through Hamming code bit-parity calculations and serializes header metadata (file names, sizes).
- **Decoder**: Reads Hamming-encoded blocks, verifies parity bits, corrects single-bit errors, unpacks file contents, and validates checksums.

---

## Build Instructions

### Prerequisites
- Modern C++ compiler with C++23 support (`gcc >= 13` or `clang >= 17`)
- CMake `>= 3.20`
- Git (for fetching GoogleTest dependency automatically)

### Building

```bash
# Clone the repository
git clone https://github.com/NikitaKkv/HamArc.git
cd HamArc

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

The compiled binary `hamarc` will be located in `build/bin/` (or root `build/`).

### Running Tests

```bash
ctest --test-dir build --output-on-failure
```

---

## CLI Usage

```bash
# Create an archive with multiple files
./hamarc -c -f backup.haf file1.txt file2.png document.pdf

# List files inside an archive
./hamarc -l -f backup.haf

# Extract all files from an archive
./hamarc -x -f backup.haf

# Extract specific files
./hamarc -x -f backup.haf file1.txt

# Append a file to an existing archive
./hamarc -a -f backup.haf extra.txt

# Delete a file from an archive
./hamarc -d -f backup.haf file2.png

# Concatenate two archives into a third
./hamarc -A archive1.haf archive2.haf -f merged.haf
```

---

## License

This project is licensed under the MIT License.
