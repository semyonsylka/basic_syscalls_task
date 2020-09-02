# Basic syscall sample


## Build

Create build directory and run cmake with path to CMakeLists.txt

```bash
mkdir build
cd build
cmake /path/to/syscall_basic_task/src
make
```

## Usage

Run server (poll_server or select_server) and client 
```bash
./pool_server
./client
```
For docker container run.sh script starts pool_server with client

## Requires
cmake, gcc
