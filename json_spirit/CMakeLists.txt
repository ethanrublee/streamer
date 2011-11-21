SET(JSON_SPIRIT_SRCS
json_spirit_reader.cpp
json_spirit_value.cpp
json_spirit_writer.cpp)

FIND_PACKAGE(Boost 1.34 REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

ADD_LIBRARY(streamer_json_spirit STATIC ${JSON_SPIRIT_SRCS})
set_target_properties(streamer_json_spirit PROPERTIES
    COMPILE_FLAGS "-fpic -Wall"
    )

