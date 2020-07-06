cmake_minimum_required(VERSION 3.5)
project(DurPreferTopK)

set(BOOST_INCLUDEDIR /usr/include)
FIND_PACKAGE(Boost)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES 
	main.cpp
	point.h data_loader.h solution.h)

add_executable(DurPreferTopK ${SOURCE_FILES})