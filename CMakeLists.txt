cmake_minimum_required(VERSION 3.30.4)

project(
	VulkanEngine
	DESCRIPTION "A modern C++26 based on VulkanAPI game engine"
	VERSION 0.0.0
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "Build type: DEBUG")
else()
	message(STATUS "Build type: RELEASE")
	add_compile_options(
		-O3
		-Wno-unused-parameter
		-Wno-unused-value
		-Wno-missing-field-initializers
		-Wno-narrowing
		-Wno-pointer-arith
	)
endif()

add_compile_options(
	-Wall
	-Wextra
)

# Setting up building
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

file(GLOB SHADERS "shaders/*.vert" "shaders/*.frag")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Checking build platform
if(WIN32)
	if(VULKAN_DIR)
		message(STATUS "Founded vulkan sdk directory")
		set(VULKAN_INCLUDE "${VULKAN_DIR}/Include")
		set(VULKAN_LIB "${VULKAN_DIR}/Lib")
		set(GLSLC "${VULKAN_DIR}/Bin/glslc.exe")
	else()
		message(FATAL_ERROR "Vulkan SDK directory not found! Please set the VULKAN_DIR variable")
	endif()

	if(GLFW_DIR)
		message(STATUS "Founded glfw directory")
		set(GLFW_INCLUDE "${GLFW_DIR}/include")
		set(GLFW_LIB "${GLFW_DIR}/lib")
	else()
		message(FATAL_ERROR "GLFW directory not found! Please set the GLFW_DIR variable")
	endif()

    target_include_directories(${PROJECT_NAME} PRIVATE ${VULKAN_INCLUDE} ${GLFW_INCLUDE})
    target_link_directories(${PROJECT_NAME} PRIVATE ${VULKAN_LIB} ${GLFW_LIB})

    target_link_libraries(${PROJECT_NAME} PRIVATE glfw3.lib vulkan-1.lib)
else()
    find_package(Vulkan REQUIRED)
	set(GLSLC "glslc")
    find_package(glfw3 REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} "src")
    target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw)
endif()

# Compiling shaders
foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME_WE)
	set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
    )
    list(APPEND SPIRV_SHADERS ${SPIRV})
endforeach()

add_custom_target(Shaders DEPENDS ${SPIRV_SHADERS})
