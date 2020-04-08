include (ExternalProject)

set_property (DIRECTORY PROPERTY EP_BASE deps-build)

set(INSTALL_DIR "/usr/local")

set (DEPENDENCIES)
set (EXTRA_CMAKE_ARGS)
list (APPEND EXTRA_CMAKE_ARGS
  -DDEPS_INSTALLED_DIR=${INCLUDE_DIR})

# list (APPEND DEPENDENCIES protobuf)
# ExternalProject_Add(protobuf
#	URL 		"https://github.com/protocolbuffers/protobuf/archive/v3.10.1.tar.gz"
#	URL_MD5     "021c09b560ab9651967afc8d9450cca2"
#	CMAKE_CACHE_ARGS
#		"-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
#		"-Dprotobuf_BUILD_TESTS:BOOL=OFF"
#		"-Dprotobuf_BUILD_EXAMPLES:BOOL=OFF"
#		"-Dprotobuf_WITH_ZLIB:BOOL=OFF"
#		"-DCMAKE_INSTALL_PREFIX:STRING=${INSTALL_DIR}"
#	SOURCE_SUBDIR cmake
# )

list (APPEND DEPENDENCIES TDengine)
ExternalProject_Add(TDengine
	SOURCE_DIR     "${CMAKE_SOURCE_DIR}/deps/TDengine"
	UPDATE_COMMAND git reset --hard release/v1.6.4.0
	COMMAND        sed -i -E "s/(\\s*install_service\\s*)$/#\\1/" <SOURCE_DIR>/packaging/tools/make_install.sh
)

ExternalProject_Add (DataCenter
  DEPENDS ${DEPENDENCIES}
  SOURCE_DIR ${PROJECT_SOURCE_DIR}
  CMAKE_ARGS -DUSE_SUPERBUILD=OFF ${EXTRA_CMAKE_ARGS}
  INSTALL_COMMAND ""
)
