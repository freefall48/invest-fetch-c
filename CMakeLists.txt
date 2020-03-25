cmake_minimum_required(VERSION 3.0.0)
project(invest-fetch VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(invest-fetch invest.c nzx.c nzx.h nzxList.c nzxList.h)

find_package(CURL REQUIRED) 


include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(invest-fetch ${CURL_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CURL_LIBRARY "-lcurl") 
include(CPack)
