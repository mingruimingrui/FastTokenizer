#include <string>
#include <vector>

#include <unicode/regex.h>
#include <unicode/brkiter.h>
#include <unicode/normalizer2.h>

#ifdef TOKENIZER_NAMESPACE
namespace TOKENIZER_NAMESPACE {
#endif

class Segmenter {
    private:
        // Placeholder variables
        UErrorCode icu_status;
        icu::UnicodeString inbuf;
        icu::UnicodeString outbuf;

        // Segmentation tools
        icu::RegexMatcher* word_matcher;
        icu::RegexMatcher* other_letter_matcher;
        const icu::Normalizer2* nfc_normalizer;
        const icu::Normalizer2* nfkc_normalizer;
        icu::BreakIterator* break_iterator;

        // Private functions
        void break_and_append_to_outbuf(int32_t start, int32_t lim);
        void normalize_inbuf();
        void segment_inbuf();

    public:
        Segmenter();
        ~Segmenter();
        Segmenter* clone();

        // Normalize
        void normalize(const std::string& text, std::string& out) {
            inbuf = icu::UnicodeString::fromUTF8(icu::StringPiece(text));
            normalize_inbuf();
            outbuf.toUTF8String(out);
        };

        std::string normalize(const std::string& text) {
            std::string out;
            normalize(text, out);
            return out;
        };

        // Segment
        void segment(const std::string& text, std::string& out) {
            inbuf = icu::UnicodeString::fromUTF8(icu::StringPiece(text));
            segment_inbuf();
            outbuf.toUTF8String(out);
        };

        std::string segment(const std::string& text) {
            std::string out;
            segment(text, out);
            return out;
        };

        // Normalize and segment
        void normalize_and_segment(const std::string& text, std::string& out) {
            inbuf = icu::UnicodeString::fromUTF8(icu::StringPiece(text));
            normalize_inbuf();
            segment_inbuf();
            outbuf.toUTF8String(out);
        };

        std::string normalize_and_segment(const std::string& text) {
            std::string out;
            normalize_and_segment(text, out);
            return out;
        };
};

#ifdef TOKENIZER_NAMESPACE
};
#endif
