#!/usr/bin/python3
from setuptools import setup, Extension
import os
from os.path import join
import re
import shutil
import glob


def find(folder: str, files: list, sources: list) -> bool:
    for filepath in (join(folder, file) for file in files):
        for source in sources:
            pattern = source.split('module')[0] + r'.*\.so'
            if re.search(pattern, filepath):
                dest = join(os.getcwd(), os.path.basename(filepath))
                print(f"Moving {filepath} to {dest}.")
                shutil.move(filepath, dest)
                print("Removing build directory.")
                shutil.rmtree('build')
                return True


sources = glob.glob("*.cpp")
name = sources[0].split('module')[0]
module = Extension(name, sources=sources, language='c++')
setup(name=name, ext_modules=[module])

for folder, subfolder, files in os.walk('.'):
    if find(folder, files, sources):
        break
