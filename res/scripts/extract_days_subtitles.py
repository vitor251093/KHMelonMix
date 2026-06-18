#!/usr/bin/env python3
"""
Extract ALL Kingdom Hearts 358/2 Days HD-cutscene subtitles from a KH 1.5+2.5 ReMIX
install and emit them as standard per-language SubRip (.srt) files for KHMelonMix (issue #199).

Every HD-movie cutscene (the `hd8xx` set) is extracted, whether or not we currently have a
matching video, because all HD cutscenes are planned for eventual use. Anything that can't be
cleanly matched/resolved is FLAGGED in the run report (printed to console and written to
`<out>/EXTRACTION_REPORT.txt`) but still extracted. SubRip has no comment syntax, so these
warnings live in the report rather than inside the .srt files.

Sources inside `Image/Mare.pkg` (indexed by `Image/Mare.hed`), resolved by name via OpenKH's
filename list (fetched on demand; names are MD5(utf8(path))):

  * `.exia2` event scripts (`uk/event/en/hd/hdN/hdN.exia2`) — cue *timing* (FrameStart/FrameEnd)
    and a `TextID` per cue, in per-language `SCHEDULE_CATEGORY_TEXT_MOVIE` blocks (the English
    build embeds all six languages).
  * `.ctd` message binaries (`<region>/message/<lang>/event/hd/bin/ctXXX.ctd`) — the game's real
    on-screen text (UTF-16LE), the source of truth: proper accents for the European languages and
    correct text where the `.exia2` inline `Text` is wrong (e.g. hd848 inline EN is Japanese).
    `@CTD` header, 8-byte (u32 key, u32 value) table at 0x20, value&0xFFFF = string offset.

Linking: a cue's `.exia2` `TextID` is the `.ctd` key in hex, so `ctd_key = int(TextID, 16)`.
When a cue has no `TextID` or it isn't in the CTD (some edited `_mm` cutscenes), we fall back to
the `.exia2` inline `Text` (accent-stripped/approximate) and flag it.

Frame -> time: cue frames index the movie's timeline. The HD re-encodes run at ~30 fps, so we use
`ms = frame * 1000 / fps` with the fps probed from the matched video, or an assumed 30 fps when no
video exists yet (flagged). (The `.exia2` `<FRAME_RATE>` reads 60 but is nominal.)

Output: assets/days/subtitles/<Language>/cinematics/<ID>.srt  (UTF-8 SubRip: numbered cues with
"HH:MM:SS,mmm --> HH:MM:SS,mmm" timing and real line breaks) plus a run report at
assets/days/subtitles/EXTRACTION_REPORT.txt.

Requires: python3 (stdlib) and ffprobe on PATH. Network access on first run to fetch the OpenKH
filename list (or pass --names to a local copy to run fully offline).

Provenance: the filename list is OpenKH's `OpenKh.Egs/resources/mare.txt` (Apache-2.0), fetched
on demand from a pinned commit rather than vendored here.
"""

import argparse
import hashlib
import html
import re
import struct
import subprocess
import urllib.request
from pathlib import Path

ASSUMED_FPS = 30.0  # used when no video exists to probe (HD re-encodes are 30 fps)

# OpenKH's Mare filename list, pinned to a commit (the .hed stores MD5(utf8(path)); this maps them
# back to names). Fetched on demand so we don't vendor third-party data; override with --names.
MARE_LIST_URL = ("https://raw.githubusercontent.com/OpenKH/OpenKh/"
                 "b4a73979d8f294d4464999a1f82d895984e85eae/OpenKh.Egs/resources/mare.txt")

# LANGUAGE_xx (as used in the .exia2) -> (ctd path prefix in the archive, output folder endonym).
LANGUAGES = {
    "LANGUAGE_JP": ("jp/message/jp", "jp"),
    "LANGUAGE_EN": ("uk/message/en", "en"),
    "LANGUAGE_GE": ("gr/message/de", "de"),
    "LANGUAGE_FR": ("fr/message/fr", "fr"),
    "LANGUAGE_IT": ("it/message/it", "it"),
    "LANGUAGE_SP": ("sp/message/sp", "es"),
}

EXIA2_INVENTORY = re.compile(r"uk/event/en/hd/(hd8[0-9a-z]+)/\1\.exia2")
TEXT_MOVIE_BLOCK = re.compile(
    r'<SCHEDULE_CATEGORY\s+type\s*=\s*"SCHEDULE_CATEGORY_TEXT_MOVIE">(.*?)</SCHEDULE_CATEGORY>',
    re.DOTALL,
)
BLOCK_LANG = re.compile(r'Language="(LANGUAGE_\w+)"')
# A cue. TextID (hex) is optional; Text (inline, UTF-8) is the fallback when there is no CTD match.
CUE = re.compile(
    r'<DATA\s+FrameStart="(\d+)"\s+FrameEnd="(\d+)"'
    r'(?:[^>]*?\bTextID="([0-9A-Fa-f]*)")?'
    r'(?:[^>]*?\bText="(.*?)")?\s*/>',
    re.DOTALL,
)
CTD_NAME = re.compile(r'bin[\\/](ct\w+\.ctd)')


class Archive:
    """Resolves named entries in Mare.pkg via Mare.hed + the OpenKH filename list."""

    def __init__(self, collection, names):
        image = Path(collection) / "Image"
        hed = (image / "Mare.hed").read_bytes()
        self.index = {}
        for i in range(len(hed) // 32):
            r = hed[i * 32 : i * 32 + 32]
            self.index[r[:16]] = (struct.unpack("<Q", r[16:24])[0], struct.unpack("<I", r[28:32])[0])
        self.pkg = open(image / "Mare.pkg", "rb")
        self.names = names
        self.known = set(names)
        self._ctd_cache = {}

    def read(self, path):
        if path not in self.known:
            return None
        key = hashlib.md5(path.encode("utf-8")).digest()
        if key not in self.index:
            return None
        offset, length = self.index[key]
        self.pkg.seek(offset + 16)  # 16-byte per-file header
        return self.pkg.read(length)

    def ctd(self, path):
        """Parse a .ctd into {key:int -> text}, cached."""
        if path in self._ctd_cache:
            return self._ctd_cache[path]
        out = {}
        b = self.read(path)
        if b and b[:4] == b"@CTD":
            str_start = struct.unpack("<I", b[0x18:0x1C])[0]
            off = 0x20
            while off + 8 <= str_start:
                key, val = struct.unpack("<II", b[off : off + 8])
                off += 8
                soff = val & 0xFFFF
                if soff == 0:
                    continue
                end = soff
                while end + 1 < len(b) and b[end : end + 2] != b"\x00\x00":
                    end += 2
                out[key] = b[soff:end].decode("utf-16-le", "replace")
        self._ctd_cache[path] = out
        return out

    def exia2_ids(self):
        """All hd8xx cutscene ids present in the (English build) filename list, sorted."""
        ids = {m.group(1) for n in self.names for m in [EXIA2_INVENTORY.fullmatch(n)] if m}
        return sorted(ids)


def clean_text(text):
    """Normalize text for SubRip: keep real line breaks (SRT puts each on its own line), drop KH
    control markers and stray control characters, collapse tabs to spaces."""
    text = text.replace("｜", "").replace("\t", " ")
    text = "".join(c for c in text if c in "\r\n" or ord(c) >= 0x20)
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    return text.strip()


def ffprobe(mp4_path):
    """Return (fps, duration_s), or (None, None) on failure."""
    def q(entries, stream=False):
        cmd = ["ffprobe", "-v", "error"]
        if stream:
            cmd += ["-select_streams", "v:0"]
        cmd += ["-show_entries", entries, "-of", "default=noprint_wrappers=1:nokey=1", str(mp4_path)]
        return subprocess.run(cmd, capture_output=True, text=True).stdout.strip().splitlines()
    try:
        num, den = q("stream=r_frame_rate", stream=True)[0].split("/")
        return float(num) / float(den), float(q("format=duration")[0])
    except Exception:
        return None, None


def block_cues(block, ctd, fps):
    """Return (cues, inline_fallback_count). Prefer CTD text via TextID, else inline Text."""
    cues, inline = [], 0
    for fs, fe, tid, inline_text in CUE.findall(block):
        text = ctd.get(int(tid, 16)) if tid else None
        if text is None and inline_text:
            text = html.unescape(inline_text)
            inline += 1
        if not text:
            continue
        text = clean_text(text)
        if not text:
            continue
        cues.append((round(int(fs) * 1000 / fps), round(int(fe) * 1000 / fps), text))
    cues.sort(key=lambda c: c[0])
    return cues, inline


def load_filename_list(names_arg):
    """Return the OpenKH Mare filename list. Uses a local --names file if given, else fetches the
    pinned copy from OpenKH (the list is only needed to map .hed MD5 names back to paths)."""
    if names_arg:
        text = Path(names_arg).read_text(encoding="utf-8")
    else:
        print(f"fetching OpenKH filename list: {MARE_LIST_URL}")
        with urllib.request.urlopen(MARE_LIST_URL) as r:
            text = r.read().decode("utf-8")
    return [l.strip() for l in text.splitlines() if l.strip()]


def ms_to_srt(ms):
    """Format milliseconds as a SubRip timestamp 'HH:MM:SS,mmm'."""
    ms = max(0, int(ms))
    h, ms = divmod(ms, 3600000)
    m, ms = divmod(ms, 60000)
    s, ms = divmod(ms, 1000)
    return f"{h:02d}:{m:02d}:{s:02d},{ms:03d}"


def write_srt(out_dir, lang_folder, cutscene_id, cues):
    """Write cues as a standard SubRip (.srt) file. Warnings are not embedded (SRT has no comment
    syntax); they go in the run report instead."""
    dest = Path(out_dir) / lang_folder / "cinematics" / f"{cutscene_id}.srt"
    dest.parent.mkdir(parents=True, exist_ok=True)
    blocks = []
    for i, (s, e, t) in enumerate(cues, start=1):
        blocks.append(f"{i}\n{ms_to_srt(s)} --> {ms_to_srt(e)}\n{t}\n")
    dest.write_text("\n".join(blocks), encoding="utf-8")


def main():
    ap = argparse.ArgumentParser(description="Extract ALL Days HD-cutscene subtitles to .srt files.")
    ap.add_argument("--collection", required=True,
                    help="KH 1.5+2.5 ReMIX install dir (containing Image/Mare.hed and Image/Mare.pkg)")
    ap.add_argument("--mp4-dir", default="assets/days/cutscenes/cinematics",
                    help="Directory of committed hd{ID}.mp4 cutscenes (for fps/duration)")
    ap.add_argument("--out", default="assets/days/subtitles", help="Output root for .srt files")
    ap.add_argument("--names", default=None,
                    help="Local copy of OpenKH's mare.txt filename list (default: fetch it online)")
    args = ap.parse_args()

    archive = Archive(args.collection, load_filename_list(args.names))
    ids = archive.exia2_ids()
    mp4_dir = Path(args.mp4_dir)

    # Mirror every report line to console and to EXTRACTION_REPORT.txt so the two stay identical.
    report = []
    def emit(line=""):
        print(line)
        report.append(line)

    emit(f"{'id':8}{'video':6}{'fps':>5}{'src':>8}{'cues(EN)':>9}{'lastCue/dur':>13}  flags")
    written = 0
    flagged = []  # (id, message, [warnings])
    for vid in ids:
        mp4 = mp4_dir / f"{vid}.mp4"
        fps, dur = ffprobe(mp4) if mp4.exists() else (None, None)
        have_video = fps is not None
        if fps is None:
            fps, dur = ASSUMED_FPS, None

        xml = archive.read(f"uk/event/en/hd/{vid}/{vid}.exia2").decode("utf-8", "replace")
        m = CTD_NAME.search(xml)
        ctd_name = m.group(1) if m else "cthd800.ctd"

        # First pass: collect cues per language so we can compute the timing ratio (and the
        # per-cutscene warnings for the report) before writing any file.
        results = []  # (folder, lang, cues, inline)
        en_count = en_last = total_inline = max_last = 0
        for block in TEXT_MOVIE_BLOCK.findall(xml):
            lang_m = BLOCK_LANG.search(block)
            if not lang_m or lang_m.group(1) not in LANGUAGES:
                continue
            lang = lang_m.group(1)
            prefix, folder = LANGUAGES[lang]
            cues, inline = block_cues(block, archive.ctd(f"{prefix}/event/hd/bin/{ctd_name}"), fps)
            if not cues:
                continue
            results.append((folder, lang, cues, inline))
            total_inline += inline
            max_last = max(max_last, cues[-1][1])
            if lang == "LANGUAGE_EN":
                en_count, en_last = len(cues), cues[-1][1]

        ratio = (max_last / 1000.0 / dur) if (results and dur) else 0.0
        overruns = ratio > 1.05

        for folder, lang, cues, inline in results:
            write_srt(args.out, folder, vid[2:] if vid.startswith("hd") else vid, cues)
            written += 1

        # Per-cutscene warnings, surfaced in the report (SubRip can't hold comments).
        warnings = []
        if not have_video:
            warnings.append(f"no matching video yet; timing assumes {ASSUMED_FPS:g} fps")
        if total_inline:
            warnings.append(f"{total_inline} cue(s) use inline .exia2 text (no CTD TextID; may be approximate)")
        if overruns:
            warnings.append("cue timing overruns the current video (likely an edited '_mm' re-cut); needs manual retiming")

        src = "inline" if total_inline else "CTD"
        flags = []
        if not have_video:
            flags.append("NO VIDEO")
        if total_inline:
            flags.append("inline fallback")
        if ratio > 1.05:
            flags.append("cues exceed video (re-cut?)")
        if en_count == 0 and total_inline == 0:
            flags.append("no dialogue cues")
        if set(flags) - {"no dialogue cues"}:
            flagged.append((vid, ", ".join(flags), warnings))
        ratio_s = f"{ratio:.2f}" if dur else "n/a"
        emit(f"{vid:8}{('yes' if have_video else 'NO'):6}{fps:5.0f}{src:>8}{en_count:9}{ratio_s:>13}  {', '.join(flags)}")

    # Videos we have but with no subtitle source at all.
    have_ids = {p.stem for p in mp4_dir.glob('hd*.mp4')}
    for vid in sorted(have_ids - set(ids)):
        emit(f"{vid:8}{'yes':6}{'-':>5}{'-':>8}{'-':>9}{'-':>13}  NO .exia2 SOURCE")
        flagged.append((vid, "video present but no .exia2 subtitle source", []))

    emit(f"\nwrote {written} .srt files under {args.out} for {len(ids)} cutscenes")
    if flagged:
        emit("\nFLAGGED (extracted where possible, needs human attention):")
        for vid, msg, warnings in flagged:
            emit(f"  {vid}: {msg}")
            for w in warnings:
                emit(f"      - {w}")

    report_path = Path(args.out) / "EXTRACTION_REPORT.txt"
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text("\n".join(report) + "\n", encoding="utf-8")
    print(f"\nwrote run report to {report_path}")


if __name__ == "__main__":
    main()
