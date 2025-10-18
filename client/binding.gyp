{
  "targets": [
    {
      "target_name": "client",
      "sources": [ "bindings.c" ],
      "include_dirs": [ "<(module_root_dir)" ],
      "libraries": [ "<(module_root_dir)/lib6502_emulator_lib.so" ],
      "ldflags": [ "-Wl,-rpath,\\$$ORIGIN" ],
      "copies": [
        {
          "files": [ "<(module_root_dir)/lib6502_emulator_lib.so" ],
          "destination": "<(module_root_dir)/build/Release"
        }
      ]
    }
  ]
}
