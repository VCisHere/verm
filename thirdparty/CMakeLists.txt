# imgui
add_subdirectory(imgui-1.90.8)

# glfw
include(FetchContent)
FetchContent_Declare(glfw URL ${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
  
    set(BUILD_SHARED_LIBS OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_INSTALL OFF)
    set(GLFW_VULKAN_STATIC OFF)

    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

add_subdirectory(glm)

add_subdirectory(glad)

add_subdirectory(freetype-2.13.2)

set(THIRD_PARTY_INCLUDE_DIR
	${CMAKE_CURRENT_SOURCE_DIR}

    # imgui
	${CMAKE_CURRENT_SOURCE_DIR}/imgui-1.90.8
	${CMAKE_CURRENT_SOURCE_DIR}/imgui-1.90.8/backends

    # glfw
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4/include/GLFW

    # glm
    ${CMAKE_CURRENT_SOURCE_DIR}/glm

    # glad
	${CMAKE_CURRENT_SOURCE_DIR}/glad/include

    # freetype
    ${CMAKE_CURRENT_SOURCE_DIR}/freetype-2.13.2/include

	PARENT_SCOPE
)
