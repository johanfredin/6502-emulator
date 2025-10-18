// TODO: Fix ugly workaround
#include "dbg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "/home/johan/.nvm/versions/node/v22.20.0/include/node/node_api.h"
#include "bus.h"
#include "cpu.h"

static inline napi_value void_return(const napi_env env) {
    napi_value nv;
    napi_get_undefined(env, &nv);
    return nv;
}

napi_value cpu_init(napi_env env, napi_callback_info info) {
    CPU_load_instructions();
    Bus_init();

    napi_value nv;
    const napi_status status = napi_get_undefined(env, &nv);
    check_return(status == napi_ok, "Could not get undefined value, return code=%d", NULL, status);

    return nv;
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

// Module initialization
napi_value init(const napi_env env, const napi_value exports) {
    napi_value fn_cpu_init;
    napi_value fn_load_rom;
    napi_value fn_cpu_reset;

    napi_create_function(env, "cpu_init", NAPI_AUTO_LENGTH, cpu_init, NULL, &fn_cpu_init);
    napi_create_function(env, "load_rom", NAPI_AUTO_LENGTH, load_rom, NULL, &fn_load_rom);
    napi_create_function(env, "cpu_reset", NAPI_AUTO_LENGTH, cpu_reset, NULL, &fn_cpu_reset);
    napi_set_named_property(env, exports, "cpu_init", fn_cpu_init);
    napi_set_named_property(env, exports, "load_rom", fn_load_rom);
    napi_set_named_property(env, exports, "cpu_reset", fn_cpu_reset);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
