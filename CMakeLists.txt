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
add_subdirectory(json_spirit)

find_package(OpenCV REQUIRED)
include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

add_library(ecto_streamer_serv SHARED
  connection.cpp
  mime_types.cpp
  reply.cpp
  request_handler.cpp
  request_parser.cpp
  server.cpp
  mjpeg_server.cpp
  )

target_link_libraries(ecto_streamer_serv
  ${OpenCV_LIBS}
  boost_thread boost_system boost_regex
  )

set_target_properties(ecto_streamer_serv PROPERTIES
    COMPILE_FLAGS "-fpic -Wall -Werror"
    )

add_executable(httpserv  main.cpp)
target_link_libraries(httpserv
  ecto_streamer_serv
  )

set_target_properties(httpserv PROPERTIES
    COMPILE_FLAGS "-fpic -Wall -Werror"
    )

install(TARGETS ecto_streamer_serv
  DESTINATION lib
  COMPONENT main
)

find_package(ecto)
if(ecto_FOUND)
    ecto_python_env_gen(${CMAKE_BINARY_DIR}/lib)
    ectomodule(mjpeg_server
       module.cpp
       cell_parameter_handlers.cpp
    )
    link_ecto(mjpeg_server
      ecto_streamer_serv
      streamer_json_spirit
      boost_thread boost_system boost_regex
      ${OpenCV_LIBS}
    )
    install_ecto_module(mjpeg_server)
endif()
  
