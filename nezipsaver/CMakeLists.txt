cmake_minimum_required (VERSION 3.8)

if (WIN32)
include_directories(
	${CMAKE_SOURCE_DIR}/deps/TDengine/src/inc
)
endif()

set(TARGET_NAME nezipsaver)
add_executable(${TARGET_NAME}
	saver.h
	saver.cc
	receiver.h
	receiver.cc
	service.h
	service.cc
	main.cc
)

add_dependencies(${TARGET_NAME} nng spdlog)
target_link_libraries(${TARGET_NAME}
	nng
	spdlog
)
