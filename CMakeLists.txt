cmake_minimum_required(VERSION 3.16)
project(LandingRateMonitor)

add_library(LandingRateMonitor SHARED src/landing_monitor.c
        src/landing_monitor.c)

target_include_directories(LandingRateMonitor PRIVATE
        ${CMAKE_SOURCE_DIR}/SDK/CHeaders/XPLM
)

target_compile_definitions(LandingRateMonitor PRIVATE
        XPLM200=1
        XPLM300=1
        XPLM301=1
        APL=1
        IBM=0
        LIN=0
)

set_target_properties(LandingRateMonitor PROPERTIES
        OUTPUT_NAME "LandingRateMonitor"
        SUFFIX ".xpl"
        PREFIX ""
        OSX_ARCHITECTURES "x86_64;arm64"
)