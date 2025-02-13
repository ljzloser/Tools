cmake_minimum_required(VERSION 3.16)

project(DeepSeekPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Sql Xml )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Sql Xml )
set(Lib_DIR ${PROJECT_SOURCE_DIR}/../../lib)

include_directories(${PROJECT_SOURCE_DIR}/include
    ${Lib_DIR}/QHotkey/QHotkey
    ${Lib_DIR}/LClass/LCore/include
    ${Lib_DIR}/LClass/LWidget/include
    ${Lib_DIR}/spdlog/include/spdlog
    ${Lib_DIR}/Utility/include
    ${Lib_DIR}/DeepSeek/include
    # ${Lib_DIR}/cmake/src
)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)
FILE(GLOB_RECURSE RC_FILES ${PROJECT_SOURCE_DIR}/res/*.rc) 
FILE(GLOB_RECURSE QRC_FILES ${PROJECT_SOURCE_DIR}/res/*.qrc)
FILE(GLOB_RECURSE UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)
FILE(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h)
# FILE(GLOB_RECURSE INCLUDE_FILES ${PROJECT_SOURCE_DIR}/../include/*.h)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_FILES)

add_library(DeepSeekPlugin SHARED
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${UI_FILES}
  ${RC_FILES}
  ${QRC_FILES}
  # ${INCLUDE_FILES}
)

target_link_libraries(DeepSeekPlugin PRIVATE Qt${QT_VERSION_MAJOR}::Core

  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Sql
  Qt${QT_VERSION_MAJOR}::Xml
  # Qt${QT_VERSION_MAJOR}::WebEngineWidgets
  Qt${QT_VERSION_MAJOR}::Widgets
  LWidget
  LCore
  spdlog
  Utility
  DeepSeek
  # cmark
)

target_compile_definitions(DeepSeekPlugin PRIVATE DeepSeekPlugin_LIBRARY)
