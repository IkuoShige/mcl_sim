cmake_minimum_required(VERSION 3.15)
# project(mcl_call)
project(mcl_module)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/c++/11 -I/usr/include/x86_64-linux-gnu/c++/11")
set(CMAKE_CXX_STANDARD 14)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)
set(Eigen3_LIBRARIES Eigen3::Eigen)
find_package(OpenCV REQUIRED)

pybind11_add_module(mcl_module mcl.cpp mcl.h)
target_link_libraries(mcl_module PUBLIC ${OpenCV_LIBRARIES} ${Eigen3_LIBRARIES})

# find_package(SWIG REQUIRED)
# find_package(PythonLibs REQUIRED)

# include(${SWIG_USE_FILE})# need for swig_add_module
# include(GenerateExportHeader)
# include_directories(
#     ${CMAKE_CURRENT_SOURCE_DIR}
#     ${PROJECT_BINARY_DIR}
#     ${PYTHON_INCLUDE_PATH}
# )

# set(EXAM_SRCS
#     mcl_call.h
#     mcl_call.cpp
#     mcl.h
#     mcl.cpp
# )
# set(INTERFACE_FILES
#     mcl_call.i
# )
# set_source_files_properties(${INTERFACE_FILES} PROPERTIES CPLUSPLUS ON)
# set_source_files_properties(${INTERFACE_FILES} PROPERTIES SWIG_FLAGS "-includeall")
# set_property(SOURCE ${INTERFACE_FILES} PROPERTY SWIG_COMPILE_OPTIONS -std=c++11 -fPIC -fdiagnostics-color=auto)
# set(SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)

# swig_add_library(mcl_call LANGUAGE python SOURCES ${INTERFACE_FILES}
#     ${EXAM_SRCS}
# )
# target_link_libraries(mcl_call
#     ${PYTHON_LIBRARIES}
# )
# set_target_properties(mcl_call PROPERTIES
#     SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
# )

