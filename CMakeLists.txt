cmake_minimum_required(VERSION 4.4.0)

project(
    GitPilot
    VERSION 1.0.0
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_HTTP "Enable HTTP transport" ON)
option(ENABLE_STDIO "Enable stdio transport" ON)

# Logger options
option(LOG_SHOW_TIME_STAMP "Show timestamps in logs" ON)
option(LOG_SHOW_SOURCE_LOCATION "Show source location in logs" OFF)
option(LOG_USE_FMT "Use fmt library instead of std::format" OFF)

# Include helper modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerOptions)

# Find dependencies
find_package(Boost REQUIRED COMPONENTS
    system
    json
    program_options
)

find_package(libgit2 REQUIRED)
find_package(Threads REQUIRED)

# Subdirectories
add_subdirectory(src)
add_subdirectory(config)

# Logger compile definitions (after git_pilot_core is created in src/)

if(LOG_SHOW_TIME_STAMP)
    target_compile_definitions(git_pilot_core PUBLIC LOG_SHOW_TIME_STAMP)
endif()

if(LOG_SHOW_SOURCE_LOCATION)
    target_compile_definitions(git_pilot_core PUBLIC LOG_SHOW_SOURCE_LOCATION)
endif()

if(LOG_USE_FMT)
    find_package(fmt REQUIRED)
    target_link_libraries(git_pilot_core PRIVATE fmt::fmt)
    target_compile_definitions(git_pilot_core PUBLIC LOG_USE_FMT)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install targets
install(TARGETS git_pilot_server RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include/git_pilot)
