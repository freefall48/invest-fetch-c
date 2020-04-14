cmake_minimum_required(VERSION 3.10)
project(invest_fetch_c C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)


set(CURL_LIBRARY "-lcurl")

find_package(CURL REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)
find_package(Hiredis REQUIRED)

add_executable(invest_fetch_c src/main.c src/nzx/models.c src/nzx/models.h src/nzx/priceHandler.c src/nzx/priceHandler.h src/nzx/httpOps.h src/nzx/httpOps.c src/scheduler/schedule.c src/scheduler/schedule.h src/threading/threadPool.c src/threading/threadPool.h src/logging/logger.c src/logging/logger.h src/helpers/cron.c src/helpers/cron.h src/nzx/performance.c src/nzx/performance.h src/helpers/postgres.c src/helpers/postgres.h src/redis/manager.c src/redis/manager.h)

target_include_directories(invest_fetch_c PRIVATE ${CURL_INCLUDE_DIR})
target_include_directories(invest_fetch_c PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_include_directories(invest_fetch_c PRIVATE ${HIREDIS_INCLUDE_DIRS})

target_link_libraries(invest_fetch_c PRIVATE ${CURL_LIBRARIES})
target_link_libraries(invest_fetch_c PRIVATE ${PostgreSQL_LIBRARIES})
target_link_libraries(invest_fetch_c PRIVATE ${HIREDIS_LIBRARY})
target_link_libraries(invest_fetch_c PRIVATE Threads::Threads)
