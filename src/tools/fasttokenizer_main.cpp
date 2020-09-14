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
    unsigned int flag = -1;
    int num_threads = 4;
    bool quiet = false;
} Args;

Args args;

vecstr* segment_lines(vecstr* lines) {
    Segmenter segmenter = Segmenter(args.protected_dash_split);
    int num_lines = lines->size();
    std::string out;
    for (int i=0; i<num_lines; ++i) {
        out.clear();

        switch (args.flag) {
        case 3:
            segmenter.desegment(lines->at(i), out);
            break;

        case 2:
            segmenter.segment(lines->at(i), out);
            break;

        case 1:
            segmenter.normalize(lines->at(i), out);
            break;

        default:
            segmenter.normalize_and_segment(lines->at(i), out);
            break;
        }

        (*lines)[i] = out;
    };
    return lines;
}

unsigned long run(std::istream* input_stream) {
    unsigned long num_lines = 0;

    ThreadPool pool(args.num_threads);
    std::queue<std::future<vecstr*>> result_queue;
    unsigned int result_queue_size = 0;
    unsigned int max_chunks = args.num_threads * 8;

    vecstr* ibuffer = new vecstr();
    ibuffer->reserve(CHUNKSIZE);
    unsigned int ibuffer_size = 0;

    vecstr* obuffer;

    std::string line;
    while (getline(*input_stream, line)) {
        ibuffer->push_back(line);
        ++ibuffer_size;

        if (ibuffer_size >= CHUNKSIZE) {
            result_queue.push(pool.enqueue(segment_lines, ibuffer));
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

            ibuffer = new vecstr();
            ibuffer->reserve(CHUNKSIZE);
            ibuffer_size = 0;
        };
    };

    if (ibuffer_size > 0) {
        result_queue.push(pool.enqueue(segment_lines, ibuffer));
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
    if (args.num_threads <= 0) throw "num_threads must be a positive value";
    if (args.norm_only && args.segm_only)
        throw "Cannot have both norm_only and segm_only";

    args.flag = 0;
    if (args.norm_only) args.flag = 1;
    if (args.segm_only) args.flag = 2;
    if (args.desegment) args.flag = 3;

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

    // Run
    auto begin = std::chrono::steady_clock::now();
    unsigned long num_lines = 0;
    if (args.input == "-") {
        num_lines = run(&std::cin);
    } else {
        std::fstream input_stream;
        input_stream.open(args.input, std::fstream::in);
        num_lines = run(&input_stream);
    }

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

    return 0;
}
