cmake_minimum_required(VERSION 3.7)
project(Library_Management)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp MyDatabase.cpp MyDatabase.h Library.cpp Library.h)

add_executable(Library_Management ${SOURCE_FILES})

execute_process(COMMAND mysql_config --libs OUTPUT_VARIABLE MYSQL_LIBS) # mysql_lib local
string(STRIP ${MYSQL_LIBS} MYSQL_LIBS) # strip due to policy CMP0004

target_link_libraries(Library_Management ${MYSQL_LIBS}) # link
