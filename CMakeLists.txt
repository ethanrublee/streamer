cmake_minimum_required(VERSION 2.8)
project(mjpeg_server)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Boost COMPONENTS
  thread
  system
  regex
  REQUIRED
  )

if (Boost_FOUND)
  message(STATUS "Boost headers at ${Boost_INCLUDE_DIRS}")
  message(STATUS "Boost libraries at ${Boost_LIBRARIES}")
endif()

find_package(OpenCV REQUIRED)
include_directories(
  ${Boost_INCLUDE_DIRS}
  )

add_library(asio_serv SHARED
  connection.cpp
  mime_types.cpp
  reply.cpp
  request_handler.cpp
  request_parser.cpp
  server.cpp
  mjpeg_server.cpp
  )

set_target_properties(asio_serv PROPERTIES
    COMPILE_FLAGS "-fpic -Wall -Werror"
    )

add_executable(httpserv  main.cpp)
target_link_libraries(httpserv
  asio_serv
  boost_thread boost_system boost_regex
  ${OpenCV_LIBS}
  )
set_target_properties(httpserv PROPERTIES
    COMPILE_FLAGS "-fpic -Wall -Werror"
    )

find_package(ecto)
if(ecto_FOUND)
    ecto_python_env_gen(${CMAKE_BINARY_DIR}/lib)
    ectomodule(mjpeg_server
         module.cpp
    )
    link_ecto(mjpeg_server
      asio_serv
      boost_thread boost_system boost_regex
      ${OpenCV_LIBS}
    )
    install_ecto_module(mjpeg_server)
endif()
  
