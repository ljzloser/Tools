cmake_minimum_required(VERSION 3.16)
project(Tools)
# 定义输出路径
set(OUTPUT_PATH 
    $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/bin/Debug/>
    $<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/bin/Release/>
    $<$<CONFIG:MinSizeRel>:${PROJECT_SOURCE_DIR}/bin/MinSizeRel/>
    $<$<CONFIG:RelWithDebInfo>:${PROJECT_SOURCE_DIR}/bin/RelWithDebInfo/>
)
string(REPLACE ";" "" OUTPUT_PATH "${OUTPUT_PATH}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
link_directories(${OUTPUT_PATH})
message(STATUS "Output path: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
add_subdirectory(Tools)
add_subdirectory(lib)
add_subdirectory(Test)
add_subdirectory(Plugin)
add_subdirectory(ToolsServer)