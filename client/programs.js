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
    },
    {
        name: "ADC",
        description: "Addition with different addressing modes, endless loop",
        asm: [
            "*=$600",
            "start:",
            "LDX #$20",
            "LDY #$30",
            "STX $01",
            "STX $00",
            "STY $02",
            "LDX #$01",
            "LDY #$02",
            "CLC",
            "addition_imm:",
            "ADC #$80",
            "BCC addition_imm",
            "CLC",
            "LDA #$0",
            "addition_zp:",
            "ADC $00",
            "BCC addition_zp",
            "CLC",
            "LDA #$0",
            "addition_zpx:",
            "ADC $00,X",
            "BCC addition_zpx",
            "CLC",
            "LDA #$0",
            "LDA #$32",
            "STA $200",
            "LDA #$0",
            "addition_abs:",
            "ADC $200",
            "BCC addition_abs",
            "CLC",
            "LDA #$15",
            "STA $201",
            "LDA #$0",
            "addition_abx:",
            "ADC $200,X",
            "BCC addition_abx",
            "CLC",
            "SEC",
            "BCS start"
        ],
        binary: "A2 20 A0 30 86 01 86 00 " +
            "84 02 A2 01 A0 02 18 69 " +
            "80 90 FC 18 A9 00 65 00 " +
            "90 FC 18 A9 00 75 00 90 " +
            "FC 18 A9 00 A9 32 8D 00 " +
            "02 A9 00 6D 00 02 90 FB " +
            "18 A9 15 8D 01 02 A9 00 " +
            "7D 00 02 90 FB 18 38 B0 " +
            "BF"
    },
    {
        name: "SBC",
        description: "Subract from A until 0",
        asm: ["WIP"],
        binary: "A9 0F A2 01 86 00 86 01 " +
            "8E 00 02 8E 01 02 A0 02 " +
            "8C 02 02 38 E9 01 E5 00 " +
            "F5 00 ED 00 02 FD 00 02 " +
            "F9 00 02 C9 00 F0 EC A9 " +
            "FF"
    },
    {
        name: "INC",
        description: "Increment address $0 and put in X until X = 10",
        asm: [
            "LDX #$0",
            "loop:",
            "STX $0",
            "INC $0",
            "LDX $0",
            "CPX #$0A",
            "BNE loop"
        ],
        binary: "a2 00 86 00 e6 00 a6 00 e0 0a d0 f6"
    },
    {
        name: "Comprehensive Test",
        description: "Tests all implemented instructions: register transfers, loads, stores, increments, arithmetic, bitwise operations, shifts, comparisons, BIT test, branching, stack operations, and subroutines.",
        asm: [
            "LDA #$05",
            "LDX #$00",
            "LDY #$03",
            "TXA",
            "TAX",
            "TAY",
            "TYA",
            "LDA #$42",
            "STA $0700",
            "LDX #$05",
            "STX $0701",
            "LDY #$03",
            "STY $0702",
            "LDA #$42",
            "LDA $0700",
            "LDX $0701",
            "LDA #$00",
            "STA $0703",
            "INC $0703",
            "INX",
            "INY",
            "LDA #$50",
            "ADC #$30",
            "CLC",
            "ADC $0703",
            "LDA #$FF",
            "AND #$0F",
            "LDA #$01",
            "ASL",
            "ASL",
            "ASL",
            "LDA #$05",
            "CMP #$05",
            "LDX #$03",
            "CPX #$03",
            "LDY #$07",
            "CPY #$07",
            "LDA #$C0",
            "STA $0704",
            "BIT $0704",
            "LDA #$00",
            "LDX #$00",
            "INX",
            "CPX #$0A",
            "BNE $0632",
            "LDA #$42",
            "PHA",
            "LDA #$00",
            "PLA",
            "JSR $063E",
            "NOP",
            "LDA #$AA",
            "RTS"
        ],
        binary: "A9 05 A2 00 A0 03 8A AA " +
            "A8 98 A9 42 8D 00 07 A2 " +
            "05 8E 01 07 A0 03 8C 02 " +
            "07 A9 42 AD 00 07 AE 01 " +
            "07 A9 00 8D 03 07 EE 03 " +
            "07 E8 C8 A9 50 69 30 18 " +
            "6D 03 07 A9 FF 29 0F A9 " +
            "01 0A 0A 0A A9 05 C9 05 " +
            "A2 03 E0 03 A0 07 C0 07 " +
            "A9 C0 8D 04 07 2C 04 07 " +
            "A9 00 A2 00 E8 E0 0A D0 " +
            "FB A9 42 48 A9 00 68 20 " +
            "63 06 EA A9 AA 60",
        expectedResult: {
            registers: {
                A: 0xAA,
                X: 0x0A,
                Y: 0x07
            },
            memory: {
                "$0700": 0x42,
                "$0701": 0x05,
                "$0702": 0x04,
                "$0704": 0xC0
            },
            flags: {
                carry: true,
                zero: false,
                interrupt: false,
                decimal: false,
                overflow: true,
                negative: true
            },
            description: "After execution: A=0xAA (from subroutine), X=0x0A (from loop), Y=0x07 (transferred), loop completes after 10 iterations, all data stored to page $0700, BIT instruction sets overflow and negative flags correctly, JSR/RTS returns properly."
            }
        }
];
