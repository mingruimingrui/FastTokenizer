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

unsigned int CHUNKSIZE = 8192;
unsigned int MAXCHUNKS = 512;

vecstr* segment_lines(vecstr* lines) {
    Segmenter segmenter = Segmenter();
    int num_lines = lines->size();
    std::string out;
    for (int i=0; i<num_lines; ++i) {
        out.clear();
        segmenter.normalize_and_segment(lines->at(i), out);
        (*lines)[i] = out;
    };
    return lines;
}

unsigned long run(
    std::istream* input_stream,
    unsigned int num_threads
) {
    unsigned long num_lines = 0;

    ThreadPool pool(num_threads);
    std::queue<std::future<vecstr*>> result_queue;
    unsigned int result_queue_size = 0;

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

            if (result_queue_size >= MAXCHUNKS) {
                obuffer = result_queue.front().get();
                result_queue.pop();
                --result_queue_size;

                for (std::string segmented_line: *obuffer) {
                    std::cout << segmented_line << "\n";
                    ++num_lines;

                    if (num_lines % 10000 == 0) std::cerr << "\r" << num_lines;
                };
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

            if (num_lines % 10000 == 0) std::cerr << "\r" << num_lines;
        };
        delete obuffer;
    };
    std::cout << std::flush;
    std::cerr << "\r" << num_lines << " Done" << std::endl;

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

    unsigned int num_threads = 4;
    app.add_option(
        "-j,--num-threads", num_threads,
        "Number of threads to use");

    CLI11_PARSE(app, argc, argv);

    // Validate args
    if (num_threads == 0) throw "num_threads cannot be 0";

    auto begin = std::chrono::steady_clock::now();

    unsigned long num_lines = 0;
    if (input == "-") {
        num_lines = run(&std::cin, num_threads);
    } else {
        std::fstream input_stream;
        input_stream.open(input, std::fstream::in);
        num_lines = run(&input_stream, num_threads);
    }

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
