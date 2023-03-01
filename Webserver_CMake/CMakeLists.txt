# CMakeList.txt : CMake project for Webserver_CMake, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (EasyWebserver "ContentTypes.h" "Endpoints.cpp" "Endpoints.h" "HttpRequest.cpp" "HttpRequest.h" "HttpResponse.cpp" "HttpResponse.h" "public.h" "Webserver.cpp")

configure_file(".env" ".env" COPYONLY) 

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET EasyWebserver PROPERTY CXX_STANDARD 20)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# TODO: Add tests and install targets if needed.
