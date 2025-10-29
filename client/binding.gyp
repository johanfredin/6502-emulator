{
  "targets": [
    {
      "target_name": "m6502_emulator",
      "sources": [ "wrapper.c" ],
      "include_dirs": [ "<(module_root_dir)" ],
      "libraries": [ "<(module_root_dir)/../cmake-build-debug/lib6502_emulator_lib.so" ],
      "ldflags": [ "-Wl,-rpath,\\$$ORIGIN" ],
      "copies": [
        {
          "files": [
            "<(module_root_dir)/../cmake-build-debug/lib6502_emulator_lib.so",
            "<(module_root_dir)/../core/bus.h",
            "<(module_root_dir)/../core/cpu.h",
            "<(module_root_dir)/../core/disassembler.h"
          ],
          "destination": "<(module_root_dir)/build/Release"
        }
      ]
    }
  ]
}
