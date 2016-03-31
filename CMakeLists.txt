cmake_minimum_required(VERSION 3.4)
project(Salangane)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp util.h util.cpp base/Thread.h base/Atomic.h base/Types.h base/Thread.cpp base/CurrentThread.h base/Exception.h base/Exception.cpp base/Logging.h base/StringPiece.h base/LogStream.h)
add_executable(Salangane ${SOURCE_FILES})