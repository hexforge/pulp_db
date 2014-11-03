import pickle

import sys
my_dict = {}
with open(sys.argv[1]) as f:
    for num, line in enumerate(f):
        word =line.strip()
        my_dict[word] = num

import time
time.sleep(10)
#with open('foobar.pickle', 'bw') as f:
#    pickle.dump(my_dict, f)


print("done")
