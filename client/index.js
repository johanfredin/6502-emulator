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
                case 'ArrowRight':
                case 'n':
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
            const rom = program.binary.toString();
            await fetch('/loadRom', {
                method: 'POST',
                body: rom,
                headers: { 'Content-Type': 'text/plain' }
            });
            await this.getCpuState();
            await this.loadDisassembly();
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

        async loadDisassembly() {
            const res = await fetch('/disassembly');
            this.disassembly = await res.json();

            // Build adress-to-index map
            this.disassembly.forEach((line, index) => {
                this.pcToLineIndex[line.pc] = index;
            })
        },

        isCurrentLine(address) {
            return address === this.cpu.pc;
        }
    }))
});