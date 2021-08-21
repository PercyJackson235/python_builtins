#!/usr/bin/python3
from subprocess import call

if call(["./compile"]):
    import sys
    sys.exit()
else:
    call(["clear"])
    import cppbuiltins


class Fake:
    @cppbuiltins.classmethod
    def fake(cls):
        print(cls)

print(Fake.fake())
print("Success")
