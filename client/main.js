const addon = require('./build/Release/client.node');

const rom = "A9 10 8D 00 02";
const len = rom.length;

addon.cpu_init();
addon.load_rom(0x0600, len, rom);
addon.cpu_reset();

addon.cpu_step();
addon.cpu_step();

console.log(addon.get_cpu_state());
console.log(addon.get_bus_page(0x02));
