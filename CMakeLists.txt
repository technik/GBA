################################################################################
# GBA Projects
################################################################################
cmake_minimum_required (VERSION 3.10)
project(GBA)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)

include_directories(common/include)
add_executable(pcRT pc/rt/main.cpp)

enable_testing()
add_executable(fixedPointMath test/fixedPointMath.cpp)
set_target_properties(fixedPointMath PROPERTIES FOLDER test)
add_test(fixed_point_math_test fixedPointMath)