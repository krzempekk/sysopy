import random
import string

file_counts = [5, 50, 100]
directories = ["files/small_diff/", "files/medium_diff", "files/big_diff/"]

f = open("commands.txt", "w")
for file_count in file_counts:
    for directory in directories:
        command = f"./main {file_count} compare_pairs "

        for i in range(file_count):
            command += f"{directory}file{i}A.txt:{directory}file{i}B.txt "

        command += "\n"

        f.write(command)
f.close()