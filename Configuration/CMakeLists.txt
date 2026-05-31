cmake_minimum_required(VERSION 3.8)

project(Configuration)

set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/source/ConfigurationFile.cpp
    ${PROJECT_SOURCE_DIR}/source/JSONConfigurationImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/XMLConfigurationImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/YAMLConfigurationImpl.cpp
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(Configuration ${SOURCES})

target_include_directories(Configuration PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/nlohmann_json/include
    ${PROJECT_SOURCE_DIR}/third_party/pugixml/include
)