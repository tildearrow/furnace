"""
Generate Discord Rich Presence assets for Furnace.

Each chip icon is a colored circle with a per-system motif drawn behind the
label, in a slightly contrasting accent color. The custom fallback uses a
stylized DIP-chip silhouette so it visually fits in the same family.

Outputs into the same folder:
  furnace_logo.png      — 1024×1024, copy of res/logo.png   (large image)
  chip_custom.png       — 512×512,  fallback DIP-chip motif
  chip_<system>.png     — 512×512,  one per supported system
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
LOGO_SRC = REPO / "res" / "logo.png"
OUT_DIR = HERE
SIZE = 512
R = SIZE // 2
CX, CY = R, R


def load_font(px):
    for c in [
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/segoeuib.ttf",
        "/System/Library/Fonts/SFNS.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    ]:
        if Path(c).exists():
            try: return ImageFont.truetype(c, px)
            except Exception: pass
    return ImageFont.load_default()


def base_circle(bg):
    """Solid color circle filling the canvas."""
    img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    ImageDraw.Draw(img).ellipse((0, 0, SIZE - 1, SIZE - 1), fill=bg + (255,))
    return img


def draw_label(img, label, fg=(255, 255, 255), max_frac=0.62):
    d = ImageDraw.Draw(img)
    target = SIZE * max_frac
    for px in (240, 220, 200, 180, 160, 140, 120, 100, 84, 70):
        font = load_font(px)
        bbox = d.textbbox((0, 0), label, font=font)
        w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
        if w <= target and h <= target * 0.95:
            break
    bbox = d.textbbox((0, 0), label, font=font)
    w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
    d.text(((SIZE - w) // 2 - bbox[0], (SIZE - h) // 2 - bbox[1]),
           label, fill=fg + (255,), font=font)


def shade(c, dr=0, dg=0, db=0):
    return (max(0, min(255, c[0] + dr)),
            max(0, min(255, c[1] + dg)),
            max(0, min(255, c[2] + db)))


# ---------- motif painters (each adds a decorative element to img) ----------

def motif_hstripe(img, bg, thick=70):
    """One thick horizontal stripe of slightly lighter color across the middle."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 25, 25, 25) + (255,)
    d.rectangle((0, CY - thick, SIZE, CY + thick), fill=c)


def motif_sega_stripes(img, bg):
    """Three horizontal stripes — classic Sega/SMS divider."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 20, 20, 20) + (255,)
    for i in (-130, 0, 130):
        d.rectangle((0, CY + i - 18, SIZE, CY + i + 18), fill=c)


def motif_dmg_grid(img, bg):
    """DMG screen grid of small dots."""
    d = ImageDraw.Draw(img)
    c = shade(bg, -25, -25, -25) + (255,)
    r0 = R - 70
    step = 60
    for x in range(CX - 4 * step // 2, CX + 4 * step // 2 + 1, step):
        for y in range(CY - 4 * step // 2, CY + 4 * step // 2 + 1, step):
            if (x - CX) ** 2 + (y - CY) ** 2 < r0 ** 2:
                d.ellipse((x - 10, y - 10, x + 10, y + 10), fill=c)


def motif_rounded_inset(img, bg):
    """Lighter rounded rectangle behind text — GBA-style."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 25, 25, 25) + (255,)
    d.rounded_rectangle((90, 170, SIZE - 90, SIZE - 170), radius=60, fill=c)


def motif_nes_bar(img, bg):
    """Black vertical band on the right — NES front-panel reference."""
    d = ImageDraw.Draw(img)
    c = (15, 15, 15, 255)
    d.rectangle((SIZE - 110, 60, SIZE - 60, SIZE - 60), fill=c)


def motif_rings(img, bg):
    """Two concentric rings — disk / CD."""
    d = ImageDraw.Draw(img)
    c = shade(bg, -30, -30, -30) + (255,)
    for r in (R - 40, R - 110):
        d.ellipse((CX - r, CY - r, CX + r, CY + r), outline=c, width=14)


def motif_corner_triangle(img, bg):
    """Big lighter triangle in the upper-left corner."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 25, 25, 25) + (255,)
    d.polygon([(0, 0), (SIZE * 0.55, 0), (0, SIZE * 0.55)], fill=c)


def motif_diag_stripes(img, bg, n=5):
    """Light diagonal stripes."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 20, 20, 20) + (255,)
    w = SIZE // (n * 2)
    for i in range(-n, n * 3):
        x = i * w * 2
        d.polygon([(x, 0), (x + w, 0), (x + w + SIZE, SIZE), (x + SIZE, SIZE)], fill=c)


def motif_rainbow(img, bg):
    """Five small colored bars (Spectrum)."""
    d = ImageDraw.Draw(img)
    cols = [(255, 70, 70), (255, 180, 50), (255, 230, 60),
            (90, 200, 90), (90, 160, 240)]
    bar_w = 60
    bar_h = 32
    total = bar_w * 5 + 8 * 4
    x = (SIZE - total) // 2
    y = SIZE - 100
    for c in cols:
        d.rounded_rectangle((x, y, x + bar_w, y + bar_h), radius=8, fill=c + (255,))
        x += bar_w + 8


def motif_checker(img, bg):
    """2×2 checker — Amiga reference."""
    d = ImageDraw.Draw(img)
    c1 = (255, 255, 255, 255)
    c2 = (0, 0, 0, 255)
    sq = 60
    cells = [(CX - sq, CY - sq, c1), (CX, CY - sq, c2),
             (CX - sq, CY, c2),       (CX, CY, c1)]
    for x, y, c in cells:
        d.rectangle((x, y, x + sq, y + sq), fill=c)


def motif_starburst(img, bg, points=8):
    """Light star rays radiating from center."""
    from math import cos, sin, pi
    d = ImageDraw.Draw(img)
    c = shade(bg, 30, 30, 30) + (255,)
    for i in range(points):
        a = (i / points) * 2 * pi
        x2 = CX + cos(a) * R * 0.95
        y2 = CY + sin(a) * R * 0.95
        # broad ray polygon
        a1 = a + 0.18; a2 = a - 0.18
        p1 = (CX + cos(a1) * R * 0.95, CY + sin(a1) * R * 0.95)
        p2 = (CX + cos(a2) * R * 0.95, CY + sin(a2) * R * 0.95)
        d.polygon([(CX, CY), p1, p2], fill=c)


def motif_dots(img, bg):
    """Small dot grid scattered across."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 30, 30, 30) + (255,)
    for x in range(80, SIZE, 80):
        for y in range(80, SIZE, 80):
            if (x - CX) ** 2 + (y - CY) ** 2 < (R - 30) ** 2:
                d.ellipse((x - 10, y - 10, x + 10, y + 10), fill=c)


def motif_vlines(img, bg, n=3):
    """N vertical accent lines — VBoy / retina display."""
    d = ImageDraw.Draw(img)
    c = (240, 50, 50, 255) if bg[0] < 150 else shade(bg, -40, -40, -40) + (255,)
    spacing = SIZE // (n + 1)
    for i in range(1, n + 1):
        x = spacing * i
        d.rectangle((x - 6, 80, x + 6, SIZE - 80), fill=c)


def motif_tuning_fork(img, bg):
    """Two parallel vertical bars + base — tuning fork (Yamaha)."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 35, 35, 35) + (255,)
    d.rectangle((CX - 90, 90, CX - 50, CY + 60), fill=c)
    d.rectangle((CX + 50, 90, CX + 90, CY + 60), fill=c)
    d.rectangle((CX - 90, CY + 60, CX + 90, CY + 100), fill=c)
    d.rectangle((CX - 30, CY + 100, CX + 30, SIZE - 100), fill=c)


def motif_two_stripes(img, bg):
    """Two thick stripes — top & bottom."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 25, 25, 25) + (255,)
    d.rectangle((0, 80, SIZE, 130), fill=c)
    d.rectangle((0, SIZE - 130, SIZE, SIZE - 80), fill=c)


def motif_cross(img, bg):
    """Plus sign across center."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 25, 25, 25) + (255,)
    d.rectangle((CX - 30, 100, CX + 30, SIZE - 100), fill=c)
    d.rectangle((100, CY - 30, SIZE - 100, CY + 30), fill=c)


def motif_hatching(img, bg):
    """Dense diagonal hatching."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 25, 25, 25) + (255,)
    for off in range(-SIZE, SIZE * 2, 40):
        d.line((off, 0, off + SIZE, SIZE), fill=c, width=8)


def motif_trident(img, bg):
    """Three-pronged spoke — N64 controller analog motif."""
    from math import cos, sin, pi
    d = ImageDraw.Draw(img)
    c = shade(bg, 30, 30, 30) + (255,)
    for i in range(3):
        a = -pi / 2 + i * 2 * pi / 3
        x2 = CX + cos(a) * (R - 70)
        y2 = CY + sin(a) * (R - 70)
        d.line((CX, CY, x2, y2), fill=c, width=30)
    d.ellipse((CX - 50, CY - 50, CX + 50, CY + 50), fill=c)


def motif_oval(img, bg):
    """Elongated horizontal oval — WonderSwan egg."""
    d = ImageDraw.Draw(img)
    c = shade(bg, 25, 25, 25) + (255,)
    d.ellipse((60, 130, SIZE - 60, SIZE - 130), fill=c)


def motif_chip(img, bg):
    """Stylized DIP-chip silhouette: body + pin teeth on both sides."""
    d = ImageDraw.Draw(img)
    body_color = shade(bg, -35, -35, -35) + (255,)
    pin_color  = shade(bg, 35, 35, 35) + (255,)
    body_left, body_top = 110, 150
    body_right, body_bot = SIZE - 110, SIZE - 150
    # pins
    pin_w = 24
    pin_h = 36
    pin_count = 5
    spacing = (body_bot - body_top) // (pin_count + 1)
    for i in range(1, pin_count + 1):
        y = body_top + spacing * i
        d.rectangle((body_left - pin_h, y - pin_w // 2, body_left, y + pin_w // 2), fill=pin_color)
        d.rectangle((body_right, y - pin_w // 2, body_right + pin_h, y + pin_w // 2), fill=pin_color)
    # body
    d.rounded_rectangle((body_left, body_top, body_right, body_bot), radius=20, fill=body_color)
    # notch (indicates pin 1)
    d.ellipse((body_left + 30 - 14, CY - 14, body_left + 30 + 14, CY + 14), fill=bg + (255,))


# ----------------- chip table -----------------------------------------------

# (filename_stem, label, bg color, motif painter)
CHIPS = [
    ("chip_genesis",   "MD",   (28,  28,  28),   motif_hstripe),
    ("chip_sms",       "SMS",  (212, 38,  38),   motif_sega_stripes),
    ("chip_gameboy",   "GB",   (139, 172, 15),   motif_dmg_grid),
    ("chip_gba",       "GBA",  (88,  43,  165),  motif_rounded_inset),
    ("chip_nes",       "NES",  (200, 27,  27),   motif_nes_bar),
    ("chip_fds",       "FDS",  (245, 200, 60),   motif_rings),
    ("chip_pce",       "PCE",  (224, 80,  35),   motif_rings),
    ("chip_snes",      "SNES", (100, 70,  155),  motif_corner_triangle),
    ("chip_c64",       "C64",  (90,  72,  155),  motif_hstripe),
    ("chip_amiga",     "AMI",  (215, 90,  35),   motif_checker),
    ("chip_tia",       "TIA",  (50,  100, 200),  motif_starburst),
    ("chip_pokey",     "POK",  (60,  60,  140),  motif_diag_stripes),
    ("chip_ay",        "AY",   (210, 130, 30),   motif_dots),
    ("chip_zx",        "ZX",   (32,  120, 220),  motif_rainbow),
    ("chip_msx",       "MSX",  (40,  140, 80),   motif_corner_triangle),
    ("chip_wonderswan","WS",   (40,  150, 175),  motif_oval),
    ("chip_yamaha",    "FM",   (200, 165, 35),   motif_tuning_fork),
    ("chip_pc98",      "98",   (110, 110, 110),  motif_two_stripes),
    ("chip_arcade",    "ARC",  (200, 40,  140),  motif_starburst),
    ("chip_neogeo",    "NG",   (220, 200, 35),   motif_two_stripes),
    ("chip_dave",      "EP",   (100, 180, 60),   motif_cross),
    ("chip_vboy",      "VB",   (220, 30,  30),   motif_vlines),
    ("chip_x68000",    "X68",  (170, 80,  150),  motif_hatching),
    ("chip_n64",       "N64",  (40,  100, 175),  motif_trident),
    ("chip_sat",       "SAT",  (32,  32,  120),  motif_dots),
    ("chip_custom",    "?",    (75,  75,  85),   motif_chip),
]


def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    logo = Image.open(LOGO_SRC).convert("RGBA")
    if logo.size != (1024, 1024):
        logo = logo.resize((1024, 1024), Image.LANCZOS)
    logo.save(OUT_DIR / "furnace_logo.png", optimize=True)
    print(f"  furnace_logo.png   1024x1024  (large image)")

    for stem, label, bg, motif in CHIPS:
        img = base_circle(bg)
        motif(img, bg)
        draw_label(img, label)
        img.save(OUT_DIR / f"{stem}.png", optimize=True)
        print(f"  {stem+'.png':<24} {label:>4}  motif={motif.__name__}")


if __name__ == "__main__":
    main()
