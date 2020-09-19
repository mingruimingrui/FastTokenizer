#include <string>
#include <chrono>
#include <queue>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"
#include "ThreadPool.h"

#include "fasttokenizer/segmenter.h"

#ifdef TOKENIZER_NAMESPACE
using namespace TOKENIZER_NAMESPACE;
#endif

typedef std::vector<std::string> vecstr;

unsigned int CHUNKSIZE = 10000;

// CLI args
typedef struct {
    std::string input = "-";
    bool protected_dash_split = false;
    bool desegment = false;
    bool norm_only = false;
    bool segm_only = false;
    int num_threads = 4;
    bool quiet = false;
} Args;

Args args;
unsigned int flag = -1;
Segmenter* segmenter;

vecstr* segment_lines(vecstr* lines) {
    Segmenter* segmenter_copy = segmenter->clone();
    int num_lines = lines->size();

    std::string input_text;
    std::string output_text;
    for (int i=0; i<num_lines; ++i) {
        input_text = lines->at(i);
        output_text.clear();

        switch (flag) {
        case 3:
            segmenter_copy->desegment(input_text, output_text);
            break;

        case 2:
            segmenter_copy->segment(input_text, output_text);
            break;

        case 1:
            segmenter_copy->normalize(input_text, output_text);
            break;

        default:
            segmenter_copy->normalize_and_segment(input_text, output_text);
            break;
        }

        lines->at(i) = output_text;
    };
    delete segmenter_copy;
    return lines;
}

size_t run(FILE* input_stream) {
    size_t num_lines = 0;

    ThreadPool pool(args.num_threads);
    std::queue<std::future<vecstr*>> result_queue;
    unsigned int result_queue_size = 0;
    unsigned int max_chunks = args.num_threads * 8;

    vecstr* input_buffer = new vecstr();
    input_buffer->reserve(CHUNKSIZE);
    unsigned int input_buffer_size = 0;

    vecstr* obuffer;

    char* line;
    size_t length;
    while ((line = fgetln(input_stream, &length)) != nullptr) {
        line[length - 1] = 0;
        input_buffer->push_back(std::string(line));
        ++input_buffer_size;

        if (input_buffer_size >= CHUNKSIZE) {
            result_queue.push(pool.enqueue(segment_lines, input_buffer));
            ++result_queue_size;

            if (result_queue_size >= max_chunks) {
                obuffer = result_queue.front().get();
                result_queue.pop();
                --result_queue_size;

                for (std::string segmented_line: *obuffer) {
                    std::cout << segmented_line << "\n";
                    ++num_lines;
                };
                if (!args.quiet) std::cerr << "\r" << num_lines;

                delete obuffer;
            }

            input_buffer = new vecstr();
            input_buffer->reserve(CHUNKSIZE);
            input_buffer_size = 0;
        };
    };

    if (input_buffer_size > 0) {
        result_queue.push(pool.enqueue(segment_lines, input_buffer));
    };

    while (!result_queue.empty()) {
        obuffer = result_queue.front().get();
        result_queue.pop();

        for (std::string segmented_line: *obuffer) {
            std::cout << segmented_line << "\n";
            ++num_lines;
        };
        if (!args.quiet) std::cerr << "\r" << num_lines;

        delete obuffer;
    };
    std::cout << std::flush;
    if (!args.quiet) std::cerr << "\r" << num_lines << " Done!" << std::endl;

    return num_lines;
}

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // Parse args
    CLI::App app{"Fast Tokenizer CLI"};
    app.add_option(
        "-i,--input", args.input,
        "Input stream, defaults to stdin.");
    app.add_flag(
        "-p,--protected-dash-split", args.protected_dash_split,
        "Perform protected dash split.");
    app.add_flag(
        "-n,--norm-only", args.norm_only,
        "Do not perform normalization.");
    app.add_flag(
        "-s,--segm-only", args.segm_only,
        "Do not perform segmentation.");
    app.add_flag(
        "-d,--desegment", args.desegment,
        "Perform desegmentation.");
    app.add_option(
        "-j,--num-threads", args.num_threads,
        "Number of threads to use.");
    app.add_flag(
        "-q,--quiet", args.quiet,
        "Run in quiet mode.");

    CLI11_PARSE(app, argc, argv);

    // Validate args
    if (args.num_threads <= 0)
        throw std::runtime_error("num_threads must be a positive value");
    if (args.norm_only && args.segm_only)
        throw std::runtime_error("Cannot have both norm_only and segm_only");

    if (args.norm_only) flag = 1;
    else if (args.segm_only) flag = 2;
    else if (args.desegment) flag = 3;
    else flag = 0;

    if (!args.quiet) {
        std::cerr << "input: " << args.input << std::endl;
        std::cerr << "protected_dash_split: "
            << args.protected_dash_split << std::endl;
        std::cerr << "norm_only: " << args.norm_only << std::endl;
        std::cerr << "segm_only: " << args.segm_only << std::endl;
        std::cerr << "desegment: " << args.desegment << std::endl;
        std::cerr << "num_threads: " << args.num_threads << std::endl;
        std::cerr << std::endl;
    };

    FILE* input_stream;
    if (args.input == "-") input_stream = stdin;
    else input_stream = fopen(args.input.c_str(), "r");
    if (input_stream == nullptr)
        throw std::runtime_error("Input file not founds.");

    segmenter = new Segmenter(args.protected_dash_split);

    // Run
    auto begin = std::chrono::steady_clock::now();
    size_t num_lines = run(input_stream);

    // Print out some statistics
    auto end = std::chrono::steady_clock::now();
    auto time_taken =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    long long millis_elapsed = time_taken.count();
    float sec_elapsed = std::max(millis_elapsed, (long long)(1)) / float(1000);

    if (!args.quiet) {
        std::cerr << "Time taken: " << millis_elapsed << "[ms]" << std::endl;
        std::cerr << "Num lines: " << num_lines << std::endl;
        std::cerr << "Rate: " << num_lines / sec_elapsed
            << " lines/s" << std::endl;
    };

    delete segmenter;
    return 0;
}
