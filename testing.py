import asyncio


class Foo:
    def __init__(self):
        print("Setting synchronous attributes")
        self.one = 1

    async def bar(self):
        print("hello world")

    @classmethod
    async def alt_constructor(cls):
        foo = cls()
        await foo.set_attribs()
        await foo.bar()
        return foo

    async def set_attribs(self):
        print("Setting asynchronous attributes")
        self.two = await get_attributes()


async def get_attributes():
    return 'Hello'


async def Fizz():
    print("Fizz!")


async def initialize():
    await Foo.alt_constructor()
    await Fizz()

if __name__ == "__main__":
    asyncio.run(initialize())
