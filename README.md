# Tea

VM compiled programming language written in C++.
I started this project in the fall of 2020 because I wanted to learn more about
how to write a compiler and a VM.
In the future, I plan to add an LLVM-based compiler.

## The Tea language

The language I'm implementing is called Tea. It's a programming language
which is close to C, but supports OOP and is a little more simple to write.
I called the language Tea because I like tea and I wanted to make a language
that is close to C but more fun.

### Basic syntax

The following is a basic example of the Tea language syntax:

```tea
u64
fibonacci(u64 n)
{
	// Base case

	if (n <= 1)
	{
		return n;
	}

	// Recursive case

	return fibonacci(n - 1) + fibonacci(n - 2);
}
```

### Variables

Tea uses shorthand integer types like `u64` (unsigned 64 bit integer)
and `i32` (signed 32 bit integer).

```tea
u8 a = 1
u8 b = 2
u8 c = a + b
```

Tea does not support multiple variable declarations in one line using commas
like you can do in C. This is deliberate, because it makes it easier to
read and write code.

### Pointers

In Tea, you have direct control over memory. You can take the address of a
variable, and dereference memory addresses.

```tea
u8 a = 1
u8* b = &a
*a = 4
// >> a == 4
```

### Arrays

In Tea, you put the size of the array in square brackets after the type,
but before the identifier. This differs slightly from C, where you put
the size of the array after the identifier.

```tea
u64[10] a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }
a[0] = 123
// >> a[0] == 123
```

### Control flow

Tea supports your usual if-else, while, and for statements.
Switch statements will be added in the future.

```tea
while (a)
{
	if (b)
	{
		break
	}
	else if (c)
	{
		continue
	}
	else
	{
		// Do something
	}

	for (i = 0; i < 10; i++)
	{
		// Do something
	}
}
```

### Classes

Tea supports classes. Classes are simple structs, but can contain functions.

```tea
class Point
{
	u64 x
	u64 y

	constructor(u64 x, u64 y)
	{
		this.x = x
		this.y = y
	}

	void
	swap()
	{
		u64 tmp = x
		x = y
		y = tmp
	}
}

Point point = { 2, 3 } // Calls constructor(2, 3)
point.swap()
// >> point.x == 3
```

Classes may contain a constructor function, which is called when the class
is assigned a value. This value is a list of arguments that are passed to
the constructor.