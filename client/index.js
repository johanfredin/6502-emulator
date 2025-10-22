document.addEventListener('alpine:init', () => {
    Alpine.data('emulator', () => ({
        cpu: {},
        memoryPage: 0xFF,
        pageData: [],

        hex: (value, size = 2) => {
            if (!value) {
                value = 0;
            }
            return value.toString(16).toUpperCase().padStart(size, '0');
        },

        async init() {
            await this.getCpuState();
            await this.loadPage(this.memoryPage);
            console.log("init done");
        },

        async getCpuState() {
            const res = await fetch('/cpu');
            this.cpu = await res.json();
        },

        async step() {
            await fetch('/step');
            await this.getCpuState();
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
            console.log(this.pageData);
        },

    }))
});