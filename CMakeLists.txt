cmake_minimum_required(VERSION 3.20)

project("vis-osgearth" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OSG_ROOT "<NOT-FOUND>" CACHE PATH "Root of OpenSceneGraph library")
set(OSG_ROOT_DBG "<NOT-FOUND>" CACHE PATH "Root of OpenSceneGraph library (Debug)")
if (${OSG_ROOT} STREQUAL "<NOT-FOUND>")
	message(FATAL_ERROR "OSG_ROOT NOT set!")
endif()
if (${OSG_ROOT_DBG} STREQUAL "<NOT-FOUND>")
	message(FATAL_ERROR "OSG_ROOT_DBG NOT set!")
endif()

list(
	APPEND
	OSG_LIBS
	"osg"
	"osgViewer"
	"osgDB"
)

if ((${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug") OR
	${CMAKE_CONFIGURATION_TYPES} STREQUAL "Debug"
	)
	set(OSG_INC_DIR "${OSG_ROOT_DBG}/include")
	set(OSG_LIB_DIR "${OSG_ROOT_DBG}/lib")

	foreach(OSG_LIB ${OSG_LIBS})
		list(APPEND NEW_LIBS "${OSG_LIB}d")
	endforeach()
	set(OSG_LIBS ${NEW_LIBS})
else()
	set(OSG_INC_DIR "${OSG_ROOT}/include")
	set(OSG_LIB_DIR "${OSG_ROOT}/lib")
endif()

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src")

set(TARGET_NAME "demo")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
add_executable(
	${TARGET_NAME}
	"main.cpp"
 "src/scalar_viser/marching_cube_table.h")
target_include_directories(
	${TARGET_NAME}
	PRIVATE
	${OSG_INC_DIR}
)
target_link_directories(
	${TARGET_NAME}
	PRIVATE
	${OSG_LIB_DIR}
)
target_link_libraries(
	${TARGET_NAME}
	PRIVATE
	"scivis"
	${OSG_LIBS}
)
