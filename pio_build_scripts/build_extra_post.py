from SCons.Script import DefaultEnvironment
from rename_firmware_update_files import remove_temp
import shutil

env = DefaultEnvironment()

ENABLE_FIRMWARE_UPDATER_PRUNING = False
firmware_update_sections = ("firmware_update_section", "firmware_update_bss", "firmware_update_data", "firmware_update_rodata")

def prune_firmware_update_sections(source, target, env):
    firmware_path = str(target[0])
    firmware_copy_path = firmware_path.replace(".elf", "_unpruned.elf")
    shutil.move(firmware_path, firmware_copy_path)
    objcopy = env.subst("$OBJCOPY")
    remove_section_args = " ".join([f"--remove-section=.{section}" for section in firmware_update_sections])
    env.Execute(f"{objcopy} {remove_section_args} {firmware_copy_path} {firmware_path}")
    print(f"Pruned firmware: {firmware_path}")

if ENABLE_FIRMWARE_UPDATER_PRUNING:
    env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", prune_firmware_update_sections)
else:
    remove_temp()