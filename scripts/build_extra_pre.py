from SCons.Script import DefaultEnvironment
from rename_firmware_update_files import append_temp, remove_temp
from generate_firmware_update_library import main
import shutil
import os

env = DefaultEnvironment()

project_dir = env.subst("$PROJECT_DIR")
build_lib_path = os.path.join(project_dir, "firmware_updater_build", "libFirmwareUpdater.a")
lib_path = os.path.join(project_dir, "lib", "FirmwareUpdater", "libFirmwareUpdater.a")

os.remove(lib_path)
remove_temp()
main()
append_temp()

shutil.copyfile(build_lib_path, lib_path)

print("Linking with libFirmwareUpdater.a at:", lib_path)
if not os.path.isfile(lib_path):
    print("Error: libFirmwareUpdater.a not found at {}".format(lib_path))
else:
    # use linker flags to include the entire archive no matter what. this is essential so that the symbols in the `firmware_update` sections are consistent between different compilations
    env.Prepend(
        LINKFLAGS=[
            "-Wl,--whole-archive",
            lib_path,
            "-Wl,--no-whole-archive"
        ]
    )