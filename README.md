# FastTokenizer

**FastTokenizer** is a tokenizer meant to perform language agnostic
tokenization using unicode information.

While the initial goal is to design a tokenizer for the purpose of
machine translation, the same tokenizer is generic enough to be adapted
to a wide range of tasks in NLP due to its' ability to handle a wide
range of languages and writing systems.

Some of the notable features of **FastTokenizer** are
- Providing *just* the right amount of tokenization.
  - Segmentation are designed to be intuitive and rule based.
    The format is ideal for downstream NLP models like subword modelling,
    RNNs or transformers.
  - Also designed to be not so aggressive.
    This way number of tokens can be kept down, allowing model to run faster.
- Works for any and every langauge/writing system.
- Cross programming language.
- Performs format retaining unicode normalization.
- Performance matches or exceeds moses-tokenizer on tasks such as WMT and GLUE.
- Tokenization can be reversed.
  - However custom desegmenter should be used to achieve desired formatting
    as desegmentation is highly use-case driven.

## Comparison with other tokenizers from the web

```
Source:          他的表现遭到《天空体育》评论员内维尔的批评。
Segmenter:       ['他的表现遭到', '《', '天空体育', '》', '评论员内维尔的批评', '。']
Moses:           ['他的表现遭到《天空体育》评论员内维尔的批评。']
Spacy Tokenizer: ['他的表现遭到《天空体育》评论员内维尔的批评。']
Tweet Tokenizer: ['他的表现遭到', '《', '天空体育', '》', '评论员内维尔的批评', '。']
NLTK Tokenizer:  ['他的表现遭到《天空体育》评论员内维尔的批评。'

Source:          AirPods耳機套
Segmenter:       ['AirPods', '耳機套']
Moses:           ['AirPods耳機套']
Spacy Tokenizer: ['AirPods耳機套']
Tweet Tokenizer: ['AirPods耳機套']
NLTK Tokenizer:  ['AirPods耳機套']

Source:          A typical master's programme has a duration of 1-1.5 years.
Segmenter:       ['A', 'typical', "master's", 'programme', 'has', 'a', 'duration', 'of', '1', '@-@', '1.5', 'years', '.']
Moses:           ['A', 'typical', 'master', "'s", 'programme', 'has', 'a', 'duration', 'of', '1', '@-@', '1.5', 'years', '.']
Spacy Tokenizer: ['A', 'typical', "master's", 'programme', 'has', 'a', 'duration', 'of', '1-1.5', 'years.']
Tweet Tokenizer: ['A', 'typical', "master's", 'programme', 'has', 'a', 'duration', 'of', '1-1', '.', '5', 'years', '.']
NLTK Tokenizer:  ['A', 'typical', 'master', "'s", 'programme', 'has', 'a', 'duration', 'of', '1-1.5', 'years', '.']
```

## Installation

### C++

Comming soon.

### Python

```sh
pip install fasttokenizer
```

## Usage

### C++

```cpp
#include <fasttokenizer/segmenter.h>

Segmenter segmenter = Segmenter(args.protected_dash_split);

std::string text = "Hello World!";
std::string output;

// Normalize
output = segmenter.normalize(text)

// Segment
output = segmenter.segment(text)

// Normalize and segment in one function
// Reduce string to icu::UnicodeString overhead
output = segmenter.normalize_and_segment(text);

// Desegment
output = segmenter.desegment(text);
```

### Python

```py
import fasttokenizer

segmenter = fasttokenizer.Segmenter()

text = "Hello World!"

# Normalize
output: str = segmenter.normalize(text)

# Segment
output: str = segmenter.segment()

# Normalize and segment
output: str = segmenter.normalize_and_segment(text)

# Output of segment is str.
# To get tokens, you can split by whitespace.
tokens = output.split()

# Desegment
output: str = segmenter.desegment(text)
```
