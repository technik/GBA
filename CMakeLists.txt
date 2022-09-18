################################################################################
# GBA Projects
################################################################################
cmake_minimum_required (VERSION 3.10)
project(GBA)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)

# Clasify sources according to folder structure. Useful for having nice visual studio filters.
# This macro is derived from http://www.cmake.org/pipermail/cmake/2013-November/056336.html
macro(GroupSources curdir dirLabel)
	file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir}
		${PROJECT_SOURCE_DIR}/${curdir}/*)
	foreach(child ${children})
		if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			GroupSources(${curdir}/${child} ${dirLabel}/${child})
		else()
			string(REPLACE "/" "\\" groupname ${dirLabel})
			source_group(${groupname} FILES
				${PROJECT_SOURCE_DIR}/${curdir}/${child})
		endif()
	endforeach()
endmacro()

################################################################################
# Actual pathtracer code
################################################################################

# Collect all sources
file(GLOB_RECURSE COMMON_FILES "pc/include/*.h" "common/source/*.cpp" "common/include/*.inl" "common/include/*.h" "common/include/*.hpp")
GroupSources(common common)
GroupSources(pc pc)

include_directories(common/include)
include_directories(pc/include)
include_directories(raycaster/include)
include_directories(raycaster/assets)
add_executable(pcRT pc/rt/main.cpp ${COMMON_FILES} raycaster/source/SectorRasterizer.iwram.cpp raycaster/source/SectorRasterizer.cpp raycaster/assets/mercury.wad.cpp raycaster/assets/test.wad.cpp)

add_executable(lutGenerator tools/lutGenerator/main.cpp)
set_target_properties(lutGenerator PROPERTIES FOLDER tools)

add_executable(pngToCpp tools/pngToCpp/main.cpp ${COMMON_FILES})
set_target_properties(pngToCpp PROPERTIES FOLDER tools)

add_executable(wadToCpp tools/wadToCpp/main.cpp ${COMMON_FILES})
set_target_properties(wadToCpp PROPERTIES FOLDER tools)

### Tests
enable_testing()

add_executable(fixedPointMath test/fixedPointMath.cpp)
set_target_properties(fixedPointMath PROPERTIES FOLDER test)
add_test(fixed_point_math_test fixedPointMath)

add_executable(cameraTest test/cameraTest.cpp)
set_target_properties(cameraTest PROPERTIES FOLDER test)
add_test(camera_test cameraTest)