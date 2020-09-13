#include <unicode/regex.h>
#include <unicode/brkiter.h>
#include <unicode/normalizer2.h>
#include <iostream>

#include "fasttokenizer/segmenter.h"

using namespace icu;

#ifdef TOKENIZER_NAMESPACE
namespace TOKENIZER_NAMESPACE {
#endif

Segmenter::Segmenter()
    : icu_status(U_ZERO_ERROR)

    , non_whitespace_matcher(new RegexMatcher("\\S+", 0, icu_status))
    , other_letter_matcher(new RegexMatcher(
        "(\\p{Lo}[\\p{Lm}\\p{Mn}\\p{Sk}]*)+", 0, icu_status))
    , word_and_space_matcher(new RegexMatcher("[\\w\\s]+", 0, icu_status))

    , left_shift_chars(new UnicodeSet(
        UnicodeString("[[:Pf:][:Pe:][,.?!:;%]]"), icu_status))
    , right_shift_chars(new UnicodeSet(
        UnicodeString("[[:Sc:][:Pi:][:Ps:][¿¡]]"), icu_status))
    , both_shift_chars(new UnicodeSet(
        UnicodeString("[|/\\\\]"), icu_status))
    , numeric_chars(new UnicodeSet(
        UnicodeString("[:N:]"), icu_status))
    , whitespace_chars(new UnicodeSet(
        UnicodeString("[:Z:]"), icu_status))

    , nfc_normalizer(Normalizer2::getNFCInstance(icu_status))
    , nfkc_normalizer(Normalizer2::getNFKCInstance(icu_status))

    , break_iterator(BreakIterator::createWordInstance(
        Locale::getUS(), icu_status))
{
    left_shift_chars->freeze();
    right_shift_chars->freeze();
    both_shift_chars->freeze();
    numeric_chars->freeze();
    whitespace_chars->freeze();
}

Segmenter::~Segmenter() {
    delete non_whitespace_matcher;
    delete other_letter_matcher;
    delete word_and_space_matcher;

    delete left_shift_chars;
    delete right_shift_chars;
    delete both_shift_chars;
    delete numeric_chars;
    delete whitespace_chars;

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

        if (whitespace_chars->contains(usegment[0])) {
            // pass

        } else if (usegment == dash_string) {
            char16_t prev_char = inbuf[start + p0 - 1];
            char16_t next_char = inbuf[start + p1];

            if (!whitespace_chars->contains(prev_char)) {
                if (prev_char != 65535) {  // 2 ** 16 - 1
                    outbuf.append('@');
                };
            };
            outbuf.append(usegment);
            if (!whitespace_chars->contains(next_char)) {
                if (next_char != 65535) {  // 2 ** 16 - 1
                    outbuf.append('@');
                };
            };
            outbuf.append(' ');

        } else {
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
    outbuf.remove();

    // Normalize with NFC
    nfc_normalizer->normalize(inbuf, outbuf, icu_status);
    inbuf = outbuf;

    // Normalize words with NFKC
    outbuf.remove();
    word_and_space_matcher->reset(inbuf);
    p0 = 0;
    while (word_and_space_matcher->find()) {
        p1 = word_and_space_matcher->start(icu_status);
        outbuf.append(inbuf.tempSubString(p0, p1 - p0));
        p0 = p1;

        p1 = word_and_space_matcher->end(icu_status);
        inbuf.tempSubString(p0, p1 - p0);
        nfkc_normalizer->normalizeSecondAndAppend(
            outbuf,
            inbuf.tempSubString(p0, p1 - p0),
            icu_status
        );
        p0 = p1;
    };
    outbuf.append(inbuf.tempSubString(p0, inbuf.length() - p0));
};

/**
 * Keep Lo substrings unsegmented and apply break_iterator to others.
 */
void Segmenter::segment_inbuf() {
    int32_t p0, p1;
    icu_status = U_ZERO_ERROR;
    outbuf.remove();

    // Segment Lo and apply break_iterator on reamining.
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
};

void Segmenter::desegment_inbuf() {
    int32_t p0, p1;
    icu_status = U_ZERO_ERROR;
    outbuf.remove();
    bool in_apos = false;
    bool in_quote = false;
    bool prepend_space = true;
    UnicodeString prev_usegment;

    non_whitespace_matcher->reset(inbuf);
    while (non_whitespace_matcher->find()) {
        p0 = non_whitespace_matcher->start(icu_status);
        p1 = non_whitespace_matcher->end(icu_status);

        // Get word
        UnicodeString usegment = inbuf.tempSubString(p0, p1 - p0);

        if (right_shift_chars->contains(usegment)) {
            if (prepend_space) outbuf.append(' ');
            outbuf.append(usegment);
            prepend_space = false;

        } else if (left_shift_chars->contains(usegment)) {
            outbuf.append(usegment);
            prepend_space = true;

        } else if (both_shift_chars->contains(usegment)) {
            outbuf.append(usegment);
            prepend_space = false;

        } else if (usegment == "@-@") {
            outbuf.append('-');
            prepend_space = false;

        } else if (usegment == "-@") {
            if (prepend_space) outbuf.append(' ');
            outbuf.append('-');
            prepend_space = false;

        } else if (usegment == "@-") {
            outbuf.append('-');
            prepend_space = true;

        } else if (usegment == '\'') {
            if (prev_usegment.endsWith('s')) {
                outbuf.append(usegment);
                prepend_space = true;
            } else if (in_apos) {
                outbuf.append(usegment);
                prepend_space = true;
                in_apos = false;
            } else {
                if (prepend_space) outbuf.append(' ');
                outbuf.append(usegment);
                prepend_space = false;
                in_apos = true;
            };

        } else if (usegment == '\"') {
            char16_t prev_last_char =
                prev_usegment[prev_usegment.length() - 1];
            if (numeric_chars->contains(prev_last_char)) {
                outbuf.append(usegment);
                prepend_space = true;
            } else if (in_quote) {
                outbuf.append(usegment);
                prepend_space = true;
                in_quote = false;
            } else {
                if (prepend_space) outbuf.append(' ');
                outbuf.append(usegment);
                prepend_space = false;
                in_quote = true;
            }

        } else {
            if (prepend_space) outbuf.append(' ');
            outbuf.append(usegment);
            prepend_space = true;
        };

        prev_usegment = usegment;
    };

    outbuf.trim();
};

#ifdef TOKENIZER_NAMESPACE
}; // namespace
#endif
