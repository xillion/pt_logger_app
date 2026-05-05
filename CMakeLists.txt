cmake_minimum_required(VERSION 3.20.0)

# Set the boards root to the current directory so Zephyr can find out-of-tree boards
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pt_logger_app)
#add_subdirectory(libs/max31888)

#target_sources(app PRIVATE src/main.cpp mbed/rv3129-c3/RV3129C3.cpp mbed/max31888/MAX31888.cpp)
target_sources(app PRIVATE src/main.cpp)
#target_include_directories(app PRIVATE mbed/rv3129-c3 mbed/ad5142a mbed/max31888)
