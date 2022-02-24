from typing import Union, Iterable, Tuple, Generator, Iterator
from typing import Mapping, Set
from types import FunctionType
from functools import wraps
import inspect
import sys
from io import TextIOWrapper

RealNums = Union[int, float]
DefaultGenerator = Generator[object, None, None]


def type_name(obj: object) -> str:
    return type(obj).__name__


def _abs(number: Union[RealNums, complex]) -> RealNums:
    if isinstance(number, (int, float)):
        if number < 0:
            number *= -1
    elif isinstance(number, complex):
        number = (number.real**2 + number.imag**2)**0.5
    elif hasattr(number, '__abs__'):
        number = number.__abs__()
    else:
        raise TypeError(f"bad operand type for abs(): {type_name(number)!r}")
    return number


def _all(iterable: Iterable) -> bool:
    for item in iterable:
        if not item:
            return False
    return True


def _any(iterable: Iterable) -> bool:
    for item in iterable:
        if item:
            return True
    return False


def _bin(number: int) -> str:
    if isinstance(number, int) or hasattr(number, '__index__'):
        header = '0b'
        binary = ''
        number = number.__index__()
        if not isinstance(number, int):
            raise TypeError(f" __index__ returned non-int (type {type_name(number)})")
        if number < 0:
            header = '-' + header
            number *= -1
        elif number == 0:
            return header + '0'
        while number != 0:
            binary = str(number % 2) + binary
            number //= 2
        return header + binary
    raise TypeError(f"{type_name(number)!r} object cannot be interpreted as an integer")


def _bool(obj: object = None) -> bool:
    if hasattr(obj, '__bool__'):
        return obj.__bool__()
    elif hasattr(obj, '__len__'):
        result = len(obj)
        if result == 0:
            return False
    return True


def _callable(obj: object) -> bool:
    return hasattr(obj, '__call__')


class _classmethod(object):
    def __init__(self, func: FunctionType):
        def function(owner: object):
            @wraps(func)
            def wrapper(*args, **kwargs):
                return func(owner, *args, **kwargs)
            return wrapper
        self.func = function

    def __get__(self, instance: object, owner=None) -> object:
        return self.func(owner)


class NullType(object):
    _singleton = None

    def __new__(cls, *args, **kwargs):
        if cls._singleton is None:
            cls._singleton = object.__new__(cls, *args, **kwargs)
        return cls._singleton


Null = NullType()


def _dir(obj: object = NullType()) -> list:
    attrs = []
    if obj is Null:
        frames = inspect.getouterframes(inspect.currentframe())
        attrs += sorted(frames[1].frame.f_locals)
    elif hasattr(obj, '__dir__'):
        attrs += obj.__dir__()
    else:
        if hasattr(obj, '__slots__'):
            if isinstance(obj.__slots__, str):
                attrs.append(obj.__slots__)
            else:
                attrs.extend(obj.__slots__)
        if hasattr(obj, '__dict__'):
            attrs.extend(list(obj.__dict__))
    try:
        attrs.extend(type(obj).__dict__)
    except AttributeError:
        pass
    return sorted(set(attrs))


def _divmod(a: RealNums, b: RealNums) -> Tuple[RealNums, RealNums]:
    if any(isinstance(i, complex) for i in (a, b)):
        raise TypeError("can't take floor or mod of complex number.")
    elif any(not isinstance(i, (int, float)) for i in (a, b)):
        msg = "unsupported operand type(s) for _divmod(): {!r} and {!r}"
        raise TypeError(msg.format(map(type_name, (a, b))))
    return a // b, a % b


def _enumerate(iterable: Iterable, start: int = 0) -> DefaultGenerator:
    if not isinstance(start, int):
        raise TypeError(f"{type_name()!r} object cannot be interpreted as an integer")
    for item in iterable:
        yield start, item
        start += 1


def _filter(function: Union[FunctionType, None], iterable: Iterable) -> DefaultGenerator:
    if function is None:
        function = bool
    for item in iterable:
        if function(item):
            yield item


def _getattr(obj: object, name: str, default: object = Null) -> object:
    if default is Null:
        return object.__getattribute__(obj, name)
    else:
        try:
            return object.__getattribute__(obj, name)
        except AttributeError:
            return default


def _hasatter(obj: object, name: str) -> bool:
    try:
        _getattr(obj, name)
        return True
    except AttributeError:
        return False


def _hash(obj: object) -> int:
    if not hasattr(obj, '__hash__') or not callable(obj.__hash__):
        raise TypeError(f"unhashable type: {type_name(obj)!r}")
    num = obj.__hash__()
    if not isinstance(num, int):
        raise TypeError("__hash__ method should return an integer")
    return num


def _hex(number: int) -> str:
    hex_char = {10: 'a', 11: 'b', 12: 'c', 13: 'd', 14: 'e', 15: 'f'}
    if isinstance(number, int) or hasattr(number, '__index__'):
        header = '0x'
        hex_num = ''
        number = number.__index__()
        if not isinstance(number, int):
            raise TypeError(f" __index__ returned non-int (type {type_name(number)})")
        if number < 0:
            header = '-' + header
            header *= -1
        elif number == 0:
            return header + '0'
        while number != 0:
            digit = number % 16
            hex_num = str(hex_char.get(digit, digit)) + hex_num
            number //= 16
        return hex_num
    raise TypeError(f"{type_name(number)!r} object cannot be interpreted as an integer")


def _input(prompt: str = '') -> str:
    prompt = repr(prompt)
    sys.stdout.write(prompt)
    return sys.stdin.readline().rstrip()


def _isinstance(obj, *classinfo) -> bool:
    if any(cls.__class__ != type for cls in classinfo):
        raise TypeError("_isinstance() arg 2 must be a type or tuple of types")
    parents = set(type(obj).mro())
    for cls in classinfo:
        if cls in parents:
            return True
    return False


def _issubclass(cls, *classinfo) -> bool:
    if cls.__class__ != type:
        raise TypeError("_issubclass() arg 1 must be a type")
    elif any(cls.__class__ != type for cls in classinfo):
        raise TypeError("_isinstance() arg 2 must be a type or tuple of types")
    parents = set(cls.mro())
    return any(clstype in parents for clstype in classinfo)


def _iter(obj: Iterable, sentinel: object = Null) -> Union[Iterator, DefaultGenerator]:
    if sentinel is Null:
        if hasattr(obj, '__iter__'):
            return obj.__iter__()
        elif hasattr(obj, '__getitem__'):
            def fake_iterator():
                num = 0
                try:
                    while True:
                        yield obj[num]
                        num += 1
                except KeyError:
                    if num == 0:
                        raise TypeError(f"{type_name(obj)!r} is not iterable") from None
                except IndexError:
                    pass
            return fake_iterator()
        raise TypeError(f"{type_name(obj)!r} is not iterable")
    else:
        if not callable(obj):
            raise TypeError("_iter(obj, sentinel): obj must be callable")

        def fake_iterator():
            while True:
                result = obj()
                if result == sentinel:
                    break
                yield result
        return fake_iterator()


def _len(obj: object) -> int:
    if hasattr(obj, "__len__"):
        result = obj.__len__()
        if not isinstance(result, int):
            if hasattr(result, '__index__'):
                result = result.__index__()
            else:
                msg = f"{type_name(result)!r} object cannot be interpreted as an integer"
                raise TypeError(msg)
        if result < 0:
            raise ValueError("__len__() should return >= 0")
        return result
    raise TypeError(f"object of type {type_name(obj)!r} has no _len()")


def _map(func: FunctionType, *iterable: Iterable) -> Generator:
    for args in zip(*iterable):
        yield func(*args)


def insertion_sort(arr: list, left: int = 0, right: int = None) -> None:
    if right is None:
        right = len(arr) - 1
    for pos in range(left + 1, right + 1):
        item = arr[pos]
        while pos > left and arr[pos - 1] > item:
            arr[pos] = arr[pos - 1]
            pos -= 1
        arr[pos] = item


def merge(left: list, right: list) -> list:
    # If the first array is empty, then nothing needs
    # to be merged, and you can return the second array as the result
    # same with the other array
    if not left or not right:
        return left or right
    result = []
    i, j = 0, 0
    while (len(result) < len(left) + len(right)):
        # The elements need to be sorted to add them to the
        # resultant array, so you need to decide whether to get
        # the next element from the first or the second array
        if left[i] <= right[j]:
            result.append(left[i])
            i += 1
        else:
            result.append(right[j])
            j += 1
        # If you reach the end of either array, then you can
        # add the remaining elements from the other array to
        # the result and break the loop
        if i == len(left) or j == len(right):
            result.extend(left[i:] or right[j:])
            break
    return result


def mergesort(arr: list) -> list:
    # If the input array contains fewer than two elements,
    # then return it as the result of the function
    if len(arr) < 2:
        return arr
    middle = len(arr) // 2
    # Sort the array by recursively splitting the input
    # into two equal halves, sorting each half and merging them
    # together into the final result
    return merge(mergesort(arr[:middle]), mergesort(arr[middle:]))


def calc_min_run(length: int) -> int:
    MINRUN = 32
    r = 0
    while length >= MINRUN:
        r |= length & 1
        length >>= 1
    return length + r


def timsort(arr: list) -> None:
    n = len(arr)
    min_run = calc_min_run(n)
    for start in range(0, n, min_run):
        end = min(start + min_run - 1, n - 1)
        insertion_sort(arr, start, end)
    size = min_run
    while size < n:
        for left in range(0, n, 2*size):
            # mid = min(n-1, left+size-1)
            right = min((left + 2 * size - 1), (n - 1))
            # arr[left:right] = merge(arr[left:mid], arr[mid:right])
            arr[left:right] = mergesort(arr[left:right])
        size *= 2


# BROKEN
def _max(*iterable: Iterable, key: FunctionType = None, default: object = Null) -> object:
    if len(iterable) == 1 and not isinstance(iterable[0], Iterable):
        raise TypeError(f"{type_name(iterable)!r} object is not iterable")
    elif len(iterable) > 1:
        iterable = list(iterable)
    else:
        iterable = iterable[0]
    if len(iterable) == 0:
        if default is not Null:
            return default
        else:
            raise ValueError("_max() arg is an empty sequence")
    if key is not None:
        if not callable(key):
            raise TypeError(f"{type_name(key)!r} is not callable")
        arr = {key(i): i for i in iterable}
        keys = list(arr)
        timsort(keys)
        return arr[keys[-1]]
    else:
        arr = iterable.copy()
        timsort(arr)
        return arr[-1]


# BROKEN
def _min(*iterable: Iterable, key: FunctionType = None, default: object = Null) -> object:
    if len(iterable) == 1 and not isinstance(iterable[0], Iterable):
        raise TypeError(f"{type_name(iterable)!r} object is not iterable")
    elif len(iterable) > 1:
        iterable = list(iterable)
    else:
        iterable = iterable[0]
    if len(iterable) == 0:
        if default is not Null:
            return default
        else:
            raise ValueError("_min() arg is an empty sequence")
    if key is not None:
        if not callable(key):
            raise TypeError(f"{type_name(key)!r} is not callable")
        arr = {key(i): i for i in iterable}
        keys = list(arr)
        timsort(keys)
        return arr[keys[0]]
    else:
        arr = iterable.copy()
        timsort(arr)
        return arr[0]


def _next(iterator: Iterator, default: object = Null) -> object:
    try:
        return iterator.__next__()
    except StopIteration:
        if default is not Null:
            return default
        raise StopIteration from None


def _oct(number: int) -> str:
    if isinstance(number, int) or hasattr(number, '__index__'):
        header = '0o'
        hex_num = ''
        number = number.__index__()
        if not isinstance(number, int):
            raise TypeError(f" __index__ returned non-int (type {type_name(number)})")
        if number < 0:
            header = '-' + header
            number *= -1
        elif number == 0:
            return header + '0'
        while number != 0:
            digit = number % 8
            hex_num = str(digit) + hex_num
            number //= 8
        return hex_num
    raise TypeError(f"{type_name(number)!r} object cannot be interpreted as an integer")


def _pow(base: RealNums, exp: RealNums, mod: RealNums = None) -> RealNums:
    if mod is not None:
        if any(not isinstance(num, int) for num in (base, exp, mod)):
            msg = "_pow() 3rd argument not allowed unless all arguments are integers"
            raise TypeError(msg)
    else:
        mod = 1
    return (base ** exp) % mod


def _print(*objects, sep: str = ' ', end: str = '\n', file: TextIOWrapper = sys.stdout,
                                flush: bool = False) -> None:
    error_msg = "{} must be None or a string not {}"
    if not isinstance(sep, str):
        if sep is not None:
            raise TypeError(error_msg.format('sep', type_name(sep)))
        else:
            sep = ' '
    if not isinstance(end, str):
        if end is not None:
            raise TypeError(error_msg.format('end', type_name(sep)))
        else:
            end = '\n'
    file.write(sep.join(map(str, objects)) + end)
    if flush:
        file.flush()


class _property(object):
    def __init__(self, fget: FunctionType = None, fset: FunctionType = None,
                 fdel: FunctionType = None, doc: str = None):
        self._doc = doc
        self.fget = fget
        self.fset = fset
        self.fdel = fdel

    def create_descriptor(self, func: FunctionType) -> FunctionType:
        def function(instance):
            @wraps(func)
            def wrapper(*args, **kwargs):
                return func(instance, *args, **kwargs)
            return wrapper
        return function

    def getter(self, func: FunctionType) -> None:
        self.fget = self.create_descriptor(func)

    def __get__(self, instance: object, owner: object = None) -> object:
        return self.fget(instance)

    def setter(self, func: FunctionType) -> None:
        self.fset = self.create_descriptor(func)

    def __set__(self, instance: object, value: object) -> None:
        if self.fset is None:
            raise AttributeError("Can't set attribute")
        self.fset(instance, value)

    def deleter(self, func: FunctionType) -> None:
        self.fget = self.create_descriptor(func)

    def __delete__(self, instance: object) -> None:
        if self.fdel is None:
            raise AttributeError("Can't delete attribute")
        self.fdel(instance)


class _range(object):
    def __init__(self, *args):
        if not all(isinstance(arg, int) for arg in args):
            msg = f"Argument or arguments in args {args} can not be interpeted as an integer."
            raise ValueError(msg)
        if len(args) == 1:
            self._start, self._stop, self._step = 0, args[0], 1
        elif len(args) == 2:
            self._start, self._stop, self._step = args[0], args[1], 1
        elif len(args) == 3:
            if self._step == 0:
                raise ValueError("Invalid step, step cannot be 0.")
            self._start, self._stop, self._step = args
        else:
            raise TypeError(f"_range expected at most 3 arguements, got {len(args)}.")
        self._num = self._start

    def __iter__(self):
        return self

    def __next__(self):
        if self._num < self._stop:
            self._num += self._step
            return self._num - self._step
        else:
            raise StopIteration

    def __repr__(self):
        if self._step == 1:
            return f"myrange({self._start},{self._stop})"
        else:
            return f"myrange({self._start},{self._stop},{self._step})"

    def __len__(self):
        return self._stop - self._start

    def __getitem__(self, index: Union[int, slice]) -> Union[int, "_range"]:
        if isinstance(index, int):
            return self._start + (self._step * index)
        elif isinstance(index, slice):
            start = self._start + index.start if index.start is not None else self._start
            stop = index.stop if index.stop is not None else self._stop
            step = index.step if index.step is not None else self._step
            return self.__class__(start, stop, step)
        else:
            raise TypeError("slice indices must be integers")


def _reversed(seq: Iterable) -> Union[Iterator, DefaultGenerator]:
    if hasattr(seq, '__reversed__'):
        return seq.__reversed__()
    elif hasattr(seq, '__len__') and hasattr(seq, '__getitem__'):
        if not isinstance(seq, Mapping) and not isinstance(seq, Set):
            def fake_iterator():
                for index in range(len(seq)-1, -1, -1):
                    yield seq[index]
            return fake_iterator()
    raise TypeError(f"{type_name(seq)!r} is not reversible")


def _setattr(obj: object, name: str, value: object) -> None:
    object.__setattr__(obj, name, value)


def _sorted(*seq: Iterable, key: FunctionType = Null, reverse: bool = False) -> bool:
    seq = list(seq[0])
    if key is not Null:
        if key is None:
            pass
        elif not callable(key):
            raise TypeError(f"{type_name(key)!r} is not callable")
        else:
            seq = [key(item) for item in seq]
    timsort(seq)
    if reverse:
        return seq[::-1]
    return seq


class _staticmethod(object):
    def __init__(self, func: FunctionType):
        def function():
            @wraps(func)
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs)
            return wrapper
        self.func = function

    def __get__(self, instance: object, owner=None) -> object:
        return self.func()


def _sum(seq: Iterable, start: RealNums = 0) -> RealNums:
    if not isinstance(start, int):
        raise TypeError(f"_sum() can't sum {type_name(start)!r}")
    for item in seq:
        start += item
    return start


def _vars(obj: object = Null) -> dict:
    if obj is Null:
        raise NotImplementedError()
    if hasattr(obj, '__dict__'):
        return obj.__dict__
    raise TypeError("_vars() argument must have __dict__ attribute")


def _zip(*sequences) -> Generator[Tuple[object, ...], None, None]:
    sentinel = object()
    sequences = [iter(seq) for seq in sequences]
    while sequences:
        result = []
        for seq in sequences:
            item = next(seq, sentinel)
            if item is sentinel:
                return
            result.append(item)
        yield tuple(result)
