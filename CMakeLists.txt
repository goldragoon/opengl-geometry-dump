cmake_minimum_required (VERSION 3.0)
project ("opengl-geometry-dump" LANGUAGES CXX)

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(OpenMesh REQUIRED)

# Soft-link files
set(DIR_SHADER ${CMAKE_SOURCE_DIR}/shader)

# Build Main Program
set(SOURCES ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB FILES_SOURCES CONFIGURE_DEPENDS "${SOURCES}/*.h" "${SOURCES}/*.hpp" "${SOURCES}/*.cpp")
file(COPY data DESTINATION ${CMAKE_BINARY_DIR})

add_executable (main ${FILES_SOURCES} )
target_include_directories(main PUBLIC 
${GLUT_INCLUDE_DIR}
${GLM_INCLUDE_DIR}
${GLEW_INCLUDE_DIR}
${OPENMESH_INCLUDE_DIR}
)

target_link_libraries(main 
${GLUT_LIBRARIES} 
${GLEW_LIBRARIES} 
${GLM_LIBARIES} 
${OPENMESH_LIBRARIES}
)

# POST-BUILD WORKS
# 1. Copy soft-linked files.
# - Copy Method 1: Can only executes on cmake-configure time.
# file(COPY ${DIR_SHADER}/shader DESTINATION ${CMAKE_BINARY_DIR}) 
# - Copy Method 2: Can run in every build time(make), but shader-only file change does not occur build.
add_custom_command(
         TARGET main POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory ${DIR_SHADER} ${CMAKE_BINARY_DIR}/shader
         COMMENT "Copying Shaders" VERBATIM)
