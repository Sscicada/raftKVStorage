# 添加 raft 目录
add_subdirectory(raft)

# 创建可执行文件并链接 raftcore 库
add_executable(raftTest main.cpp)

target_link_libraries(raftTest PRIVATE raftcore)