file(GLOB_RECURSE minity_sources *.cpp *.h)
file(GLOB imgui_sources ${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp ${CMAKE_SOURCE_DIR}/lib/imgui/*.h)
file(GLOB tinyfd_sources ${CMAKE_SOURCE_DIR}/lib/tinyfd/tinyfiledialogs.c ${CMAKE_SOURCE_DIR}/lib/tinyfd/tinyfiledialogs.h)
file(GLOB stb_sources ${CMAKE_SOURCE_DIR}/lib/stb/*.c ${CMAKE_SOURCE_DIR}/lib/stb/*.h)

add_executable(minity ${minity_sources} ${imgui_sources} ${tinyfd_sources} ${stb_sources} )

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/lib/glm)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/lib/glbinding)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/lib/globjects)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/lib/glfw)

find_package(glm REQUIRED)
find_package(glbinding REQUIRED)
find_package(globjects REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/)
include_directories(${CMAKE_SOURCE_DIR}/lib/tinyfd/)
include_directories(${CMAKE_SOURCE_DIR}/lib/stb/)

target_link_libraries(minity PUBLIC glfw)
target_link_libraries(minity PUBLIC glbinding::glbinding )
target_link_libraries(minity PUBLIC glbinding::glbinding-aux )
target_link_libraries(minity PUBLIC globjects::globjects)

set_target_properties(minity PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
