import random
import string
import sys

def random_string(string_length):
    letters = string.ascii_lowercase
    return "".join(random.choice(letters) for i in range(string_length))

def random_file_pair(line_count, line_length, diff_ratio):
    file1_arr = [random_string(line_length) for i in range(line_count)]
    file2_arr = file1_arr.copy()
    for i in random.sample(range(line_count), int(diff_ratio * line_count)):
        file2_arr[i] = random_string(line_length)
    return ('\n'.join(file1_arr), '\n'.join(file2_arr))

file_count = int(sys.argv[1])
line_count = int(sys.argv[2])
line_length = int(sys.argv[3])
diff_ratio = float(sys.argv[4])

for i in range(file_count):
    random_pair = random_file_pair(line_count, line_length, diff_ratio)

    f = open("file" + str(i) + "A.txt", "w")
    f.write(random_pair[0])
    f.close()

    f = open("file" + str(i) + "B.txt", "w")
    f.write(random_pair[1])
    f.close()
    
