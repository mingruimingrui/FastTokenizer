#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "fasttokenizer/segmenter.h"

namespace py = pybind11;

#ifdef TOKENIZER_NAMESPACE
using namespace TOKENIZER_NAMESPACE ;
#endif

PYBIND11_MODULE(_fasttokenizer, m) {
    py::class_<Segmenter>(m, "Segmenter")
        .def(py::init<const bool>())
        .def("clone", &Segmenter::clone, "Clone this segmenter.")
        .def(
            "normalize",
            (std::string (Segmenter::*)(const std::string&))
            &Segmenter::normalize
        )
        .def(
            "segment",
            (std::string (Segmenter::*)(const std::string&))
            &Segmenter::segment
        )
        .def(
            "normalize_and_segment",
            (std::string (Segmenter::*)(const std::string&))
            &Segmenter::normalize_and_segment
        )
        .def(
            "desegment",
            (std::string (Segmenter::*)(const std::string&))
            &Segmenter::desegment
        );

#ifdef TOKENIZER_VERSION_INFO
    m.attr("__version__") = TOKENIZER_VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif

};
