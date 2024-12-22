import os
import re

def find_su_files(directory):
    su_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.su'):
                su_files.append(os.path.join(root, file))
    return su_files

def extract_stack_usage(file_path):
    function_stack_usage = {}
    with open(file_path, 'r') as file:
        for line in file:
            match = re.search(r'(\S+)\s+(\d+)\s+static', line)
            if match:
                function_name = " ".join(line.split("\t")[:-2])
                usage = int(match.group(2))
                if function_name in function_stack_usage:
                    function_stack_usage[function_name] += usage
                else:
                    function_stack_usage[function_name] = usage
    return function_stack_usage

def calculate_stack_usage_by_function(directory):
    su_files = find_su_files(directory)
    function_stack_usage = {}
    for su_file in su_files:
        file_stack_usage = extract_stack_usage(su_file)
        for func, usage in file_stack_usage.items():
            # key = f"{su_file}:{func}"
            key = func
            if func in function_stack_usage:
                function_stack_usage[key] += usage
            else:
                function_stack_usage[key] = usage
    sorted_stack_usage = dict(sorted(function_stack_usage.items(), key=lambda item: item[1], reverse=True))
    return sorted_stack_usage

def calculate_stack_usage_by_file(directory):
    su_files = find_su_files(directory)
    stack_usage = {}
    for su_file in su_files:
        stack_usage[su_file] = extract_stack_usage(su_file)
    sorted_stack_usage = dict(sorted(stack_usage.items(), key=lambda item: item[1], reverse=True))
    return sorted_stack_usage

if __name__ == "__main__":
    # directory = input("directory: ")
    directory = ".pio/build/nucleo_f303k8"
    # directory = "firmware_updater_build/"
    stack_usage = calculate_stack_usage_by_function(directory)
    print(f"Total stack usage by function:")
    for function, usage in stack_usage.items():
        print(f"{function}: {usage}")