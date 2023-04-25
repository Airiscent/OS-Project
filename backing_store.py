#!usr/bin/env python

import random

with open("BACKING_STORE.bin", "wb") as f:
    f.write(bytes(random.randbytes(65536)))
