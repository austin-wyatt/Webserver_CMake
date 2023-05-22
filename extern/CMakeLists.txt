include(FetchContent)

# ------------------------------------------------------------------------------
# zlib compression library
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        04f42ceca40f73e2978b50e93806c2a18c1281fc #zlib 1.2.13
)

FetchContent_GetProperties(zlib)

if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)
  add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
  target_include_directories(zlib PUBLIC ${zlib_BINARY_DIR} ${zlib_SOURCE_DIR})
endif()

set_target_properties(zlib
    PROPERTIES
    #ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    #LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)