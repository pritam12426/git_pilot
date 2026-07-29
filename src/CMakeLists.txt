# Collect all source files
set(SOURCES
    main.cpp
    server.cpp
    session.cpp
    transport/transport.cpp
    transport/stdio_transport.cpp
    protocol/mcp_handler.cpp
    protocol/json_rpc.cpp
    protocol/types.cpp
    tools/tool_registry.cpp
    tools/git_clone.cpp
    tools/git_status.cpp
    tools/git_commit.cpp
    tools/git_log.cpp
    tools/git_diff.cpp
    utils/logger.cpp
    utils/json_helpers.cpp
    utils/error_handling.cpp
)

# Conditionally add HTTP transport source
if(ENABLE_HTTP)
    list(APPEND SOURCES transport/http_transport.cpp)
endif()

# Create the main library (object library — linked directly into the executable)
add_library(git_pilot_core OBJECT ${SOURCES})

# Transport compile definitions
if(ENABLE_HTTP)
    target_compile_definitions(git_pilot_core PRIVATE ENABLE_HTTP=1)
endif()

if(ENABLE_STDIO)
    target_compile_definitions(git_pilot_core PRIVATE ENABLE_STDIO=1)
endif()

# Include directories
target_include_directories(git_pilot_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/transport
        ${CMAKE_CURRENT_SOURCE_DIR}/protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/tools
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${PROJECT_BINARY_DIR}/include
)

# Link libraries
target_link_libraries(git_pilot_core
    PUBLIC
        Boost::system
        Boost::json
        Boost::program_options
        libgit2::libgit2package
        Threads::Threads
    PRIVATE
        ${CMAKE_DL_LIBS}
)

# Compiler options
target_compile_options(git_pilot_core PRIVATE ${DEFAULT_COMPILE_OPTIONS})

# Link options
target_link_options(git_pilot_core PRIVATE ${DEFAULT_LINK_OPTIONS})

# Create the executable
add_executable(git_pilot_server $<TARGET_OBJECTS:git_pilot_core>)
target_link_libraries(git_pilot_server PRIVATE git_pilot_core)
target_link_options(git_pilot_server PRIVATE ${DEFAULT_LINK_OPTIONS})
