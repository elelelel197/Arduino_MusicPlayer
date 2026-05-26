from reportlab.lib.pagesizes import A4
from reportlab.pdfgen import canvas

NOTE_CODES = {
    "IDLE": 0,
    "START": 1,
    "END": 2,
    "A2": 3, "A2s": 4, "B2": 5, "C3": 6, "C3s": 7, "D3": 8, "D3s": 9, "E3": 10, "F3": 11, "F3s": 12, "G3": 13, "G3s": 14,
    "A3": 15, "A3s": 16, "B3": 17, "C4": 18, "C4s": 19, "D4": 20, "D4s": 21, "E4": 22, "F4": 23, "F4s": 24, "G4": 25, "G4s": 26,
    "A4": 27, "A4s": 28, "B4": 29, "C5": 30, "C5s": 31
}

STRIPE_WIDTH = 22.5  # mm (2.25 cm)
STRIPE_HEIGHT = 22.5  # mm (2.25 cm)
CLOCK_HEIGHT = STRIPE_HEIGHT / 2  # 11 mm (half of data stripe)
MARGIN = 0         # mm
A4_WIDTH = 210     # mm
A4_HEIGHT = 297    # mm
COLOR_BLACK = 0
COLOR_WHITE = 1
BIT_1_COLOR = COLOR_BLACK
BIT_0_COLOR = COLOR_WHITE

def mm_to_pt(mm):
    return mm * 2.83465

def even_parity(bits):
    return sum(bits) % 2 == 0

def encode_note(note):
    code = NOTE_CODES[note]
    data_bits = [(code >> i) & 1 for i in reversed(range(5))]  # 5 bits
    parity = 0 if even_parity(data_bits) else 1
    return data_bits + [parity]

def draw_vertical_stripes(c, x, y, data_bits):
    for i, bit in enumerate(data_bits):
        # Data stripe (vertical)
        color = BIT_0_COLOR if bit == 0 else BIT_1_COLOR
        c.setFillGray(color)
        c.rect(mm_to_pt(x), mm_to_pt(y + i * STRIPE_HEIGHT), mm_to_pt(STRIPE_WIDTH), mm_to_pt(STRIPE_HEIGHT), fill=1, stroke=0)
        # Clock stripes (two, each half height, in parallel with data)
        # Top half: 0
        c.setFillGray(BIT_0_COLOR)
        c.rect(mm_to_pt(x + STRIPE_WIDTH), mm_to_pt(y + i * STRIPE_HEIGHT), mm_to_pt(STRIPE_WIDTH), mm_to_pt(CLOCK_HEIGHT), fill=1, stroke=0)
        # Bottom half: 1
        c.setFillGray(BIT_1_COLOR)
        c.rect(mm_to_pt(x + STRIPE_WIDTH), mm_to_pt(y + i * STRIPE_HEIGHT + CLOCK_HEIGHT), mm_to_pt(STRIPE_WIDTH), mm_to_pt(CLOCK_HEIGHT), fill=1, stroke=0)

def main(notes, output_pdf="output.pdf"):
    c = canvas.Canvas(output_pdf, pagesize=A4)
    word_height = STRIPE_HEIGHT * 6  # 6 bits per note
    col_width = STRIPE_WIDTH * 2     # data + clock
    x, y = MARGIN, MARGIN
    max_words_per_col = int((A4_HEIGHT - 2 * MARGIN) // word_height)

    word_count = 0
    col_count = 0

    for note in notes:
        # If we need a new column
        if word_count >= max_words_per_col:
            word_count = 0
            col_count += 1
            x = MARGIN + col_count * col_width
            y = MARGIN
        # If the next column would go out of bounds, start a new page
        if x + col_width > A4_WIDTH - MARGIN:
            c.showPage()
            x, y = MARGIN, MARGIN
            col_count = 0
            word_count = 0
        draw_vertical_stripes(c, x, y, encode_note(note))
        word_count += 1
        y += word_height
        print(f"Encoded note '{note}' at column {col_count}, word {word_count}, position ({x}mm, {y}mm)")

    c.save()
    print(f"PDF saved as {output_pdf}")

if __name__ == "__main__":
    notes = ["START", "A2", "B2", "C3", "D3", "E3", "F3", "G3", "END"]
    main(notes)