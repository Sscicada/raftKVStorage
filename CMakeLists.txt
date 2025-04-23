# 设置cmake的最低版本和项目名称
cmake_minimum_required(VERSION 3.22)
project(KVRaftCpp)

set(CMAKE_CXX_STANDARD 20)

# 生成debug版本，可以进行gdb调试
set(CMAKE_BUILD_TYPE "Debug")

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf_FOUND=${Protobuf_FOUND}")  # 检查是否找到 Protobuf
message(STATUS "Protobuf_PROTOC_EXECUTABLE=${Protobuf_PROTOC_EXECUTABLE}")  # 检查 protoc 路径

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# set(PROTO_FILES
#     ${PROJECT_SOURCE_DIR}/proto/friend.proto
#     ${PROJECT_SOURCE_DIR}/proto/rpcheader.proto
# )

# message(STATUS "PROTO_FILES=${PROTO_FILES}")  # 打印路径确认

# 编译 proto 文件，生成相应的 C++ 头文件和源文件
# protobuf_generate_cpp(PROTO_SRC PROTO_HDR ${PROTO_FILES})

# message("成功编译 proto 文件")
# message(STATUS "PROTO_SRC=${PROTO_SRC}")  # 应显示生成的 .pb.cc 文件路径
# message(STATUS "PROTO_HDR=${PROTO_HDR}")  # 应显示生成的 .pb.h 文件路径

# 设置项目编译头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include/common)
# include_directories(${PROJECT_SOURCE_DIR}/include/fiber)
include_directories(${PROJECT_SOURCE_DIR}/include/rpc)
include_directories(${PROJECT_SOURCE_DIR}/example)
include_directories(${CMAKE_BINARY_DIR}/example/rpc)

# include_directories(${PROJECT_SOURCE_DIR}/include/raftCore)
# include_directories(${PROJECT_SOURCE_DIR}/include/raftRpcPro)
# include_directories(${PROJECT_SOURCE_DIR}/include/raftClerk)
# include_directories(${PROJECT_SOURCE_DIR}/include/skipList)

include_directories(${PROJECT_SOURCE_DIR}/third_party/muduo/include)
include_directories(${PROJECT_SOURCE_DIR}/third_party/protobuf/include)

# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src 包含所有核心模块代码
add_subdirectory(src)

# example 包含模块的示例代码
add_subdirectory(example)

# add_library(skip_list_on_raft STATIC  ${src_rpc} ${src_fiber} ${rpc_example} ${raftsource} ${src_raftCore} ${src_raftRpcPro})
# target_link_libraries(skip_list_on_raft muduo_net muduo_base pthread dl)
# # 添加格式化目标 start
# # from : https://blog.csdn.net/guotianqing/article/details/121661067

# add_custom_target(format
#         COMMAND bash ${PROJECT_SOURCE_DIR}/format.sh
#         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#         COMMAND echo "format done!"
# )
# # 添加格式化目标 end