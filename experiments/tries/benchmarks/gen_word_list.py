import sys
import random

random.seed(None)

letters = [chr(x) for x in range(ord('A'), ord('z'))]
#print(letters)

with open(sys.argv[1], 'w') as f:
    for x in range(int(sys.argv[2])):
        word_len = random.randint(1,20)
        f.write(''.join(random.sample(letters, word_len)) + '\n')

