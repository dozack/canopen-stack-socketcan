cmake_minimum_required(VERSION 3.20)

add_library(CanopenSocketCanDriver STATIC)

target_include_directories(CanopenSocketCanDriver
    PUBLIC
    ../include
    )

target_sources(CanopenSocketCanDriver
    PRIVATE
    ../src/co_can_socketcan.c
    )

target_link_libraries(CanopenSocketCanDriver
    Canopen
    )

