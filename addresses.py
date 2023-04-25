#!usr/bin/env python

import random

with open("specific.txt", "w") as f:
    for i in range(1000):
        f.write(str(128 * (i % 5)) + "\n")
        # f.write(str(random.randint(0, 65535)) + "\n")
