cmake_minimum_required(VERSION 3.4)
project(Salangane)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lrt -Wall -Wextra -Werror -rdynamic -std=c++11")
#set_target_properties(PROPERTIES LINK_FLAGS "ENV{}")
set(SOURCE_FILES main.cpp base/Thread.h base/Atomic.h base/Types.h base/Thread.cpp base/CurrentThread.h base/Exception.h base/Exception.cpp base/Logging.h base/StringPiece.h base/LogStream.h base/LogStream.cpp base/Timestamp.h base/copyable.h base/Timestamp.cpp base/Logging.cpp base/TimeZone.h base/TimeZone.cpp base/Date.h base/Date.cpp base/Mutex.h base/Condition.h base/ThreadLocal.h base/ThreadPool.h base/ThreadPool.cpp base/ThreadLocalSingleton.h base/Condition.cpp base/CountDownLatch.h base/CountDownLatch.cpp base/BlockingQueue.h base/BoundedBlockingQueue.h base/ProcessInfo.h base/ProcessInfo.cpp base/FileUtil.h base/FileUtil.cpp base/AsyncLogging.h base/AsyncLogging.cpp base/LogFile.h base/LogFile.cpp base/Singleton.h base/GzipFile.h base/WeakCallback.h)
#find_package(Protobuf)
#find_package(CURL)
#find_package(ZLIB)
#find_library(GD_LIBRARY NAMES gd)
#add_library(salangane_base ${SOURCE_FILES})
#set(CMAKE_SHARED_LINKER_FLAGS "-lmaplec -Wl, --no-as-needed -lrt")
#set(CMAKE_SHARED_LINKER_FLAGS "-lmaplec -Wl -lgccpp -lgc, --no-as-needed -rt")
#set(CMAKE_EXE_LINKER_FLAGS "-pthread -lrt ")
#set(CMAKE_EXE_LINKER_FLAGS "-lrt")
#install(TARGETS salangane_base DESTINATION lib)
add_executable(Salangane ${SOURCE_FILES})
target_link_libraries(Salangane -pthread -lrt)
