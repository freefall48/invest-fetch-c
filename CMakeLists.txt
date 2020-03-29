cmake_minimum_required(VERSION 3.10)
project(invest_fetch_c C)

set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(CURL REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

add_executable(invest_fetch_c src/main.c src/nzx/models.c src/nzx/models.h src/nzx/handler.c src/nzx/handler.h src/nzx/httpOps.h src/nzx/httpOps.c src/scheduler/schedule.c src/scheduler/schedule.h src/threading/threadPool.c src/threading/threadPool.h)

target_include_directories(invest_fetch_c PRIVATE ${CURL_INCLUDE_DIR})
target_include_directories(invest_fetch_c PRIVATE ${PostgreSQL_INCLUDE_DIRS})

target_link_libraries(invest_fetch_c PRIVATE ${CURL_LIBRARIES})
target_link_libraries(invest_fetch_c PRIVATE ${PostgreSQL_LIBRARIES})

target_link_libraries(invest_fetch_c PRIVATE Threads::Threads)

set(CURL_LIBRARY "-lcurl")