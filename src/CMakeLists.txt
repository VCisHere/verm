set(VERM_FOLDER
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/display
	${CMAKE_CURRENT_SOURCE_DIR}/font
	${CMAKE_CURRENT_SOURCE_DIR}/misc
	${CMAKE_CURRENT_SOURCE_DIR}/pty
)

foreach(folder ${VERM_FOLDER})
    file(GLOB srcs ${folder}/*.cpp)
    list(APPEND VERM_SRC ${srcs})
endforeach()

add_executable(verm ${VERM_SRC})

target_include_directories(verm PUBLIC
	${THIRD_PARTY_INCLUDE_DIR}

	${VERM_FOLDER}
)

target_link_libraries(verm PUBLIC
	imgui glfw opengl32 freetype glad
)

if(MSVC)
set_target_properties(verm PROPERTIES 
    LINK_FLAGS "${LINK_FLAGS}"# /ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS"
)
endif()

add_custom_command(TARGET verm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
)
