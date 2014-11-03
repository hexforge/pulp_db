import sys
import random
import argparse

random.seed(None)

letters = [chr(x) for x in range(ord('A'), ord('z'))]

def generate_random_string():
    word_len = random.randint(1, 20)
    key = ''.join(random.sample(letters, word_len))
    return key

def generate_values(count):
    for x in range(count):
        key = generate_random_string()
        value = str(x)
        yield key, value

def generate_missing_values(count, data):
    x = 0
    while True:
        key = generate_random_string()
        
        if key in data:
            continue
        yield key

        x += 1
        if x > count:
            break

def generate_input(filename, n):
    data = {}
    with open(filename + '.input', 'w') as ifile:
        for key, value in generate_values(n):
            data[key] = value
            ifile.write('{} {}\n'.format(key, value))
    return data

def generate_expected(data, filename):
    with open(filename + '.expected', 'w') as rfile:
        sorted_data = sorted(((key, value) for key, value in data.items()), key=lambda x: x[0])
        for key, value in sorted_data:
            rfile.write('{} {}\n'.format(key, value))
    return data

def generate_missing(data, filename, n):
    with open(filename + '.missing', 'w') as mfile:
        for item in generate_missing_values(n, data):
            mfile.write(item + '\n')
    return data

def parse_args():
    parser = argparse.ArgumentParser(prog="")
    parser.add_argument("n", type=int, help="number_of_items")
    parser.add_argument("--filename", default="random")
    parser.add_argument("--missing", type=int, default=1000, help="number_of_items to be in missing")
    return parser.parse_args()

def main():
    args = parse_args()
    data = generate_input(args.filename, args.n)
    generate_expected(data, args.filename)
    generate_missing(data, args.filename, args.missing)

if __name__ == "__main__":
    main()


