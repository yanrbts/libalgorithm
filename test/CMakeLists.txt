
# 查找所有的源文件
file(GLOB_RECURSE SOURCE_FILES "Unity/*.c" "main.c")

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/src)

add_executable(unitytest ${SOURCE_FILES})
target_link_libraries(unitytest algorithm_shared)