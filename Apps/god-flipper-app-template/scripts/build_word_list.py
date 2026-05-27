#!/usr/bin/env python3
"""
Build a unique, sorted word list from dict/kjv.txt (verse text only).
Reads: "Reference\tVerse text..."
Outputs: one word per line, lowercase, sorted, no duplicates.
Overwrites dict/kjv.txt with the new content.
"""
import re
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DICT_PATH = os.path.join(SCRIPT_DIR, "..", "dict", "kjv.txt")

def main():
    with open(DICT_PATH, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()

    words = set()
    # Skip header (first line)
    for line in lines[1:]:
        line = line.strip()
        if not line:
            continue
        # Format: "Reference\tVerse text..."
        idx = line.find("\t")
        if idx == -1:
            continue
        text = line[idx + 1:]
        # Strip <i> and </i>
        text = re.sub(r"</?i>", "", text, flags=re.IGNORECASE)
        # Extract words (letters only), lowercase
        for word in re.findall(r"[a-zA-Z]+", text):
            if word:
                words.add(word.lower())

    sorted_words = sorted(words)
    with open(DICT_PATH, "w", encoding="utf-8") as f:
        f.write("\n".join(sorted_words))
        f.write("\n")

    print(f"Wrote {len(sorted_words)} unique words to {DICT_PATH}")

if __name__ == "__main__":
    main()
