import subprocess
import os
import random

nose_tests = [x for x in os.listdir() if "test_" in x]
print(nose_tests)

while True:
    cmd = "/usr/bin/nosetests-3.3 {} > /dev/null"
    random_test = random.choice(nose_tests)
    print("running", random_test)
    rc = subprocess.call(cmd.format(random_test), shell=True)
    if rc != 0:
        break
