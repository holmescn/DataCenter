cmake_minimum_required (VERSION 3.8)

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
if (WIN32)
target_include_directories(${TARGET_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/deps/TDengine/src/inc
)
else()
target_include_directories(${TARGET_NAME} PRIVATE
	${TDengine_INCLUDE_DIR}
)
target_link_directories(${TARGET_NAME} PRIVATE
	${TDengine_LIBRARY_DIR}
)
endif()

target_link_libraries(${TARGET_NAME}
	nng
	spdlog
	taos
)

install(
	TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin                     
         CONFIGURATIONS Release                 
)                                               

install(
	TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)
