from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

# HEX from ELF
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY", "-O", "ihex", "-R", ".eeprom", 
        '"$BUILD_DIR/${PROGNAME}.elf"', '"$BUILD_DIR/${PROGNAME}.hex"'
    ]), "Building $BUILD_DIR/${PROGNAME}.hex")
)

# BIN from ELF
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY", "-O", "binary", 
        '"$BUILD_DIR/${PROGNAME}.elf"', '"$BUILD_DIR/${PROGNAME}.bin"'
    ]), "Building $BUILD_DIR/${PROGNAME}.bin")
)