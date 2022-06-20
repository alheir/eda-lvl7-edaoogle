cmake_minimum_required(VERSION 3.0)

project(microhttpdtest)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)

# main
add_executable(edahttpd main.cpp CommandLineParser.cpp HttpServer.cpp ServeHttpRequestHandler.cpp EDAoogleHttpRequestHandler.cpp)

# libmicrohttps
find_path(MICROHTTPD_INCLUDE_PATHS NAMES microhttpd.h)
find_library(MICROHTTPD_LIBRARIES NAMES microhttpd libmicrohttpd libmicrohttpd-dll)
target_include_directories(edahttpd PRIVATE ${MICROHTTPD_INCLUDE_PATHS})
target_link_libraries(edahttpd PRIVATE ${MICROHTTPD_LIBRARIES})

# Windows: Copy libmicrohttpd.dll
find_file(MICROHTTPD_BINARIES NAMES ../bin/libmicrohttpd-dll.dll)
if(MICROHTTPD_BINARIES)
    set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE_INIT})
    if (NOT EXISTS ${OUTPUT_DIR})
        set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    file(COPY ${MICROHTTPD_BINARIES} DESTINATION ${OUTPUT_DIR} NO_SOURCE_PERMISSIONS)
endif()



enable_testing()
add_executable(edahttpd_test main_test.cpp HttpServer.cpp ServeHttpRequestHandler.cpp EDAoogleHttpRequestHandler.cpp)
add_test(NAME test1 COMMAND main_test)

target_include_directories(edahttpd_test PRIVATE ${MICROHTTPD_INCLUDE_PATHS})
target_link_libraries(edahttpd_test PRIVATE ${MICROHTTPD_LIBRARIES})
