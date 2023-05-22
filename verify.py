import os
import sys
f1 = sys.argv[1]
f2 = sys.argv[2]
# open output.txt, in read mode
output = open(f1, "r")
# open expected.txt, in read mode
expected = open(f2, "r")

# sort lines based on first interger, then second integer, then third integer
output_lines = sorted(output.readlines(), key=lambda x: (int(x.split()[0]), int(x.split()[1]), int(x.split()[2])))
expected_lines = sorted(expected.readlines(), key=lambda x: (int(x.split()[0]), int(x.split()[1]), int(x.split()[2])))

# check if the number of lines are the same
if len(output_lines) != len(expected_lines):
    print("Incorrect number of lines")
    exit(1)

# write the sorted lines to ghs.txt and kruskals.txt
ghs = open("ghs.txt", "w")
ghs.writelines(output_lines)
kruskals = open("kruskals.txt", "w")
kruskals.writelines(expected_lines)

# check if the lines are the same
if output_lines != expected_lines:
    print("Incorrect output")
    exit(1)
else:
    print("Correct output")
    exit(0)

# close all files
output.close()
expected.close()
ghs.close()
kruskals.close()