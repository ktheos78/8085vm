program = [             # start:
    0x3A, 0x77, 0x77,   # lda 0x7777 
    0x06, 0x02,         # mvi b, 0x02
    0x16, 0x04,         # mvi d, 0x04
    0x0C,               # inr c
    0xC2, 0x00, 0x09,   # jmp start
    0x76                # hlt
]

with open("prog.bin", "wb") as f:
    f.write(bytes(program))

print("Wrote prog.bin with", len(program), "bytes")