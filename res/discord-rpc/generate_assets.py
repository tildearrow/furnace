"""
Generate Discord Rich Presence assets for Furnace.

Each chip icon is a stylized rendering of the *actual* sound chip used by
that system: correct package style (DIP / SDIP / QFP / BGA), correct pin
count, authentic part-number label, and a brand-themed background tint.
The SVG source files are also written next to the PNGs so the art is
inspectable and re-renderable without this script.

Outputs:
  furnace_logo.png    — 1024×1024, copy of res/logo.png
  chip_<system>.svg   — vector source
  chip_<system>.png   — 512×512 raster, rendered via rsvg-convert

Run from MSYS2 MINGW64 (or any shell where rsvg-convert is on PATH):
  python3 generate_assets.py
"""
from __future__ import annotations
import os, shutil, subprocess, sys
from pathlib import Path
from PIL import Image

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
LOGO_SRC = REPO / "res" / "logo.png"
OUT_DIR = HERE
SIZE = 512
CX, CY = SIZE // 2, SIZE // 2


# --- find rsvg-convert (MSYS2 ships it in /mingw64/bin) ---------------------
RSVG = (
    shutil.which("rsvg-convert")
    or shutil.which("C:/msys64/mingw64/bin/rsvg-convert.exe")
    or "C:/msys64/mingw64/bin/rsvg-convert.exe"
)
if not Path(RSVG).exists() and not shutil.which("rsvg-convert"):
    print("WARNING: rsvg-convert not found on PATH. PNGs will be skipped.")
    RSVG = None


# ---------------------------------------------------------------------------
# SVG primitives — produce snippets, joined into a full doc later.
# ---------------------------------------------------------------------------

def svg_doc(body: str) -> str:
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {SIZE} {SIZE}">\n'
        f'{body}\n'
        f'</svg>\n'
    )


def bg_circle(color: str) -> str:
    return f'<circle cx="{CX}" cy="{CY}" r="{CX}" fill="{color}"/>'


def text(label: str, x: float, y: float, size: int, color: str = "white",
         weight: str = "900") -> str:
    # avoid system-font dependency at render time by listing several bold sans
    fam = "Arial Black, Segoe UI Bold, DejaVu Sans Bold, sans-serif"
    return (
        f'<text x="{x}" y="{y}" font-family="{fam}" font-weight="{weight}" '
        f'font-size="{size}" fill="{color}" text-anchor="middle" '
        f'dominant-baseline="central">{label}</text>'
    )


# ----- DIP horizontal: rectangular body, pins poke out top+bottom edges ----
def dip_horizontal(pin_count: int, body_color: str, pin_color: str,
                   bw: int = 360, bh: int = 200,
                   label: str | None = None, label_size: int = 64,
                   sublabel: str | None = None, sublabel_size: int = 36) -> str:
    per_side = max(1, pin_count // 2)
    x1, x2 = CX - bw // 2, CX + bw // 2
    y1, y2 = CY - bh // 2, CY + bh // 2
    pin_w = max(6, (bw / per_side) * 0.55)
    pin_h = 38
    spacing = bw / per_side
    parts = []
    # pins (draw before body so the body's inner edge can overlap if needed)
    for i in range(per_side):
        px = x1 + spacing * (i + 0.5)
        parts.append(f'<rect x="{px - pin_w/2:.2f}" y="{y1 - pin_h}" '
                     f'width="{pin_w:.2f}" height="{pin_h}" fill="{pin_color}" rx="3"/>')
        parts.append(f'<rect x="{px - pin_w/2:.2f}" y="{y2}" '
                     f'width="{pin_w:.2f}" height="{pin_h}" fill="{pin_color}" rx="3"/>')
    # body
    parts.append(f'<rect x="{x1}" y="{y1}" width="{bw}" height="{bh}" '
                 f'rx="16" fill="{body_color}"/>')
    # pin-1 marker — small lighter dot near top-left of body
    parts.append(f'<circle cx="{x1 + 28}" cy="{y1 + 28}" r="9" fill="#cfd2d8"/>')
    # subtle top stripe (etched line found on many real chips)
    parts.append(f'<rect x="{x1 + 14}" y="{y1 + 14}" width="{bw - 28}" height="3" '
                 f'fill="#000" fill-opacity="0.45"/>')
    # label
    if label:
        ly = CY + (10 if not sublabel else -14)
        parts.append(text(label, CX, ly, label_size))
    if sublabel:
        parts.append(text(sublabel, CX, CY + 38, sublabel_size, "#cbd3df", weight="700"))
    return "\n".join(parts)


# ----- DIP vertical (rotated 90°): pins on left+right edges ----------------
def dip_vertical(pin_count: int, body_color: str, pin_color: str,
                 bw: int = 200, bh: int = 360,
                 label: str | None = None, label_size: int = 56) -> str:
    per_side = max(1, pin_count // 2)
    x1, x2 = CX - bw // 2, CX + bw // 2
    y1, y2 = CY - bh // 2, CY + bh // 2
    pin_h = max(6, (bh / per_side) * 0.55)
    pin_w = 38
    spacing = bh / per_side
    parts = []
    for i in range(per_side):
        py = y1 + spacing * (i + 0.5)
        parts.append(f'<rect x="{x1 - pin_w}" y="{py - pin_h/2:.2f}" '
                     f'width="{pin_w}" height="{pin_h:.2f}" fill="{pin_color}" rx="3"/>')
        parts.append(f'<rect x="{x2}" y="{py - pin_h/2:.2f}" '
                     f'width="{pin_w}" height="{pin_h:.2f}" fill="{pin_color}" rx="3"/>')
    parts.append(f'<rect x="{x1}" y="{y1}" width="{bw}" height="{bh}" '
                 f'rx="16" fill="{body_color}"/>')
    parts.append(f'<circle cx="{x1 + 26}" cy="{y1 + 26}" r="9" fill="#cfd2d8"/>')
    if label:
        parts.append(text(label, CX, CY, label_size))
    return "\n".join(parts)


# ----- QFP: square body, pins on all four sides ---------------------------
def qfp(pin_count: int, body_color: str, pin_color: str,
        bsize: int = 280, label: str | None = None, label_size: int = 56,
        sublabel: str | None = None) -> str:
    per_side = max(1, pin_count // 4)
    bx1 = CX - bsize // 2
    bx2 = CX + bsize // 2
    by1 = CY - bsize // 2
    by2 = CY + bsize // 2
    pin_w = max(3, (bsize / per_side) * 0.45)
    pin_long = 26
    spacing = bsize / per_side
    parts = []
    for i in range(per_side):
        off = spacing * (i + 0.5)
        # top
        parts.append(f'<rect x="{bx1 + off - pin_w/2:.2f}" y="{by1 - pin_long}" '
                     f'width="{pin_w:.2f}" height="{pin_long}" fill="{pin_color}" rx="2"/>')
        # bottom
        parts.append(f'<rect x="{bx1 + off - pin_w/2:.2f}" y="{by2}" '
                     f'width="{pin_w:.2f}" height="{pin_long}" fill="{pin_color}" rx="2"/>')
        # left
        parts.append(f'<rect x="{bx1 - pin_long}" y="{by1 + off - pin_w/2:.2f}" '
                     f'width="{pin_long}" height="{pin_w:.2f}" fill="{pin_color}" rx="2"/>')
        # right
        parts.append(f'<rect x="{bx2}" y="{by1 + off - pin_w/2:.2f}" '
                     f'width="{pin_long}" height="{pin_w:.2f}" fill="{pin_color}" rx="2"/>')
    parts.append(f'<rect x="{bx1}" y="{by1}" width="{bsize}" height="{bsize}" '
                 f'rx="12" fill="{body_color}"/>')
    parts.append(f'<circle cx="{bx1 + 30}" cy="{by1 + 30}" r="10" fill="#cfd2d8"/>')
    if label:
        ly = CY + (10 if not sublabel else -22)
        parts.append(text(label, CX, ly, label_size))
    if sublabel:
        parts.append(text(sublabel, CX, CY + 36, 28, "#cbd3df", weight="700"))
    return "\n".join(parts)


# ----- BGA / flat package: square body, no visible pins, optional ball grid -
def bga(body_color: str, bsize: int = 320, label: str | None = None,
        label_size: int = 60, sublabel: str | None = None, dots: bool = True) -> str:
    bx1 = CX - bsize // 2
    by1 = CY - bsize // 2
    parts = [f'<rect x="{bx1}" y="{by1}" width="{bsize}" height="{bsize}" '
             f'rx="22" fill="{body_color}"/>']
    parts.append(f'<circle cx="{bx1 + 30}" cy="{by1 + 30}" r="11" fill="#cfd2d8"/>')
    if dots:
        step = bsize / 6
        for i in range(1, 6):
            for j in range(1, 6):
                px = bx1 + step * i
                py = by1 + step * j
                parts.append(f'<circle cx="{px}" cy="{py}" r="5" fill="#1a1f25" '
                             f'fill-opacity="0.55"/>')
    if label:
        ly = CY + (10 if not sublabel else -22)
        parts.append(text(label, CX, ly, label_size))
    if sublabel:
        parts.append(text(sublabel, CX, CY + 36, 28, "#cbd3df", weight="700"))
    return "\n".join(parts)


# ----- Genesis combo: a big YM2612 with a smaller SN76489 behind it --------
def genesis_combo(bg_color: str) -> str:
    # main YM2612 (24-pin DIP), centered slightly low and right
    big_bw, big_bh = 320, 180
    sml_bw, sml_bh = 200, 110
    parts = []
    # small chip behind, top-left
    parts.append(_dip_at(CX - 95, CY - 95, sml_bw, sml_bh, 16, "#1a1d22",
                         "#bdc4cf", "SN76489", 24))
    # big chip in front, lower-right
    parts.append(_dip_at(CX + 30, CY + 40, big_bw, big_bh, 24, "#0e1115",
                         "#cfd6e0", "YM2612", 44))
    return "\n".join(parts)


def _dip_at(cx: int, cy: int, bw: int, bh: int, pin_count: int,
            body_color: str, pin_color: str, label: str, label_size: int) -> str:
    per_side = max(1, pin_count // 2)
    x1, x2 = cx - bw // 2, cx + bw // 2
    y1, y2 = cy - bh // 2, cy + bh // 2
    pin_w = max(4, (bw / per_side) * 0.55)
    pin_h = 26
    spacing = bw / per_side
    parts = []
    for i in range(per_side):
        px = x1 + spacing * (i + 0.5)
        parts.append(f'<rect x="{px - pin_w/2:.2f}" y="{y1 - pin_h}" '
                     f'width="{pin_w:.2f}" height="{pin_h}" fill="{pin_color}" rx="2"/>')
        parts.append(f'<rect x="{px - pin_w/2:.2f}" y="{y2}" '
                     f'width="{pin_w:.2f}" height="{pin_h}" fill="{pin_color}" rx="2"/>')
    parts.append(f'<rect x="{x1}" y="{y1}" width="{bw}" height="{bh}" '
                 f'rx="10" fill="{body_color}"/>')
    parts.append(f'<circle cx="{x1 + 18}" cy="{y1 + 18}" r="7" fill="#cfd2d8"/>')
    parts.append(text(label, cx, cy, label_size))
    return "\n".join(parts)


# ---------------------------------------------------------------------------
# Chip table — each entry produces one chip_<key>.svg/png.
# Tuples: (key, bg_color, body fn(bg_color) -> svg body)
# ---------------------------------------------------------------------------

BODY_DARK = "#0e1115"
PIN_LIGHT = "#cfd6e0"


def make(svg_body: str, bg: str) -> str:
    return svg_doc(bg_circle(bg) + "\n" + svg_body)


CHIPS = {
    # Sega ----
    "chip_genesis":   ("#1c2937", lambda: genesis_combo("#1c2937")),
    "chip_sms":       ("#c8202a", lambda: dip_horizontal(16, BODY_DARK, PIN_LIGHT,
                                                          bw=320, bh=180,
                                                          label="SN76489", label_size=48)),
    # Nintendo handhelds ----
    "chip_gameboy":   ("#8bac0f", lambda: qfp(80, BODY_DARK, PIN_LIGHT,
                                                bsize=270, label="DMG-CPU",
                                                label_size=46, sublabel="LR35902")),
    "chip_gba":       ("#582ba5", lambda: bga("#0e1115", bsize=300,
                                                label="AGB-CPU", label_size=48,
                                                sublabel="ARM7TDMI")),
    "chip_vboy":      ("#dc1e1e", lambda: qfp(60, BODY_DARK, PIN_LIGHT,
                                                bsize=270, label="VSU", label_size=80,
                                                sublabel="Virtual Boy")),
    # Nintendo consoles ----
    "chip_nes":       ("#c81b1b", lambda: dip_horizontal(40, BODY_DARK, PIN_LIGHT,
                                                          bw=380, bh=170,
                                                          label="RP2A03", label_size=58)),
    "chip_fds":       ("#f5c83c", lambda: dip_horizontal(40, BODY_DARK, PIN_LIGHT,
                                                          bw=380, bh=170,
                                                          label="2A03+FDS", label_size=46)),
    "chip_snes":      ("#64469b", lambda: qfp(64, BODY_DARK, PIN_LIGHT,
                                                bsize=280, label="S-SMP",
                                                label_size=70, sublabel="SPC700")),
    "chip_n64":       ("#2864af", lambda: bga("#0e1115", bsize=290, dots=True,
                                                label="RCP", label_size=72,
                                                sublabel="Nintendo 64")),
    # NEC ----
    "chip_pce":       ("#e05023", lambda: qfp(64, BODY_DARK, PIN_LIGHT,
                                                bsize=280, label="HuC6280",
                                                label_size=42, sublabel="PC Engine")),
    "chip_pc98":      ("#6e6e6e", lambda: qfp(64, BODY_DARK, PIN_LIGHT,
                                                bsize=280, label="YM2608",
                                                label_size=52, sublabel="OPNA")),
    # Commodore / MOS ----
    "chip_c64":       ("#5a489b", lambda: dip_horizontal(28, BODY_DARK, PIN_LIGHT,
                                                          bw=380, bh=180,
                                                          label="MOS 6581",
                                                          label_size=50,
                                                          sublabel="SID")),
    "chip_amiga":     ("#d75a23", lambda: dip_horizontal(48, BODY_DARK, PIN_LIGHT,
                                                          bw=400, bh=180,
                                                          label="PAULA", label_size=66,
                                                          sublabel="Amiga 8364")),
    # Atari ----
    "chip_tia":       ("#3264c8", lambda: dip_horizontal(40, BODY_DARK, PIN_LIGHT,
                                                          bw=380, bh=170,
                                                          label="TIA", label_size=86,
                                                          sublabel="C010444")),
    "chip_pokey":     ("#3c3c8c", lambda: dip_horizontal(40, BODY_DARK, PIN_LIGHT,
                                                          bw=380, bh=170,
                                                          label="POKEY", label_size=66,
                                                          sublabel="C012294")),
    # General Instrument ----
    "chip_ay":        ("#d2821e", lambda: dip_horizontal(40, BODY_DARK, PIN_LIGHT,
                                                          bw=400, bh=170,
                                                          label="AY-3-8910", label_size=42)),
    # Sinclair / ZX ----
    "chip_zx":        ("#2078dc", lambda: dip_horizontal(40, BODY_DARK, PIN_LIGHT,
                                                          bw=380, bh=170,
                                                          label="ULA",
                                                          label_size=86,
                                                          sublabel="ZX Spectrum")),
    # MSX ----
    "chip_msx":       ("#288c50", lambda: dip_horizontal(40, BODY_DARK, PIN_LIGHT,
                                                          bw=380, bh=170,
                                                          label="SCC", label_size=84,
                                                          sublabel="Konami K051649")),
    # Bandai ----
    "chip_wonderswan":("#2896af", lambda: qfp(100, BODY_DARK, PIN_LIGHT,
                                                bsize=270, label="WS-SoC",
                                                label_size=58, sublabel="WonderSwan")),
    # Yamaha (generic / non-console) ----
    "chip_yamaha":    ("#c8a523", lambda: dip_horizontal(24, BODY_DARK, PIN_LIGHT,
                                                          bw=360, bh=190,
                                                          label="YM2151", label_size=58,
                                                          sublabel="OPM")),
    # Arcade combos ----
    "chip_arcade":    ("#c8288c", lambda: dip_horizontal(24, BODY_DARK, PIN_LIGHT,
                                                          bw=360, bh=190,
                                                          label="YM2151", label_size=58,
                                                          sublabel="+ samples")),
    "chip_neogeo":    ("#dcc823", lambda: qfp(64, BODY_DARK, PIN_LIGHT,
                                                bsize=280, label="YM2610",
                                                label_size=54, sublabel="Neo Geo")),
    # Enterprise ----
    "chip_dave":      ("#64b43c", lambda: dip_horizontal(40, BODY_DARK, PIN_LIGHT,
                                                          bw=380, bh=170,
                                                          label="DAVE", label_size=82,
                                                          sublabel="Enterprise")),
    # Sharp X68000 ----
    "chip_x68000":    ("#aa5096", lambda: dip_horizontal(24, BODY_DARK, PIN_LIGHT,
                                                          bw=360, bh=190,
                                                          label="YM2151", label_size=58,
                                                          sublabel="X68000")),
    # Sega Saturn (placeholder for legacy asset) ----
    "chip_sat":       ("#202078", lambda: qfp(100, BODY_DARK, PIN_LIGHT,
                                                bsize=280, label="SCSP",
                                                label_size=68, sublabel="Saturn")),
    # Generic fallback ----
    "chip_custom":    ("#4b4b55", lambda: dip_horizontal(28, BODY_DARK, PIN_LIGHT,
                                                          bw=360, bh=190,
                                                          label="?", label_size=140)),
}


def rasterize(svg_path: Path, png_path: Path) -> bool:
    if not RSVG:
        return False
    cmd = [str(RSVG), "-w", str(SIZE), "-h", str(SIZE),
           "-o", str(png_path), str(svg_path)]
    try:
        subprocess.run(cmd, check=True, capture_output=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"  ! rsvg-convert failed for {svg_path.name}: {e.stderr.decode()}")
        return False


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    # Furnace logo (large image) - copy verbatim
    logo = Image.open(LOGO_SRC).convert("RGBA")
    if logo.size != (1024, 1024):
        logo = logo.resize((1024, 1024), Image.LANCZOS)
    logo.save(OUT_DIR / "furnace_logo.png", optimize=True)
    print(f"  furnace_logo.png   1024x1024")

    for key, (bg, body_fn) in CHIPS.items():
        svg_text = make(body_fn(), bg)
        svg_path = OUT_DIR / f"{key}.svg"
        png_path = OUT_DIR / f"{key}.png"
        svg_path.write_text(svg_text, encoding="utf-8")
        ok = rasterize(svg_path, png_path)
        print(f"  {key+'.svg':<26} {'+png' if ok else ''}")


if __name__ == "__main__":
    main()
