cmake_minimum_required(VERSION 3.5.1)

project(gtest_comm VERSION "0.0.0.1")
set(DEBUG_MODE 1)
CONFIGURE_FILE(version.h.in ${PROJECT_BINARY_DIR}/version.h)

set(COMM_ROOT "${PROJECT_SOURCE_DIR}/../..")

if( ${DEBUG_MODE} EQUAL 1 )
    message(">>>> Debug Mode setting <<<<")
    set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
    # add_definitions(-DLOG_FILE_PATH="/dlt/debug.txt")
    # add_definitions(-DLOG_HLOG_TYPE)
else()
    message(">>>> Release Mode setting <<<<")
    set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    # add_definitions(-DLOG_FILE_PATH="/dlt/release.txt")
endif()

message(">>>> PROJECT_NAME = ${PROJECT_NAME} <<<<")
message(">>>> OUTPUT_DIR = ${OUTPUT_DIR} <<<<")
set(INSTALL_BIN_DIR "${OUTPUT_DIR}/bin")
set(INSTALL_LIB_DIR "${OUTPUT_DIR}/lib")
set(INSTALL_INC_DIR "${OUTPUT_DIR}/include")
set(INSTALL_PKGCONFIG_DIR "${OUTPUT_DIR}/share/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${COMM_ROOT}/debug/lib/pkgconfig")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

find_package(PkgConfig REQUIRED)
pkg_check_modules(COMM communicator REQUIRED)
pkg_check_modules(PROTO proto REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

link_directories(
    ${COMM_LIBRARY_DIRS}
    ${PROTO_LIBRARY_DIRS}
)

include_directories(
    inc
    ${PROJECT_BINARY_DIR}
    ${COMM_INCLUDE_DIRS}
    ${PROTO_INCLUDE_DIRS}
)

set(LIBS
    pthread
    ${COMM_LIBRARIES}
    ${PROTO_LIBRARIES}
    gtest
)

file(GLOB GTEST_SRCS
    "./src/*.cpp"
)

#make executable for gtest binary
add_executable(${PROJECT_NAME} ${GTEST_SRCS} )

target_link_libraries(${PROJECT_NAME}
	${LIBS}
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

