
# *TEMPLATES*
 
  

templates can be used when needing to specify what type should be where.

here is an example, of casting an 'any' type var to an int and removing its abstraction layer.

```

int x = 9

any x_any = x

x = downcast<int>(x_any) + 1

log(x) # will print '10'

```

  
  templates can also be declared as optional. an example here from the inbuilt tuple code (tuple.em):

```

# tuples only support 4 values.

class<T, T1, T2?, T3?> tuple {
T i1
T1 i2,
T2 i3 = null
T3 i4 = null

[public]
constructor(T _i1, T1 _i2, T2 _i3 = null, T3 _i4 = null) => i1(_i1), i2(_i2), i3(_i3), i4(_i4){}

	method get(int i) {
		return [i1, i2, i3, i4][i]
	}
}

```
  as you can see, T2 and T3 are followed by a '?' to indicate that they are not used. if they are not used, every use of their type will be deleted, variables, functions with the same return type, and class inheritance with tuples of that type in template usage.

# *DEFAULT SPECIFIER*

  

the default specifier can be used to tell the compiler to figure out and auto fill what
you are doing. Here is an example, we tell a class constructor to initialize its variables accordingly.

  

```

class Person{
	string name
	int age
	string email

# default here '=> default' intializes each class variable in order, so:
# name = n
# age = a
# email = e

constructor(string n, int a, string e) => default {}

}


  

```

# *DECLARE KEYWORD*

the declare keyword is used to declare an inbuilt compiler flag. They should be put at the top of the file. Here are some examples:

```
# this declares that the file is in the inbuilt library.
declare inb true

# NOTE: 
# include.em is included with each .em file that uses this line of code.
# include.em validates that the file is in the root directory of the uncompiled ember inb files:
# include.em: 

	__err_assert(__dir__ == __config["root-dir"])

# if the user wants to make inbuilt modifications to ember,
# they just have to place their library code INSIDE their root dir.

```


```
# does not allow the file to be included.
declare NO_INCLUDE true

```

**here are the rest of the compiler flags:**
```
enum class COMPILER_FLAGS
{
	inb
	NO_INCLUDE
	NO_ENTRY # specifies that this file cannot be run on its own
	[TODO] add more
}
```


# *THE USE KEYWORD*

The use keyword is used to include files outside of your file being executed. its syntax is very easy to follow however it can be used more in depth:

## including local files

including local files is as follows:
```
use 'filename'
```
absolute paths are supported, and prefixes such as './' or '../' can be used.


## including inbuilt files

including inbuilt files can only be used if the `inb` compiler flag is true. if not,  `__err_assert()` will throw an error.

```
	use inb(flag)
```
some examples of `flag` are below:
```
__events
__config # this is automatically included.
```

## including modules or classes

if you dont want to keep typing out module names, you can explicitly tell the compiler you want to use a certain **module / class**.
```
	use "person"
	
	use Person.Jobs.Profession

	Profession p("programmer")

```