#!/usr/bin/python3
import cppbuiltins
import random

a = list(range(15))
print(a)

b = a.copy()
random.shuffle(b)
d = b.copy()
print(b)

c = cppbuiltins.merge_sort(b)
print(f"a == c : {a == c}")
print(c)

cppbuiltins.timsort(d)
print(f"a == d : {a == d}")

