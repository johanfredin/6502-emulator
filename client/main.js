const emulator = require('./build/Release/m6502_emulator.node');
const express = require('express');
const path = require('path');

const app = express();
const port = 3000;

// Load up the emulator with a stupid program that does
// LDA #$10, STA $00, program starts at $0600
emulator.cpu_init();
const rom = "A9 10 8D 00 00";
const len = rom.length;
emulator.load_rom(0x0600, len, rom);
emulator.cpu_reset();


// Serve static files
app.use(express.static(path.join(__dirname)));

// Expose emulator endpoints -----
app.get('/cpu', (req, res) => {
    const cpuState = emulator.get_cpu_state();
    return res.json(cpuState);
});

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

app.listen(port, () => console.log(`Server running on port ${port}`));

const helloWorld = () => {
    const rom = "A9 10 8D 00 02";
    const len = rom.length;

    emulator.cpu_init();
    emulator.load_rom(0x0600, len, rom);
    emulator.cpu_reset();

    emulator.cpu_step();
    emulator.cpu_step();

    console.log(emulator.get_cpu_state());
    console.log(emulator.get_bus_page(0x02));
}
