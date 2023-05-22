# CMakeList.txt : CMake project for Webserver_CMake, include source and define
# project specific logic here.
#

# Add source to this project's executable.

add_executable (EasyWebserver "Definitions.h" "Endpoints.cpp" "Endpoints.h" 
	"HttpRequest.cpp" "HttpRequest.h" "HttpResponse.cpp" "HttpResponse.h" "public.h" 
	"Webserver.cpp"  "CompressionHandler.h")

configure_file(".env" "${CMAKE_BINARY_DIR}/bin/.env" COPYONLY) 

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET EasyWebserver PROPERTY CXX_STANDARD 20)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

target_link_libraries(EasyWebserver PRIVATE zlib)

set_target_properties(EasyWebserver
    PROPERTIES
    #ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    #LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# TODO: Add tests and install targets if needed.
