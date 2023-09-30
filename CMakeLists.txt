cmake_minimum_required(VERSION 3.5)

# Definitions
set(PROJECT_NAME sprite_editor)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#project(cmake_c_template)
project(${PROJECT_NAME})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

# Include paths
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)

# Assign the include directories
include_directories(${INCLUDE_DIRS})

# Find library sources
find_package(SDL2 REQUIRED)
include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})

# Build static/shared library
# add_library(${PROJECT_NAME}_LIB STATIC ${LIB_SRC_FILES})

# Install library
# install(TARGETS ${PROJECT_NAME}_LIB DESTINATION lib)

# Install library headers
# file(GLOB_RECURSE LIB_INC_FILES include/*/*.h)
# install(FILES ${LIB_INC_FILES} DESTINATION include)

set(SOURCE_FILES src/main.c)
add_executable(sprite_editor ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} m)