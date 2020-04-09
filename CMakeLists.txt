# CMakeList.txt: DataCenter 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)
cmake_policy(SET CMP0077 NEW)

project ("DataCenter")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/deps/concurrentqueue
	${PROJECT_SOURCE_DIR}/deps/cxxopts/include
	${PROJECT_SOURCE_DIR}/deps/json/single_include
	${PROJECT_SOURCE_DIR}/deps/spdlog/include
)

if (WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
else()
	set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -pg -fsanitize=undefined -Wall -Wextra")
	set(CMAKE_C_FLAGS_RELEASE "-O2")
	set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
	set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

	find_path(TDengine_INCLUDE_DIR taos.h HINT /usr/local/include)
	find_path(TDengine_LIBRARY_DIR taos HINT /usr/local/lib)
endif(WIN32)

add_subdirectory ("deps/nng")
add_subdirectory ("deps/spdlog")

if (WIN32)
	add_subdirectory ("nezipreceiver")

	# For development
	# add_subdirectory ("nezipsaver")
else()
	add_subdirectory ("nezipsaver")
	# install(FILES ${CTPLib_LIBRARIES} DESTINATION lib)
endif()
