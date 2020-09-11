#include <unicode/regex.h>
#include <unicode/brkiter.h>
#include <unicode/normalizer2.h>

#include "fasttokenizer/Segmenter.h"

using namespace icu;

#ifdef TOKENIZER_NAMESPACE
namespace TOKENIZER_NAMESPACE {
#endif

Segmenter::Segmenter()
    : icu_status(U_ZERO_ERROR)
    , word_matcher(new RegexMatcher("[\\w\\s]+", 0, icu_status))
    , other_letter_matcher(new RegexMatcher(
        "(\\p{Lo}[\\p{Lm}\\p{Mn}\\p{Sk}]*)+", 0, icu_status))
    , nfc_normalizer(Normalizer2::getNFCInstance(icu_status))
    , nfkc_normalizer(Normalizer2::getNFKCInstance(icu_status))
    , break_iterator(BreakIterator::createWordInstance(
        Locale::getUS(), icu_status))
{}

Segmenter::~Segmenter() {
    delete word_matcher;
    delete other_letter_matcher;
    delete break_iterator;
}

Segmenter* Segmenter::clone() {
    // Note this function is just a dummy for now.
    // Will be used when segmenter takes states for skipping patterns.
    return new Segmenter();
}

/**
 * Method to apply break_iterator on substring.
 */
void Segmenter::break_and_append_to_outbuf(int32_t start, int32_t lim) {
    int32_t p0, p1;
    break_iterator->setText(inbuf.tempSubString(start, lim));

    p0 = break_iterator->first();
    p1 = break_iterator->next();
    while (p1 != BreakIterator::DONE) {
        // For each segment, trim and if not empty, append to outbuf
        UnicodeString usegment = inbuf.tempSubString(start + p0, p1 - p0);
        usegment.trim();
        if (!usegment.isEmpty()) {
            outbuf.append(usegment);
            outbuf.append(' ');
        };

        p0 = p1;
        p1 = break_iterator->next();
    };
}

/**
 * Normalize input buffer using NFKC for word characters and NFC for others.
 */
void Segmenter::normalize_inbuf() {
    int32_t p0, p1;
    icu_status = U_ZERO_ERROR;

    // Normalize with NFC
    outbuf.remove();
    nfc_normalizer->normalize(inbuf, outbuf, icu_status);
    inbuf = outbuf;

    // Normalize words with NFKC
    outbuf.remove();
    word_matcher->reset(inbuf);
    p0 = 0;
    while (word_matcher->find()) {
        p1 = word_matcher->start(icu_status);
        outbuf.append(inbuf.tempSubString(p0, p1 - p0));
        p0 = p1;

        p1 = word_matcher->end(icu_status);
        inbuf.tempSubString(p0, p1 - p0);
        nfkc_normalizer->normalizeSecondAndAppend(
            outbuf,
            inbuf.tempSubString(p0, p1 - p0),
            icu_status
        );
        p0 = p1;
    };
    outbuf.append(inbuf.tempSubString(p0, inbuf.length() - p0));
    inbuf = outbuf;
};

/**
 * Keep Lo substrings unsegmented and apply break_iterator to others.
 */
void Segmenter::segment_inbuf() {
    int32_t p0, p1;
    icu_status = U_ZERO_ERROR;

    // Segment Lo and apply break_iterator on reamining.
    outbuf.remove();
    other_letter_matcher->reset(inbuf);
    p0 = 0;
    while (other_letter_matcher->find()) {
        // Apply ICU WordBreakITerator on non-Lo substrings
        p1 = other_letter_matcher->start(icu_status);
        break_and_append_to_outbuf(p0, p1 - p0);
        p0 = p1;

        // Do not break Lo
        p1 = other_letter_matcher->end(icu_status);
        outbuf.append(inbuf.tempSubString(p0, p1 - p0));
        outbuf.append(' ');
        p0 = p1;
    };
    // Apply ICU WordBreakITerator on non-Lo substrings
    break_and_append_to_outbuf(p0, inbuf.length() - p0);
    outbuf.trim();

    inbuf = outbuf;
}

#ifdef TOKENIZER_NAMESPACE
}; // namespace
#endif
