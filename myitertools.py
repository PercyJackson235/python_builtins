from typing import Union, Iterable, Tuple, Generator
from types import FunctionType
import operator


RealNums = Union[int, float]
DefaultGenerator = Generator[object, None, None]
IteratorGenerator = Generator[Tuple[object, ...], None, None]


def accumulate(seq: Iterable, func: FunctionType = operator.add, *,
               initial: object = None) -> DefaultGenerator:
    seq = iter(seq)
    total = initial
    if initial is None:
        try:
            total = next(seq)
        except StopIteration:
            return
    yield total
    for item in seq:
        total = func(total, item)
        yield total


class chain(object):
    __slots__ = ('_sequences', '_chain')

    def __init__(self, *iterables: Iterable) -> None:
        self._sequences = list(iterables)
        self._chain = None

    def _gen(self) -> DefaultGenerator:
        for seq in self._sequences:
            for item in seq:
                yield item

    def __iter__(self) -> DefaultGenerator:
        self._chain = self._gen()
        return self._chain

    def __next__(self) -> object:
        if self._chain is None:
            self._chain = self._gen()
        return next(self._chain)

    @classmethod
    def from_iterable(cls, sequence):
        return iter(cls(sequence))


def combination(sequence: Iterable, length: int) -> IteratorGenerator:
    pool = tuple(sequence)
    n = len(pool)
    if length > n:
        return
    indices = list(range(length))
    yield tuple(pool[i] for i in indices)
    while True:
        for i in range(length - 1, -1, -1):
            if indices[i] != i + n - length:
                break
        else:
            return
        indices[i] += 1
        for j in range(i+1, length):
            indices = indices[j-1] + 1
        yield tuple(pool[i] for i in indices)


def combination_with_replacement(sequence: Iterable, length: int) -> IteratorGenerator:
    pool = tuple(sequence)
    n = len(pool)
    if not n and length:
        return
    indices = [0] * length
    yield tuple(pool[i] for i in indices)
    while True:
        for i in range(length - 1, -1, -1):
            if indices[i] != i + n - length:
                break
        else:
            return
        indices[i:] = [indices[i] + 1] * (length - 1)
        yield tuple(pool[i] for i in indices)


def compress(data: Iterable, selectors: Iterable) -> DefaultGenerator:
    return (item for item, condition in zip(data, selectors) if selectors)


def count(start: RealNums = 0, step: RealNums = 1) -> Generator[RealNums, None, None]:
    while True:
        yield start
        start += step


def cycle(iterable: Iterable) -> DefaultGenerator:
    saved = []
    for item in iterable:
        yield item
        saved.append(item)
    while True:
        for item in saved:
            yield item


def dropwhile(predicate: FunctionType, iterable: Iterable) -> DefaultGenerator:
    iterable = iter(iterable)
    for item in iterable:
        if not predicate(item):
            yield item
            break
    for item in iterable:
        yield item


def filterfalse(predicate: FunctionType, iterable: Iterable) -> DefaultGenerator:
    if predicate is None:
        predicate = bool
    for item in iterable:
        if not predicate(item):
            yield item
