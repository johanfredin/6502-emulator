// TODO: Fix ugly workaround
#include "dbg.h"
#include <stdio.h>
#include <stdlib.h>

#include "/home/johan/.nvm/versions/node/v22.20.0/include/node/node_api.h"
#include "../core/bus.h"
#include "../core/cpu.h"

static inline napi_value void_return(const napi_env env) {
    napi_value nv;
    napi_get_undefined(env, &nv);
    return nv;
}

static inline void bind_unsigned_int_field(const napi_env env, const napi_value c_struct, const char *field_name,
                                           const uint32_t field_value) {
    napi_value nv;
    napi_create_uint32(env, field_value, &nv);
    napi_set_named_property(env, c_struct, field_name, nv);
}

static inline napi_value bind_uint8_array(const napi_env env, const uint8_t *bus_chunk) {
    napi_value array_buffer;
    napi_status status = napi_create_external_arraybuffer(
        env,
        (void *) bus_chunk,
        0x100,
        NULL,
        NULL,
        &array_buffer
    );
    check_return(status == napi_ok, "Could not create external array buffer", NULL);

    napi_value typed_array;
    status = napi_create_typedarray(
        env,
        napi_uint8_array,
        0x100,
        array_buffer,
        0,
        &typed_array
    );
    check_return(status == napi_ok, "Could not create external array buffer", NULL);

    return typed_array;
}

napi_value cpu_init(const napi_env env, napi_callback_info info) {
    Bus_init();
    CPU_load_instructions();
    return void_return(env);
}

napi_value load_rom(const napi_env env, const napi_callback_info info) {
    // Requires arguments org and rom
    size_t argc = 3;
    napi_value args[3];
    const napi_status argc_result = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    try(argc_result == napi_ok, "Failed to retrieve arguments, status=%u", argc_result);
    try(argc == 3, "Wrong amount of arguments, expected: 3, got %lu", argc);

    // Retrieve org arg
    uint32_t org = 0;
    const napi_value org_arg = args[0];
    const napi_status result = napi_get_value_uint32(env, org_arg, &org);
    try(result == napi_ok, "Could not get org argument. status=%d.", result);

    // Retrieve length of the rom arg
    uint32_t rom_size = 0;
    const napi_value rom_size_arg = args[1];
    const napi_status rom_size_result = napi_get_value_uint32(env, rom_size_arg, &rom_size);
    try(rom_size_result == napi_ok, "Could not get rom size, return code=%d", rom_size_result);
    try(rom_size > 0, "Rom size is 0");

    // Retrieve the rom string itself

    char *rom = calloc(rom_size + 1, sizeof(char));
    check_mem(rom, goto catch);
    const napi_value rom_arg = args[2];
    const napi_status rom_result = napi_get_value_string_utf8(env, rom_arg, rom, rom_size + 1, NULL);
    try(rom_result == napi_ok, "Could not get the rom, return code=%d", rom_result);

    Bus_load_rom((uint16_t) org, rom);

    // No need to return anything
    free(rom);
    return void_return(env);
catch:
    napi_throw_error(env, NULL, "Error loading rom");
    return void_return(env);
}


napi_value cpu_reset(const napi_env env, napi_callback_info info) {
    CPU_reset();
    return void_return(env);
}

napi_value get_cpu_state(const napi_env env, napi_callback_info info) {
    const CPU *cpu = CPU_get_state();
    try(cpu, "CPU is null");

    napi_value cpu_bind;
    napi_create_object(env, &cpu_bind);

    bind_unsigned_int_field(env, cpu_bind, "a", cpu->a);
    bind_unsigned_int_field(env, cpu_bind, "x", cpu->x);
    bind_unsigned_int_field(env, cpu_bind, "y", cpu->y);
    bind_unsigned_int_field(env, cpu_bind, "sp", cpu->sp);
    bind_unsigned_int_field(env, cpu_bind, "pc", cpu->pc);
    bind_unsigned_int_field(env, cpu_bind, "status", cpu->status);
    bind_unsigned_int_field(env, cpu_bind, "addr_abs", cpu->addr_abs);
    bind_unsigned_int_field(env, cpu_bind, "addr_rel", cpu->addr_rel);
    bind_unsigned_int_field(env, cpu_bind, "cycles", cpu->cycles);
    bind_unsigned_int_field(env, cpu_bind, "curr_opcode", cpu->curr_opcode);
    bind_unsigned_int_field(env, cpu_bind, "data_fetched", cpu->data_fetched);

    return cpu_bind;
catch:
    napi_throw_error(env, NULL, "Error loading rom");
    return void_return(env);
}

napi_value get_bus_page(const napi_env env, const napi_callback_info info) {
    // Retrieve page arg
    size_t argc = 1;
    napi_value args[1];
    const napi_status argc_result = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    try(argc_result == napi_ok, "Failed to retrieve arguments, status=%u", argc_result);
    try(argc == 1, "Wrong amount of arguments, expected: 1, got %lu", argc);

    uint32_t page = 0;
    const napi_value page_arg = args[0];
    const napi_status result = napi_get_value_uint32(env, page_arg, &page);
    try(result == napi_ok, "Could not get page argument. status=%d.", result);

    const uint8_t *bus_chunk = Bus_get_page((uint8_t) page);

    const napi_value typed_array = bind_uint8_array(env, bus_chunk);
    try(typed_array, "Typed array is null");
    return typed_array;
catch:
    napi_throw_error(env, NULL, "Error getting page");
    return void_return(env);
}

napi_value cpu_step(const napi_env env, napi_callback_info info) {
    CPU_step();
    return void_return(env);
}

// Module initialization
napi_value init(const napi_env env, const napi_value exports) {
    napi_value fn_cpu_init;
    napi_value fn_load_rom;
    napi_value fn_cpu_reset;
    napi_value fn_get_cpu_state;
    napi_value fn_get_bus_page;
    napi_value fn_cpu_step;

    napi_create_function(env, "cpu_init", NAPI_AUTO_LENGTH, cpu_init, NULL, &fn_cpu_init);
    napi_create_function(env, "load_rom", NAPI_AUTO_LENGTH, load_rom, NULL, &fn_load_rom);
    napi_create_function(env, "cpu_reset", NAPI_AUTO_LENGTH, cpu_reset, NULL, &fn_cpu_reset);
    napi_create_function(env, "get_cpu_state", NAPI_AUTO_LENGTH, get_cpu_state, NULL, &fn_get_cpu_state);
    napi_create_function(env, "get_bus_page", NAPI_AUTO_LENGTH, get_bus_page, NULL, &fn_get_bus_page);
    napi_create_function(env, "cpu_step", NAPI_AUTO_LENGTH, cpu_step, NULL, &fn_cpu_step);
    napi_set_named_property(env, exports, "cpu_init", fn_cpu_init);
    napi_set_named_property(env, exports, "load_rom", fn_load_rom);
    napi_set_named_property(env, exports, "cpu_reset", fn_cpu_reset);
    napi_set_named_property(env, exports, "get_cpu_state", fn_get_cpu_state);
    napi_set_named_property(env, exports, "get_bus_page", fn_get_bus_page);
    napi_set_named_property(env, exports, "cpu_step", fn_cpu_step);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
