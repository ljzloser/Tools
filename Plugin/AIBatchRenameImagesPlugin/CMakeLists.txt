cmake_minimum_required(VERSION 3.16)

project(AIBatchRenameImagesPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Sql Xml Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Sql Xml Concurrent)
set(Lib_DIR ${PROJECT_SOURCE_DIR}/../../lib)

include_directories(${PROJECT_SOURCE_DIR}/include
    ${Lib_DIR}/QHotkey/QHotkey
    ${Lib_DIR}/LClass/LCore/include
    ${Lib_DIR}/LClass/LWidget/include
    ${Lib_DIR}/spdlog/include/spdlog
    ${Lib_DIR}/Utility/include
)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)
FILE(GLOB_RECURSE RC_FILES ${PROJECT_SOURCE_DIR}/res/*.rc) 
FILE(GLOB_RECURSE QRC_FILES ${PROJECT_SOURCE_DIR}/res/*.qrc)
FILE(GLOB_RECURSE UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)
FILE(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h)
# FILE(GLOB_RECURSE INCLUDE_FILES ${PROJECT_SOURCE_DIR}/../include/*.h)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_FILES)

add_library(AIBatchRenameImagesPlugin SHARED
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${UI_FILES}
  ${RC_FILES}
  ${QRC_FILES}
  # ${INCLUDE_FILES}
)

target_link_libraries(AIBatchRenameImagesPlugin PRIVATE Qt${QT_VERSION_MAJOR}::Core
  LWidget
  LCore
  spdlog
  Utility
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Sql
  Qt${QT_VERSION_MAJOR}::Xml
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Concurrent
)

target_compile_definitions(AIBatchRenameImagesPlugin PRIVATE AIBatchRenameImagesPlugin_LIBRARY)
