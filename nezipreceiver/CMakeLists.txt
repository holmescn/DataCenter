cmake_minimum_required (VERSION 3.8)

add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
set(TARGET_NAME NeZipReceiver)
add_executable(${TARGET_NAME} WIN32
	basewindow.h
	mainwindow.h
	mainwindow.cc
	worker.h
	worker.cc
	main.cc
)

add_dependencies(${TARGET_NAME} nng)
target_link_libraries(${TARGET_NAME} PRIVATE
	Threads::Threads
	nng
)

set_target_properties(${TARGET_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)