import os
import sys
from generate_firmware_update_library import SOURCE_FILES

def append_temp():
    """Append '.removed' to all files in SOURCE_FILES."""
    for file in SOURCE_FILES:
        if os.path.exists(file):
            new_name = f"{file}.removed"
            os.rename(file, new_name)
            print(f"Renamed: {file} -> {new_name}")
        else:
            print(f"File not found: {file}")

def remove_temp():
    """Remove '.removed' from all files in SOURCE_FILES."""
    for file in SOURCE_FILES:
        temp_name = f"{file}.removed"
        if os.path.exists(temp_name):
            os.rename(temp_name, file)
            print(f"Renamed: {temp_name} -> {file}")
        else:
            print(f"Temp file not found: {temp_name}")

def main():
    """Main function to handle command-line arguments."""
    if len(sys.argv) != 2:
        print("Usage: python rename_files.py [append|remove]")
        sys.exit(1)

    command = sys.argv[1].lower()
    if command == "append":
        append_temp()
    elif command == "remove":
        remove_temp()
    else:
        print("Invalid command. Use 'append' to add '.removed' or 'remove' to revert.")
        sys.exit(1)

if __name__ == "__main__":
    main()