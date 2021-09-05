#!/usr/bin/python3
import cppbuiltins
import mybuiltins

class Fake(object):
    @mybuiltins._classmethod
    def mybuiltins(cls):
        print(f"mybuiltins._classmethod: {cls!r}")
    @classmethod
    def builtins(cls):
        print(f"classmethod: {cls!r}")
    @cppbuiltins.classmethod
    def cppbuiltins(cls):
        print(f"cppbuiltins.classmethod: {cls!r}")

Fake.mybuiltins()
Fake.builtins()
Fake.cppbuiltins()

print(Fake.mybuiltins)
print(Fake.builtins)
print(Fake.cppbuiltins)
