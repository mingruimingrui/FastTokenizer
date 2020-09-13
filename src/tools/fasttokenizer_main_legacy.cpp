#include <string>
#include <chrono>
#include <queue>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include "fasttokenizer/segmenter.h"

#ifdef TOKENIZER_NAMESPACE
using namespace TOKENIZER_NAMESPACE;
#endif

typedef std::vector<std::string> vecstr;

unsigned int CHUNKSIZE = 10000;

void segment_lines(vecstr* lines, unsigned int flag) {
    Segmenter segmenter = Segmenter();
    int num_lines = lines->size();
    std::string out;
    for (int i=0; i<num_lines; ++i) {
        out.clear();

        switch (flag) {
        case 4:
            segmenter.desegment(lines->at(i), out);
            break;

        case 3:
            segmenter.normalize_and_segment(lines->at(i), out);
            break;

        case 2:
            segmenter.segment(lines->at(i), out);
            break;

        case 1:
            segmenter.normalize(lines->at(i), out);
            break;

        default:
            // Case 0
            out = lines->at(i);
            break;
        }

        (*lines)[i] = out;
    };
}

unsigned long run(
    std::istream* input_stream,
    unsigned int flag
) {
    unsigned long num_lines = 0;

    vecstr buffer;
    buffer.reserve(CHUNKSIZE);
    unsigned int buffer_size = 0;

    std::string line;
    std::string out;
    while (getline(*input_stream, line)) {
        buffer.push_back(line);
        ++buffer_size;

        if (buffer_size >= CHUNKSIZE) {
            segment_lines(&buffer, flag);

            for (std::string segmented_line: buffer) {
                std::cout << segmented_line << "\n";
                ++ num_lines;
            };
            std::cerr << "\r" << num_lines;

            buffer.clear();
            buffer_size = 0;
        };
    };

    for (std::string segmented_line: buffer) {
        std::cout << segmented_line << "\n";
        ++ num_lines;
    };
    std::cerr << "\r" << num_lines << " Done!" << std::endl;
    std::cout << std::flush;

    return num_lines;
}

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // Parse args
    CLI::App app{"Fast Tokenizer CLI"};

    std::string input = "-";
    app.add_option(
        "-i,--input", input,
        "Input stream, defaults to stdin.");

    bool do_desegment = false;
    app.add_flag(
        "-d,--desegment", do_desegment,
        "Perform desegmentation");

    bool do_norm = true;
    app.add_flag(
        "!--no-norm", do_norm,
        "Do not perform normalization.");

    bool do_segm = true;
    app.add_flag(
        "!--no-segm", do_segm,
        "Do not perform segmentation.");

    CLI11_PARSE(app, argc, argv);

    // Validate args
    unsigned int flag = 0;
    if (do_norm) flag += 1;
    if (do_segm) flag += 2;
    if (do_desegment) flag = 4;

    std::cerr << "input: " << input << std::endl;
    std::cerr << "do_desegment: " << do_desegment << std::endl;
    std::cerr << "do_norm: " << do_norm << std::endl;
    std::cerr << "do_segm: " << do_segm << std::endl;
    std::cerr << std::endl;

    // Run
    auto begin = std::chrono::steady_clock::now();
    unsigned long num_lines = 0;
    if (input == "-") {
        num_lines = run(&std::cin, flag);
    } else {
        std::fstream input_stream;
        input_stream.open(input, std::fstream::in);
        num_lines = run(&input_stream, flag);
    }

    // Print out some statistics
    auto end = std::chrono::steady_clock::now();
    auto time_taken =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    long long millis_elapsed = time_taken.count();
    float sec_elapsed = std::max(millis_elapsed, (long long)(1)) / float(1000);

    std::cerr << "Time taken: " << millis_elapsed << "[ms]" << std::endl;
    std::cerr << "Num lines: " << num_lines << std::endl;
    std::cerr << "Rate: " << num_lines / sec_elapsed << " lines/s" << std::endl;

    return 0;
}
