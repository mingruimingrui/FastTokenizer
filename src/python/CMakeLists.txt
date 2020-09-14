add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../deps/pybind11-2.5.0)
pybind11_add_module(_fasttokenizer
	${CMAKE_CURRENT_SOURCE_DIR}/fasttokenizer_py.cpp
)
target_link_libraries(_fasttokenizer PRIVATE
	fasttokenizer-dev ${LINK_LIBRARIES}
)
