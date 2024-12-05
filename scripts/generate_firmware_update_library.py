import subprocess
import os
import shutil

DEBUG = True # setting to `True` will enable debug symbols in the firmware update critical section, which is not wanted for the actual buoy

SOURCE_FILES = [
    "lib/fatfs/diskio.cpp",
    "lib/fatfs/fatfs.cpp",
    "lib/fatfs/ff.cpp",
    "lib/fatfs/ff_gen_drv.cpp",
    "lib/fatfs/user_diskio_spi.cpp",
    "lib/fatfs/user_diskio.cpp",
    "lib/fatfs/option/syscall.cpp",
    "lib/FirmwareUpdater/firmware_objects.cpp",
    "lib/FirmwareUpdater/firmware_update_linker.cpp",
    "lib/FirmwareUpdater/FirmwareUpdaterCritical.cpp",
    "lib/PAL_STM32_COMMON/PAL_STM32_COMMON.cpp",
    "lib/SD/SD.cpp",
    "lib/SD_File/SD_File.cpp"
]

OUTPUT_DIR = "firmware_updater_build"
LIBRARY_NAME = "libFirmwareUpdater.a"
LINKER_SCRIPT = "STM32F303K8TX_FLASH_modified.ld"

COMPILER = "arm-none-eabi-gcc"
AR_COMPILER = "arm-none-eabi-gcc-ar"
STRIP_TOOL = "arm-none-eabi-strip"
CFLAGS = [
    "-mcpu=cortex-m4",
    "-mthumb",
    "-Os",
    "-Wall",
    "-fno-tree-loop-distribute-patterns",
    "-fno-builtin",
    "-ffreestanding",
    "-nodefaultlibs",
    "-nostdlib",
    "-fno-lto",
    "-fno-record-gcc-switches",
    "-frandom-seed=2024",
    f"-T{LINKER_SCRIPT}",
    "-Wno-attributes",
    "-DPLATFORMIO=60116",
    "-DSTM32F3",
    "-DSTM32F303x8",
    "-DUSE_HAL_DRIVER",
    "-DF_CPU=72000000L",
    "-std=gnu++17",
    "-fno-exceptions",
    "-fno-rtti",
    "-Ilib/fatfs",
    "-Ilib/FirmwareUpdater",
    "-Ilib/PAL_STM32_COMMON",
    "-Ilib/SD",
    "-Ilib/SD_File",
    "-IC:/Users/User/.platformio/packages/framework-stm32cubef3/Drivers/CMSIS/Include",
    "-IC:/Users/User/.platformio/packages/framework-stm32cubef3/Drivers/CMSIS/Device/ST/STM32F3xx/Include",
    "-IC:/Users/User/.platformio/packages/framework-stm32cubef3/Drivers/STM32F3xx_HAL_Driver/Inc",
    "-IC:/Users/User/.platformio/packages/framework-stm32cubef3/Drivers/STM32F3xx_HAL_Driver/Src",
    "-IC:/Users/User/.platformio/packages/framework-stm32cubef3/Drivers/CMSIS/DSP/Include",
    "-fno-inline",
    "-fno-inline-functions",
]
if DEBUG:
    CFLAGS.append("-g")

def compile_sources(sources, output_dir, cflags):
    object_files = []
    for source in sources:
        # get the output file names for the object files
        output_file = os.path.join(
            output_dir,
            os.path.basename(source).replace(".cpp", ".o").replace(".c", ".o")
        )
        compile_cmd = [COMPILER, "-c", source, "-o", output_file] + cflags
        print(f"Compiling: {' '.join(compile_cmd)}")
        subprocess.run(compile_cmd, check=True)
        object_files.append(output_file)
    return object_files

def create_static_library(object_files, output_dir, library_name):
    object_files_sorted = sorted(object_files) # keep the same order
    library_path = os.path.join(output_dir, library_name)
    ar_cmd = [AR_COMPILER, "rcsD", library_path] + object_files_sorted # 'deterministic' keeps consistency even with different timestamps etc.
    print(f"Creating static library: {' '.join(ar_cmd)}")
    subprocess.run(ar_cmd, check=True)
    return library_path

def reset_output_dir():
    shutil.rmtree(OUTPUT_DIR)
    os.makedirs(OUTPUT_DIR)

def main():
    reset_output_dir()

    try:
        object_files = compile_sources(SOURCE_FILES, OUTPUT_DIR, CFLAGS)
        library_path = create_static_library(object_files, OUTPUT_DIR, LIBRARY_NAME)
        print(f"Static library created: {library_path}")
    except subprocess.CalledProcessError as e:
        print(f"Error during compilation: {e}")

if __name__ == "__main__":
    main()