const emulator = require('./build/Release/m6502_emulator.node');

const rom = "A9 10 8D 00 02";
const len = rom.length;

emulator.cpu_init();
emulator.load_rom(0x0600, len, rom);
emulator.cpu_reset();

emulator.cpu_step();
emulator.cpu_step();

console.log(emulator.get_cpu_state());
console.log(emulator.get_bus_page(0x02));
