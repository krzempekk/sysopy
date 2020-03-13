import sys

RECORD_SIZES = [1, 4, 512, 1024, 4096, 8192]
RECORD_COUNTS = [50000, 100000]

f = open("commands.txt", "w")

command = ""
i = 1

for size in RECORD_SIZES:
    for count in RECORD_COUNTS:
        command += f"\t@echo '**** {count} records with size of {size} bytes ****'\n"
        command += f"\t@./main generate tests/t{i} {count} {size}\n"
        command += f"\t@echo copy using system functions\n"
        command += f"\t@./main copy tests/t{i} tests/t{i}_1 {count} {size} sys\n"
        command += f"\t@echo copy using library functions\n"
        command += f"\t@./main copy tests/t{i} tests/t{i}_2 {count} {size} lib \n"
        command += f"\t@echo sort using system functions\n"
        command += f"\t@./main sort tests/t{i}_1 {count} {size} sys\n"
        command += f"\t@echo sort using library functions\n"
        command += f"\t@./main sort tests/t{i}_2 {count} {size} lib \n"
        i += 1

f.write(command)
f.close()