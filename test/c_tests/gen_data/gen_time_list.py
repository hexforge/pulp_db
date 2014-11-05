import sys
import random
import argparse


def generate_times(min_hour, max_hour, count):
    hours = min_hour
    mins = 0
    seconds = 0
    mill = 0

    value = 0
    while (True):
        mill += 1
        value += 1

        if (value > count):
            break
        if mill == 1000:
            seconds += 1
            mill = 0
        if seconds == 60:
            mins += 1
            seconds = 0
        if mins == 60:
            hours += 1
            mins = 0

        if hours == max_hour:
            break
        else:
            yield ("{0:0>2}:{1:0>2}:{2:0>2}.{3:0>3}".format(hours, mins, seconds, mill))

def generate_values(min_hour, max_hour, count):
    for value, key in enumerate(generate_times(min_hour, max_hour, count)):
        yield key, value

# We write three files, input, results, missing
def generate_input(filename, n):
    data = []
    with open(filename + '.input', 'w') as ifile:
        for key, value in generate_values(9, 18, n):
            string = "{} {}\n".format(key, value)
            data.append((key, value))
            ifile.write(string)
    return data

def generate_expected(data, filename):
    with open(filename + '.expected', 'w') as rfile:
        sorted_data = sorted(((key, value) for key, value in data), key=lambda x: x[0])
        for key, value in sorted_data:
            rfile.write('{} {}\n'.format(key, value))
    return data

def generate_missing(data, filename, n):
    with open(filename + '.missing', 'w') as mfile:
        for item in generate_times(21, 22, n):
            mfile.write(item + '\n')
    return data

def parse_args():
    parser = argparse.ArgumentParser(prog="")
    parser.add_argument("n", type=int, help="number_of_items")
    parser.add_argument("--filename", default="wSrcTime")
    parser.add_argument("--missing", type=int, default=1000, help="number_of_items to be in missing")
    return parser.parse_args()

def main():
    args = parse_args()
    data = generate_input(args.filename, args.n)
    generate_expected(data, args.filename)
    generate_missing(data, args.filename, args.missing)

if __name__ == "__main__":
    main()
