# 查找所有的源文件
file(GLOB_RECURSE SOURCE_FILES "list/*.c" "tree/*.c" "zmalloc.c")

# 生成共享库 libalgorithm.so
add_library(algorithm_shared SHARED ${SOURCE_FILES})

# 生成静态库 libalgorithm.a
add_library(algorithm_static STATIC ${SOURCE_FILES})

# 设置库的版本号和 SOVERSION
set_target_properties(algorithm_shared PROPERTIES VERSION ${VERSION})
set_target_properties(algorithm_static PROPERTIES VERSION ${VERSION})

# 设置库的输出名称
set_target_properties(algorithm_shared PROPERTIES OUTPUT_NAME "algorithm")
set_target_properties(algorithm_static PROPERTIES OUTPUT_NAME "algorithm")

# 指定库的安装路径
install(TARGETS algorithm_shared algorithm_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)