document.addEventListener('alpine:init', () => {
    Alpine.data('emulator', () => ({
        cpu: {},
        memoryPage: 0x00,
        stackPage: 0x01,
        pageData: [],
        stackData: [],
        disassembly: [],
        pcToLineIndex: {},
        loadedProgramName: null,

        // status bitmasks
        FLAG_C: (1 << 0),
        FLAG_Z: (1 << 1),
        FLAG_I: (1 << 2),
        FLAG_D: (1 << 3),
        FLAG_B: (1 << 4),
        FLAG_U: (1 << 5),
        FLAG_V: (1 << 6),
        FLAG_N: (1 << 7),

        // Called on startup by alpine
        async init() {
            await this.getCpuState();
            await this.loadPage(this.memoryPage);
        },

        isFlagSet(flag) {
            return (this.cpu.status & flag) ? 1 : 0;
        },

        isSpPosition(index) {
            // SP points to 0x01XX, so the offset within the stack page is the low byte
            const spOffset = this.cpu.sp & 0xFF;
            return index === spOffset;
        },

        hex(value, size = 2) {
            if (!value) {
                value = 0;
            }
            return value.toString(16).toUpperCase().padStart(size, '0');
        },

        getProgramDescription() {
            if (!this.loadedProgramName) {
                return "No program loaded";
            }
            return programs.find(p => p.name === this.loadedProgramName).description;
        },

        handleKeydown(event) {
            switch (event.key) {
                case 's':
                    this.step();
                    break;
                case 'r':
                    this.reset();
                    break;
                default:
                    break;
            }
        },

        async reset() {
            if (this.loadedProgramName) {
                await this.loadProgram();
            } else {
                this.memoryPage = 0x00;
                await fetch('/reset');
                await this.getCpuState();
            }
        },

        async getCpuState() {
            const res = await fetch('/cpu');
            this.cpu = await res.json();
        },

        async loadProgram() {
            this.memoryPage = 0x00;
            const program = programs.find(p => p.name === this.loadedProgramName);
            let disassemblyResponse = null;
            if (program.binary) {
                const rom = program.binary.toString();
                disassemblyResponse = await fetch('/loadRom', {
                    method: 'POST',
                    body: rom,
                    headers: {'Content-Type': 'text/plain'}
                });
            } else if (program.file) {
                const file = program.file;
                disassemblyResponse = await fetch('/loadFile', {
                    method: 'POST',
                    body: file,
                    headers: {'Content-Type': 'text/plain'}
                });
            } else {
                throw new Error("No binary or file specified for program");
            }

            await this.getCpuState();

            // Retrieve the disassembled result
            this.disassembly = disassemblyResponse.json();
            this.disassembly.forEach((line, index) => {
                this.pcToLineIndex[line.pc] = index;
            })

            await this.loadPage(this.memoryPage);
            await this.loadStackPage();
        },

        async step() {
            const cpuRes = await fetch('/step');
            this.cpu = await cpuRes.json();
            await this.loadPage(this.memoryPage);
            await this.loadStackPage();
        },

        async loadPage(page) {
            if (isNaN(page)) {
                throw new Error("Must be a number")
            }

            if (page < 0) {
                page = 0xFF;
            } else if (page > 0xFF) {
                page = 0;
            }
            this.memoryPage = page;
            const res = await fetch('/memory/' + page);
            const buffer = await res.arrayBuffer();
            this.pageData = new Uint8Array(buffer);
        },

        async loadStackPage() {
            const res = await fetch('/memory/' + this.stackPage);
            const buffer = await res.arrayBuffer();
            this.stackData = new Uint8Array(buffer);
        },

        async nmi() {
            await fetch('/nmi');
        },

        async irq() {
            await fetch('/irq');
        },

        isCurrentLine(address) {
            return address === this.cpu.pc;
        }
    }))
});