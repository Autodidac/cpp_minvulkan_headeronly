# Create the static library target
add_library(VulkanStaticLib1 STATIC)

# Specify include directories for VulkanStaticLib1
target_include_directories(VulkanStaticLib1
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${VCPKG_INSTALLATION_ROOT}/x64-linux/include
)

# Add all the source files for the static library
target_sources(VulkanStaticLib1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanStaticLib1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkancore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkanbuffers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkancommands.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkantextures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkanpipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkandescriptors.cpp
)

# Find necessary packages
find_package(glfw3 CONFIG REQUIRED)

# Link GLM (header-only) to VulkanStaticLib1
target_link_libraries(VulkanStaticLib1 PUBLIC glm::glm)

# Compiler-specific options for GNU/Clang/MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(VulkanStaticLib1 PRIVATE -Wall -Wextra -Wpedantic -std=c++20)
elseif(MSVC)
    target_compile_options(VulkanStaticLib1 PRIVATE /W4 /std:c++20)
endif()

# Installation rules for VulkanStaticLib1
install(TARGETS VulkanStaticLib1
    ARCHIVE DESTINATION lib    # For static libraries on Unix
    LIBRARY DESTINATION lib    # For shared libraries (if any)
    RUNTIME DESTINATION bin    # For executables (if any)
)

# Install public headers from the "include" directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
