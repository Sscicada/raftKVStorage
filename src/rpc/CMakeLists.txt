# 获取当前源文件所在目录的绝对路径
get_filename_component(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

# 将该目录下所有源文件添加到 SRC_LIST 变量中
aux_source_directory(${SRC_DIR} SRC_LIST)

# 创建一个库目标（静态库或共享库）
add_library(rpc ${SRC_LIST} ${src_common} )

# 指定目标（如库或可执行文件）需要链接的外部依赖库
target_link_libraries(rpc boost_serialization)

# 设置缓存变量
set(src_rpc ${SRC_LIST} CACHE INTERNAL "Description of the variable")
