#!usr/bin/env python

import random

with open("addresses.txt", "w") as f:
    for i in range(1000):
        f.write(str(random.randint(0, 65535)) + "\n")
