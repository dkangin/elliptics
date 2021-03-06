cmake_minimum_required (VERSION 2.6 FATAL_ERROR)
cmake_policy (VERSION 2.6.0)

#set(CMAKE_SKIP_RPATH OFF)
#set(CMAKE_SKIP_BUILD_RPATH  OFF)
#set(CMAKE_SKIP_INSTALL_RPATH ON)
#set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

project (elliptics)
FILE (READ "${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog" DEBCHANGELOG)

string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)" DEBFULLVERSION "${DEBCHANGELOG}")
STRING(REGEX MATCH "([0-9]+\\.[0-9]+)" ELLIPTICS_VERSION_ABI "${DEBFULLVERSION}")
STRING(REGEX MATCH "([0-9]+\\.[0-9]+$)" ELLIPTICS_VERSION_MINOR "${DEBFULLVERSION}")
string(REPLACE "." ";" VERSION_LIST ${DEBFULLVERSION})
list(GET VERSION_LIST 0 ELLIPTICS_VERSION_0)
list(GET VERSION_LIST 1 ELLIPTICS_VERSION_1)
list(GET VERSION_LIST 2 ELLIPTICS_VERSION_2)
list(GET VERSION_LIST 3 ELLIPTICS_VERSION_3)

add_definitions(-DCONFIG_ELLIPTICS_VERSION_0=${ELLIPTICS_VERSION_0})
add_definitions(-DCONFIG_ELLIPTICS_VERSION_1=${ELLIPTICS_VERSION_1})
add_definitions(-DCONFIG_ELLIPTICS_VERSION_2=${ELLIPTICS_VERSION_2})
add_definitions(-DCONFIG_ELLIPTICS_VERSION_3=${ELLIPTICS_VERSION_3})

option(WITH_PYTHON "Build python bindings" ON)
option(WITH_COCAINE "Build with cocaine support" ON)
option(WITH_EXAMPLES "Build example applications" ON)
option(HAVE_MODULE_BACKEND_SUPPORT "Build ioserv with shared library backend support" ON)

set(ELLIPTICS_VERSION "${ELLIPTICS_VERSION_ABI}.${ELLIPTICS_VERSION_MINOR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
include(CheckLargefile)
include(CheckAtomic)
include(CheckSendfile)
include(CheckIoprio)
include(TestBigEndian)
include(CheckProcStats)
include(CreateStdint)
include(FixRpath)

configure_file(cmake/EllipticsConfig.cmake.in
                "${PROJECT_BINARY_DIR}/cmake/EllipticsConfig.cmake"
                @ONLY)
configure_file(cmake/EllipticsConfigVersion.cmake.in
                "${PROJECT_BINARY_DIR}/cmake/EllipticsConfigVersion.cmake"
                @ONLY)

create_stdint(${CMAKE_CURRENT_SOURCE_DIR}/include/elliptics/typedefs.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Set id_size
if (NOT ID_SIZE)
    set(ID_SIZE 64)
endif()
add_definitions(-DCONFIG_ID_SIZE=${ID_SIZE})

# Test endianness
test_big_endian(HAVE_BIG_ENDIAN)
if(HAVE_BIG_ENDIAN)
    add_definitions(-DBYTEORDER=4321)
    add_definitions(-DWORDS_BIGENDIAN=1)
else()
    add_definitions(-DBYTEORDER=1234)
endif()

if (UNIX OR MINGW)
    add_definitions(-W -Wall -Wextra -fstack-protector-all -fno-strict-aliasing -g)
endif()

# Check for threads
if (UNIX AND BSD)
    set(CMAKE_THREAD_LIBS -pthread)
    set(CMAKE_USE_PTHREADS ON)
    set(CMAKE_EXE_LINKER_FLAGS -pthread -Wl,--as-needed)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

if(WITH_COCAINE)
    # Find some cocaine
    find_package(Cocaine REQUIRED)
    include_directories(${COCAINE_INCLUDE_DIRS})
    add_definitions(${COCAINE_CFLAGS})
endif()

# Find eblob
find_package(Eblob REQUIRED)
include_directories(${EBLOB_INCLUDE_DIRS})

if (HAVE_MODULE_BACKEND_SUPPORT)
    add_definitions(-DHAVE_MODULE_BACKEND_SUPPORT=1)
endif()

# GoLD does no overlinking
find_package(Boost REQUIRED COMPONENTS thread system)
message(STATUS "Boost information:")
message(STATUS "  Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "  Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
message(STATUS "  Boost_LIBRARIES: ${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# Collect all libraries together
set(ELLIPTICS_LIBRARIES
    ${SENDFILE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${EBLOB_LIBRARIES}
    ${COCAINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

# Build parts
add_subdirectory(srw)
add_subdirectory(cache)
add_subdirectory(indexes)
add_subdirectory(library)
add_subdirectory(bindings)
if(WITH_EXAMPLES)
    add_subdirectory(example)
endif()
if(WITH_COCAINE)
    add_subdirectory(cocaine/plugins)
endif()

install(FILES
        include/elliptics/core.h
        include/elliptics/interface.h
        include/elliptics/packet.h
        include/elliptics/srw.h
	include/elliptics/async_result.hpp
        include/elliptics/cppdef.h
	include/elliptics/debug.hpp
	include/elliptics/error.hpp
	include/elliptics/result_entry.hpp
	include/elliptics/session.hpp
	include/elliptics/utils.hpp
	include/elliptics/backends.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/elliptics/typedefs.h
        include/elliptics/module_backend.h
        include/elliptics/module_backend.hpp
        DESTINATION include/elliptics/
        )

FILE(GLOB man_pages "man/*")
install(FILES ${man_pages} DESTINATION share/man/man1)

enable_testing()
