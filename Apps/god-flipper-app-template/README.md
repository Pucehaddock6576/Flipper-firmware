# God

A Flipper Zero port of **TempleOS's God** — the random number generator that uses user-timed entropy.

## Concept (from TempleOS)

In TempleOS, "God" is an RNG that mixes:

- **Kernel PRNG** (e.g. LCG + timer).
- **User-timed entropy**: when the system needs bits, it can prompt you to press OK and uses the timestamp of your keypress as entropy. That human timing is fed into a bit FIFO and consumed for "God"-based choices (e.g. random words, Bible verses).

So "God" is literally the RNG: your button press adds entropy; random verses (or words) are chosen from that pool.

## This app

**Launch → 15 words → Back to exit.** No menu or other screens.

On start, the app seeds a 256-bit entropy pool from the HAL RNG and adds one shot of timer entropy, then picks 15 random words from the dict (TempleOS-style). If the pool has enough bits, word indices come from the pool; otherwise the system RNG is used. The words are shown in a scrollable view. Press Back to exit.

## Dict (word list)

The app uses a **unique, sorted word list** derived from the KJV verses (no full text, no duplicates).

- **Path**: bundled via **app assets**. The `dict` folder is set as `fap_file_assets="dict"` in `application.fam`, so `dict/kjv.txt` is unpacked when the FAP is installed.
- **Format**: one word per line, lowercase, sorted, no duplicates (e.g. `a`, `aaron`, `abide`, …).

To regenerate the list from a full KJV file (e.g. `Reference\tVerse text` per line), run:

```bash
python3 scripts/build_word_list.py
```

That script reads `dict/kjv.txt`, keeps only the verse text (after the tab), strips HTML, extracts words, dedupes, sorts, and overwrites `dict/kjv.txt` with one word per line. The repo ships this reduced list so the FAP stays small.

## References

- [TempleOS: God, the Random Number Generator](https://xeiaso.net/blog/templeos-2-god-the-rng-2019-05-30/)
- TempleOS kernel RNG and God/HolySpirit bit FIFO (e.g. `Adam/God/HolySpirit.HC`, `KMathB`)
