# KDB-Tree

This is an implementation of the KDB-Tree data structure in C++.

[[_TOC_]]

## Building

The project includes a `meson.build` file for building the project using [Meson](https://mesonbuild.com). Experienced users may modify this file or use it as reference for alternate build profiles. However, the following requirements and instructions are only intended for the original build profile.

### Requirements

1. [Meson](https://mesonbuild.com/Getting-meson.html)
2. [Ninja](https://ninja-build.org/)

### Instructions

Execute the following commands to setup a `./build` directory and compile the program.
```
meson setup build
meson compile -C build
```
The generated `./build` directory contains an `Index` binary for executing the program.

For easy of use, link the binary to the root directory.
```
ln -s build/Index .
```

Optional - Users may link the `compile_commands.json` file to the root as well for linting purposes.
```
ln -s build/compile_commands.json .
```

## Configuration

The project configuration lies in `include/config.h`. The options are explained in the comments.
```cpp
constexpr bool BULKLOAD = true; // Set it to true if bulk loading the data set, and false otherwise.
constexpr bool EVAL = true; // Set it to true for execute the operation file.
constexpr bool DEBUG = false; // Set it to true for debugging query results.
constexpr bool SNAPSHOT = true; // Set it to true for creating a snapshot of the index.

constexpr unsigned int FANOUT = 204; // Set the fanout here.
constexpr unsigned int PAGECAP = 204; // Set the page capacity here.
```

## Usage

### When bulk loading
Execute the program with two command line arguments, first is the path to the *data file* and second is the path to the *operation file*.
```
./Index <Path to the data file> <Path to the operation file>
```
### When NOT bulk loading,
Execute the program with a single command line argument, i.e. the path to the *operation file*.
```
./Index <Path to the operation file>
```

## File format

### Data file format

The data file is used for bulk loading the index and must contain a record on each line. A record consists of an integer for the *id*, and two floats for the *point*.
```
<id> <x-axis coordinate> <y-axis coordinate>
```
**Note**: Users may need to modify the program to supply any metadata in the subsequent fields to satisfy their usecase. The original program only supports the simplest case of spatial indexing.

### Operation file format

The operation file must contain an operation description on each line. Each operation entry begins with a character to distinguish the operation followed by operation specific description.
The format of all three is detailed below.
- Insertion (i): Requires a point (two floats) and an id (integer).
```
i <x-axis coordinate> <y-axis coordinate> <id>
```
- k-NN query (k): Requires a point (two floats) and the value of k (integer).
```
k <x-axis coordinate> <y-axis coordinate> <k>
```
- Range query (r): Requires an MBR (four floats) and size (float; size is only used for logging purposes when evaluating the index, and denotes the ratio of each side of the MBR and the data set extent).
```
r <lower x-axis coordinate> <lower y-axis coordinate> <higher x-axis coordinate> <higher y-axis coordinate> <size>
```
- Log (l): Requires no additional information. Used to initiate the performance measurements.
```
l
```
- Reset (z): Requires no additional information. Used to reset the performance measurements.
```
z
```

## Miscellaneous

### Logging

After each execution, the program outputs a `log.txt` file detailing all statistics of the previous run.

### Snapshot

When `SNAPSHOT` is enabled in the config file, the program outputs `Index.csv`. Run the python script `drawIndex.py` to create the snapshot `Index.png` to get an overview of index partitions.
```
python drawIndex.py
```
The script may require the [Plotly](https://plotly.com/python/getting-started/) library to be installed.

### Examples

The following example scripts setup the build directory, compile the program and execute the code for static and dynamic data sets respectively.
- Execute `staticExample.sh` to bulk load `sampleData.txt` to the index and operate `sampleOperations.txt` on it. Make sure to enable `BULKLOAD` in the config file before executing this script.
- Execute `dynamicExample.sh` to operate `sampleOperations.txt` on the index. Make sure to disable `BULKLOAD` in the config file before executing this script.

## Remarks

The program runs fine for benchmarking purposes, but may undergo refactoring to improve overall readability and code aesthetics. Users are advised to clone the master branch for the best version of the program.