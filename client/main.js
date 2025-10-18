const addon = require('./build/Release/client.node');

const rom = "A9 10 8D 00 02";
const len = rom.length;

addon.cpu_init();
addon.load_rom(0x0600, len, rom);
addon.cpu_reset();
