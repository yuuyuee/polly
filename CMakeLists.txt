# Copyright RCT Power 2025
# Author: ivan.yu (ivan.yu@rct-power.com.cn)

cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME rctems)
set(SIMULATOR_NAME simulator)
set(RCTEMS_VERSION_MAJOR 0)
set(RCTEMS_VERSION_MINOR 1)
set(RCTEMS_VERSION_PATCH 0)
set(RCTEMS_VERSION "${RCTEMS_VERSION_MAJOR}.${RCTEMS_VERSION_MINOR}.${RCTEMS_VERSION_PATCH}")

project(
    ${PROJECT_NAME}
    LANGUAGES C CXX
    VERSION ${RCTEMS_VERSION}
)

# Specify the C/C++ standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

# Specify the environments
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(RCTEMS_ARCH "x86_64")
    message(STATUS "Build target architecture: ${CMAKE_SYSTEM_PROCESSOR}")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(RCTEMS_ARCH "arm64")
    message(STATUS "Build target architecture: ${CMAKE_SYSTEM_PROCESSOR}")
else()
    message(FATAL_ERROR "Unsupported target architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

option(RCTEMS_USE_MYSQL "Use MySQL database" FALSE)
# if (RCTEMS_USE_MYSQL)
#     set(RCTEMS_DATABASE_LIB "libmysqlclient.a")
# endif()

option(RCTEMS_USE_POSTGRESQL "Use PostgreSQL database" FALSE)
# if (RCTEMS_USE_POSTGRESQL)
#     set(RCTEMS_DATABASE_LIB "libpq.a")
# endif()

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE RCTEMS_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Current git commit: ${RCTEMS_GIT_HASH}")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(RCTEMS_PKT_NAME "${PROJECT_NAME}-${RCTEMS_VERSION}-${CMAKE_BUILD_TYPE}-${RCTEMS_ARCH}-${RCTEMS_GIT_HASH}")
string(TOLOWER "${RCTEMS_PKT_NAME}" RCTEMS_PKT_NAME)
set(CMAKE_INSTALL_PREFIX ${RCTEMS_PKT_NAME})
set(CMAKE_INSTALL_RPATH "../lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_executable(
    ${PROJECT_NAME}
        rctems/algorithm/adjust_power.cc
        rctems/algorithm/sample.cc
        rctems/algorithm/storage.cc
        rctems/algorithm/system_setting.cc
        rctems/algorithm/tou.cc
        rctems/communication/modbus.cc
        rctems/common/asio.cc
        rctems/common/logging.cc
        rctems/common/strings.cc
        rctems/database/database.cc
        rctems/database/mysql.cc
        rctems/database/sql.cc
        rctems/component/bess.cc
        rctems/component/bms.cc
        rctems/component/pcs.cc
        rctems/component/voltmeter.cc
        rctems/station/station.cc
        rctems/config.cc
        rctems/rctems.cc)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/third_party/boost_1_88_0/include
        ${PROJECT_SOURCE_DIR}/third_party/libmodbus-3.1.11/include
        ${PROJECT_SOURCE_DIR}/third_party/fmt-11.2.0/include
)

target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
        -Wall -Wextra -Wno-sign-compare
        -D_GNU_SOURCE ${RCTEMS_COMPILE_OPTIONS}
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        $<$<CONFIG:Debug>:-fsanitize=address>
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:-lasan>
        -L${PROJECT_SOURCE_DIR}/third_party/boost_1_88_0/lib/${RCTEMS_ARCH}
        libboost_log.a
        libboost_log_setup.a
        libboost_thread.a
        libboost_filesystem.a
        libboost_regex.a
        libboost_filesystem.a
        libboost_date_time.a
        libboost_json.a
        libboost_url.a
        -L${PROJECT_SOURCE_DIR}/third_party/libmodbus-3.1.11/lib/${RCTEMS_ARCH}
        libmodbus.a
        -ldl -lssl -lcrypto -lrt
        -lpthread
        -lm
)

if (RCTEMS_USE_MYSQL)
    target_compile_options(${PROJECT_NAME} PRIVATE -DRCTEMS_USE_MYSQL)
    target_link_libraries(${PROJECT_NAME} PRIVATE libmysqlclient.a)
endif()

if (RCTEMS_USE_POSTGRESQL)
    target_compile_options(${PROJECT_NAME} PRIVATE -DRCTEMS_USE_POSTGRESQL)
    target_link_libraries(${PROJECT_NAME} PRIVATE libpq.a)
endif()

add_executable(
    ${SIMULATOR_NAME}
        rctems/getline/linenoise.c
        rctems/simulator.cc)

target_include_directories(
    ${SIMULATOR_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/third_party/boost_1_88_0/include
        ${PROJECT_SOURCE_DIR}/third_party/libmodbus-3.1.11/include
        ${PROJECT_SOURCE_DIR}/third_party/fmt-11.2.0/include
)

target_compile_options(
    ${SIMULATOR_NAME}
    PRIVATE
        -Wall -Wextra -D_GNU_SOURCE
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        $<$<CONFIG:Debug>:-fsanitize=address>
)

target_link_libraries(
    ${SIMULATOR_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:-lasan>
        -L${PROJECT_SOURCE_DIR}/third_party/libmodbus-3.1.11/lib/${RCTEMS_ARCH}
        libmodbus.a
        -lrt -lpthread -lm
)

# make install
# add_custom_target(create_install_dir
#     COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}"
#     COMMENT "Creating install directory: ${CMAKE_INSTALL_PREFIX}"
# )
# add_dependencies(install create_install_dir)
install(CODE "file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX})")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES profile DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
    ${PROJECT_SOURCE_DIR}/etc/rctems_station_config.json.template
    DESTINATION etc
    RENAME rctems_station_config.json)
# install(CODE "file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/log)")

# make pack
add_custom_target(
    pack
    # DEPENDS install # Require cmake 3.19+
    COMMAND ${CMAKE_COMMAND} -E tar zcf ${RCTEMS_PKT_NAME}.tar.gz ${RCTEMS_PKT_NAME}
    COMMENT "Generating install package: ${RCTEMS_PKT_NAME}.tar.gz"
)