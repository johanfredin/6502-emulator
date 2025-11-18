programs = [
    {
        name: "HelloWorld",
        asm: [
            "LDA #$10",
            "STA $0200"
        ],
        description: "Load $10 into A and store at $0200.",
        binary: "A9 10 8D 00 02"
    },
    {
        name: "ABS",
        description: "Load and store using absolute addressing.",
        asm: [
            "LDA #10",
            "STA $0200",
            "LDX $0200",
            "LDY $0200"
        ],
        binary: "A9 0A 8D 00 02 AE 00 02 AC 00 02"
    },
    {
        name: "CPX",
        description: "Increment X until it reaches 10.",
        asm: [
            "loop:",
            "INX",
            "CPX #$0A",
            "BNE loop",
            "TXA"
        ],
        binary: "E8 E0 0A D0 FB 8A"
    },
    {
        name: "JSR and RTS",
        description: "Jump to different subroutines and return from them",
        asm: [
            "JSR init",
            "JSR loop",
            "JSR end",
            "",
            "init:",
            "LDY #$10",
            "RTS",
            "",
            "loop:",
            "INX",
            "CPX #$05",
            "BNE loop",
            "RTS",
            "",
            "end:",
            "LDA #$FF"
        ],
        binary: "20 09 06 20 0C 06 20 12 06 A0 10 60 E8 E0 05 D0 FB 60 A9 FF"
    },
    {
        name: "PHA and PLA",
        description: "Push and pop A to and from the stack",
        asm: [
            "LDX #$00",
            "LDY #$00,",
            "firstloop:",
            "TXA",
            "STA $0200,Y",
            "PHA",
            "INX",
            "INY",
            "CPY #$10",
            "BNE firstloop ;loop until Y is $10",
            "secondloop:",
            "PLA",
            "STA $0200,Y",
            "INY",
            "CPY #$20      ;loop until Y is $20",
            "BNE secondloop"
        ],
        binary: "a2 00 a0 00 8a 99 00 02 48 e8 c8 c0 10 d0 f5 68 99 00 02 c8 c0 20 d0 f7"
    },
    {
        name: "Registers",
        description: "Transfer data between registers",
        asm: [
            "LDA #10",
            "STA $0200",
            "LDX #20",
            "STX $0201",
            "LDY #30",
            "STY $0202",
            "LDA #0",
            "TXA",
            "LDX #0",
            "TYA",
            "LDY #0",
            "TAX",
            "TAY"
        ],
        binary: "A9 0A 8D 00 02 A2 14 8E 01 02 A0 1E 8C 02 02 A9 00 8A A2 00 98 A0 00 AA A8"
    },
    {
        name: "Zero Page",
        description: "Load and store data in the zero page.",
        asm: [
            "LDA #$FE",
            "STA $00",
            "LDX $00",
            "STX $01",
            "LDY $01",
            "STY $02"
        ],
        binary: "A9 FE 85 00 A6 00 86 01 A4 01 84 02"
    }
];
