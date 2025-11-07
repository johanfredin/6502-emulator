const emulator = require('./build/Release/m6502_emulator.node');
const express = require('express');
const path = require('path');

const app = express();
const port = 3000;

// Load up the emulator with a stupid program
const reset = function() {
    emulator.cpu_init();
    const rom = "E8 E0 0A D0 FB 8A";
    const len = rom.length;
    emulator.load_rom(0x0600, len, rom);
    emulator.disassemble(0x0600, 0x0700);
    emulator.cpu_reset();
}

// Init on startup
reset();

// Serve static files
app.use(express.static(path.join(__dirname)));

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

app.get('/disassembly', (req, res) => {
    const disassembly = emulator.get_disassembly();
    return res.json(disassembly);
})

app.listen(port, () => console.log(`Server running on port ${port}`));


