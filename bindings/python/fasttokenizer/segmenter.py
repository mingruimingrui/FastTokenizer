"""Type hint and docstrings for fasttokenizer::Segmenter."""

import _fasttokenizer


class Segmenter(_fasttokenizer.Segmenter):
    """A universal text segmenter that works for all languages."""

    def normalize(self, text: str) -> str:
        """Normalize an input text.

        Cast all word characters to NFKC format and others to NFC format.
        """
        return super().normalize(text)

    def segment(self, text: str) -> str:
        """Segment a given input text into segments based on unicode info.

        Words are segmented and part-of-words like apostrophe are retained.
        For logographic languages like Chinese and Thai, no segmentation is
        done.

        Dashes are protected using the @ symbol to identify if
        there should be a whitespace preceding or following it.

        Output of this function is whitespace separated tokens.

        eg.
            "Hello World!" -> "Hello World !"

            "It's 2.5-3 miles away." -> "It's 2.5 @-@ 3 miles away."

            "a-b a -b a- b a - b" -> "a @-@ b a -@ b a @- b a - b"
        """
        return super().segment(text)

    def normalize_and_segment(self, text: str) -> str:
        """Perform normalize then segment on an input text."""
        return super().normalize_and_segment(text)

    def desegment(self, text: str) -> str:
        """Desegment a segmented sentence using english rules."""
        return super().desegment(text)
