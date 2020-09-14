add_executable(fasttokenizer
	${CMAKE_CURRENT_SOURCE_DIR}/fasttokenizer_main.cpp
)
target_link_libraries(fasttokenizer PRIVATE
	fasttokenizer-dev ${LINK_LIBRARIES}
)
