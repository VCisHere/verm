set(GLAD_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c
)

add_library(glad ${GLAD_SRC})

target_include_directories(glad PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
