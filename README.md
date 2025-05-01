# RaftKVStorage - A Raft-based Distributed Key-Value Store in C++

> **Notice**: This project is for learning the principles of Raft and implementing a simple key-value storage database. It is not intended for production environments.

## Project Status
Early Development

## Prerequisites

### Library Dependencies
- muduo
- boost
- protoc
- clang-format (optional)

### Installation Instructions

1. **clang-format** (code formatting tool):
   ```bash
   sudo apt-get install clang-format
   ```

2. **protoc** (Protocol Buffers compiler, tested with v3.12.4):
   ```bash
   sudo apt-get install protobuf-compiler libprotobuf-dev
   ```

3. **boost** (C++ libraries):
   ```bash
   sudo apt-get install libboost-dev libboost-test-dev libboost-all-dev
   ```

4. **muduo** (C++ network library):
   Follow installation guide at:  
   [https://blog.csdn.net/QIANGWEIYUAN/article/details/89023980](https://blog.csdn.net/qq_55882840/article/details/145346115?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522390848fb287d7d8cd2264d4e3c91f674%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=390848fb287d7d8cd2264d4e3c91f674&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~sobaiduend~default-1-145346115-null-null.nonecase&utm_term=muduo%E5%BA%93%E5%AE%89%E8%A3%85&spm=1018.2226.3001.4450)

## Build Instructions

```bash
mkdir build && cd build
cmake ..
make
```

## Use

```bash
./kvserver --nums 5 --cluster cluster.conf
```
or
```bash
# terminal_1 - node1
./kvserver --id 1 --cluster "1:localhost:50051,2:localhost:50052,3:localhost:50053"

# terminal_2 - node2
./kvserver --id 2 --cluster "1:localhost:50051,2:localhost:50052,3:localhost:50053"

# terminal_3 - node3
./kvserver --id 3 --cluster "1:localhost:50051,2:localhost:50052,3:localhost:50053"
```
raft cluster
```bash
----------------------------------------
|          Raft Cluster State          |
----------------------------------------
| [ Node 1 ] Role: Leader  Term: 7     |
| [ Node 2 ] Role: Follower Term: 7    |
| [ Node 3 ] Role: Follower Term: 7    |
|--------------------------------------|
| Leader ID: Node 1                    |
| Election Timeout: 150ms              |
| Heartbeat Interval: 50ms             |
|--------------------------------------|
| Log Progress                         |
| Node 1 [##########......] 80%        |
| Node 2 [#######.........] 50%        |
| Node 3 [#########.......] 60%        |
----------------------------------------
```

run raft clerk
```bash
./kvclient localhost:50051 localhost:50052 localhost:50053
> put name Alice
OK
> get name
Alice
```
