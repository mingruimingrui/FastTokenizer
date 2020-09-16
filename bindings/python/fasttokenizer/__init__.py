"""Type hint and docstrings for fasttokenizer."""

from fasttokenizer import _fasttokenizer

__all__ = ['Segmenter']
__version__ = _fasttokenizer.__version__


class Segmenter(_fasttokenizer.Segmenter):
    """A universal text segmenter that works for all languages.

    Args:
        protected_dash_split (bool, optional): Protect dashes by annotating
            if there should be a space preceding or following it.
            Defaults to False.
    """

    def __init__(self, protected_dash_split=False):
        super().__init__(protected_dash_split)

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
