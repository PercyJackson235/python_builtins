#!/usr/bin/python3
import timeit


class myrange(object):
    def __init__(self, *args):
        if not all(map(lambda x: isinstance(x, int), args)):
            msg = "Argument or arguments in args {} can not be interpeted as an integer."
            raise ValueError(msg.format(args))
        if len(args) == 1:
            self._start, self._stop, self._step = 0, args[0], 1
        elif len(args) == 2:
            self._start, self._stop, self._step = args[0], args[1], 1
        elif len(args) == 3:
            self._start, self._stop, self._step = args
            if self._step == 0:
                raise ValueError("Invalid step, step cannot be 0.")
        else:
            raise TypeError(f"myrange expected at most 3 arguements, got {len(args)}.")
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


def arg_gen(num=3):
    args = [0]
    while len(args) <= num:
        yield args
        args.append(num)


if __name__ == "__main__":
    for i in ("myrange(100)", "range(100)"):
        a = timeit.timeit(stmt=f"[i for i in {i}]", globals=globals(), number=10000)
        print(f"{i} took {a} seconds.")
