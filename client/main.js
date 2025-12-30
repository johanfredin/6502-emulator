const emulator = require('./build/Release/m6502_emulator.node');
const express = require('express');
const path = require('path');

const app = express();
const port = 3000;

// Load up the emulator with a stupid program
const reset = function() {
    emulator.cpu_init();
    emulator.cpu_reset();
}

// Expose emulator endpoints -----
app.get('/cpu', (req, res) => {
    const cpuState = emulator.get_cpu_state();
    return res.json(cpuState);
});

app.get('/reset', (req, res) => {
    reset();
    return res.status(200).send();
})

app.get('/step', (req, res) => {
    emulator.cpu_step();
    const cpuState = emulator.get_cpu_state();
    return res.json(cpuState);
});

app.get('/memory/:page', (req, res) => {
    const page = parseInt(req.params.page);
    const chunk = emulator.get_bus_page(page);
    return res.send(Buffer.from(chunk));
})

app.post('/loadRom', express.text({ type: '*/*' }), (req, res) => {
    // Clear bus and cpu state
    emulator.cpu_init();

    const rom = req.body;
    const len = rom.length;
    emulator.load_rom(0x0600, len, rom);

    // Call reset again to load the program into memory
    emulator.cpu_reset();

    const disassembly = emulator.get_disassembly();
    return res.json(disassembly);
});

app.post('/loadFile', express.text({ type: '*/*' }), (req, res) => {
    // Clear bus and cpu state
    emulator.cpu_init();

    const file = req.body;
    emulator.load_file(file);

    // Call reset again to load the program into memory
    emulator.cpu_reset();

    // Retrieve disassembled lines
    const disassembly = emulator.get_disassembly();
    return res.json(disassembly);
});

app.get('/nmi', (req, res) => {
    emulator.cpu_nmi();
    const cpuState = emulator.get_cpu_state();
    return res.json(cpuState);
})

app.get('/irq', (req, res) => {
    emulator.cpu_irq();
    const cpuState = emulator.get_cpu_state();
    return res.json(cpuState);
})

// Init on startup
reset();

// Serve static files
app.use(express.static(path.join(__dirname)));
app.use(express.text({ type: 'text/plain' }))
app.listen(port, () => console.log(`Server running on port ${port}`));


