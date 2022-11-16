# How to SVS
Well, there is not much documentation at the moment. I hope that this document and included examples will help you understand the SVS language.

### Quick access
* [Builtin functions](builtins.md)
* [Code examples](examples.md)


### Hello world
This is how Hello world in SVS look like:

    function main {
      # this is a comment
      print("Hello world program:");
      variable = "hello";
      variable = variable + " world";
      print(variable + "!");
    }
Or simply:

    function main {
      print("Hello world!");
    }

You can find more examples [here](examples.md) or as .svs files in the [Examples folder](../examples).

### Keywords
All keywords, variables and functions are case sensitive. Here is a table of all keywords (in alphabetical order).

| Keyword | Meaning |
| --- | --- |
| and | Logical and. |
| arg0, arg1 ... arg9 | Function arguments. |
| array | Initializes array. |
| break | Breaks the for or while loop. |
| end | Specifies the end of file. Not mandatory if EOF is present. |
| for | For loop. |
| function | Defines new function. |
| if, else | If and else, standard conditional branching. |
| local | Specifies local variable. |
| not | Logical negation. |
| or | Logical or. |
| return | Return from function. |
| sys | System function call |
| while | while loop. |

### Symbols
| Symbol | Meaning |
|--|--|
| "(" and ")" | Round brackets are used in math operations, function and *sys* calls |
| "{" and "}" | Curly brackets are used to enclosure a block of code. |
| ; | Command separation  |
| , | Function arguments separation. |
| = | Assign operation. |
| . | Decimal point for float constants. |
| " | Enclosures string constants. |
| # | Begin of single line comment. |
| #* | Begin of multi-line comment. |
| *# | End of multi-line comment. |

### Operators used in expressions

| Operator | Meaning |
|--|--|
| + | Addition for strings and numbers.|
| - | subtract |
| * | multiply |
| / | division |
| % | modulus |
| & | reference string operator |
| == | equal |
| != | not equal |
| <= | lower or equal |
| < | lower |
| >= | greater or equal |
| > | greater |

Note: If there is addition *+* between string and number, the number is converted to string and the two strings are added together.

Operators +, -, *, /, % can also be used in an variable assignment:

    x += 5; # equivalent to x = x + 5;
    y -= 17.0;
    z /= 5;
    ...
Note: SVS supports incrementing (decrementing) *num* type variables with ++ (- -), but only as a command outside expression.

This works:

    ...some code...
    x++;
    ...some code...

This does **NOT** work:

    ...some code...
    if (i == x++) {
    ...some code...

### Priority of operators
| Priority <br /> (highest to lowest.) | Operator(s) |
|--|--|
| 1. | not, & |
| 2. | *, /, % |
| 3. | +, - |
| 4. | + (between strings or strings and numbers) |
| 5. | ==, !=, <=, >=, <, > |
| 6. | and, or |

Operators of the same priority are executed from left to right. Operations inside brackets are executed first. Text reference operator (&) is handled while script is parsed to tokens.

### Types of values
| Type | Example | Description |
|--|--|--|
| num | a = 5; <br /> b = -97; <br /> c = 0x3ff;| 32 bit integer by default,<br /> can be writen in hexa.  |
| float | a = 6.22; <br /> b = 8.5; <br /> c = 0.0; | 32 bit float.* |
| string | a = "car";  <br /> b = "bed"; | String,<br />maximal length of string is limited by the string memory. |

#### Text constants allows these escape sequences
|Sequence| Meaning |
|--|--|
| \\n | newline |
| \\\\ | backslash |
| \\" | double quote |

Note: Before SVS version 1.1 pound (#) symbol inside text constant must be written as ##, otherwise rest of line is threated like a comment, bit of a design oversight.

Escape characters \\a,\\b,\\f,\\r,\\t,\\v are also supported since SVS version 1.2.1.

#### Operations across types
|  | string | num | float |
|--|--|--|--|
| string | +, ==, != | + | + |
| num |  | all operators|  |
| float |  |  | all operators, except the logic ones |

If you want to convert one type to another or get type of some data, you can use built-in functions of the SVS. More on that in ***Built-in functions*** chapter.

Conversion from *float* to *string* is NOT IEEE 754 compatible and  its results are only informational.

### Logic operators
Logic operators can be used only on num type, they also returns num type.

### Naming of functions and variables
Name of function (or variable) must start with a upper-case or lower-case letter or underscore symbol (_), then it may continue with another 13 (can be adjusted with NAME_LENGTH define) letters, numbers or underscores. Functions and variables do not share the same namespace.

### Variables
Variables are by default all global (accessible across different functions), and value of uninitialized variable is *undefined*. Keyword *local* allows to add local variable to the current block of code. Local variables are destroyed at the end of block.
**Note:** Up to SVS v. 1.2.0 uninitialized variables were set to type *num* and value 0 (zero). SVS 1.3 produces warning when uninitialized variable is used and in future releases this will result in an error.


    function main {
      a = 5;
      {
        local a;
        a = 8;
        {
          local a;
          a = "Some text";
          print("a = " + a);
        }
        print("a = " + a);
      }
      print("a = " + a);
    }
   Output:

    a = Some text
    a = 8
    a = 5
Maximum number of variables is also limited (VAR_TABLE_L define).

### Arrays
Since SVS version 1.0 implementation of arrays moved from sys warpper to the core language. There were no changes to the inner workings of SVS arrays, so they are still quite limited, but now the syntax is quite simple.
Arrays can be defined by array keyword:

    array a[50];
This will create an array named *a* with 50 cells.

    array b = ["alpha", "beta", "gama"];
This will create an array named *b* with three cells initialized with string values "alpha", "beta" and "gama".
For now arrays can't be destroyed. When the array memory does not allow creation of new array an error is thrown.
You can assign values to the cells of an array:

    a[7] = 123;
Or you can read value of cells in an expression:

    print("value: " + a[7]);

String identificator a (in this example), can be (since v.1.4) passed in expressions. In every case only identificator is passed, array is never duplicated.
Multidimensional arrays are not supported. String functions does not work on arrays because strings in SVS are not internally stored as arrays.

## Program flow controll

### Loops and branching
SVS supports simple branching with *if* and *else* and two types of loops.

#### If and else
Statement if must be followed by brackets containing an expression. If the expression is zero, the next command (or block of code) will be skipped, otherwise it will be executed. Else branch is optional.

*Note:*
Do not use "else if" construction in SVS versions bellow 0.8.7.

Example:

	if (1) {
	  print("true");
	}
	if ( b < 10) {
	  b = b + 1;
	} else {
	  b = 0;
	}
Note: Try to always use curly brackets.

#### While
While loop is done by *while* keyword, followed by expression inside brackets. Following command will be executed while the expression is not equal zero. You can stop the loop by using *break* statement anywhere inside the loop.

Example:

    while(1) {
      if (a == 5) {
        break;
      }
      a = a + 1;
      print("loop");
    }

    b = 5;
    while(b) {
      b = b - 1;
      print("loop 2");
    }

#### For
For loop is done by *for* keyword, followed by brackets containing statement as following:
for (< initialisation statement >; < expression >; < usually increment >;)
For will execute first statement once, evaluate expression and at the end of loop the increment/decrement statement is executed. For loop can be exited with *break* keyword.

Example:

    for (a = 10; a > 0; a--;) {
      print("loop");
    }
Note: Do not forget the semicolon at the end of second statement. It is required by the current version of SVS.

###  Functions
SVS functions can be called from another SVS function or from the script interpreter. SVS for command line calls function *main* (this behaviour can be changed).

    function main {
      hello();
      print("" + add(1, 1));
    }

    function hello {
      print("Hello");
      return;
    }

    function add {
      return arg0 + arg1;
    }
The output looks like:

    Hello
    2
Function must contain at least one command or empty command block to be valid.
Examples of minimal valid functions:

    function f1 {}
    function f2 return arg0 + 2;
    function f3 b = 2;


Maximum of functions in one file is limited by FUNCTION_TABLE_L define. Return command and return value are both optional.

### Built-in functions
SVS has a few built-in functions that can help you accomplish mainly type conversions and type checking. Conversion to string is not included, because is provided as a basic expression by SVS. If you define function with a same name as one of the built-in functions, then every call after the function definition will execute your function instead of the built-in one.

**List of builtin functions:**

[Full description of all the built-in fuctions](builtins.md)


    typeof(expression);
    num(expression);
    float(expr);
    isnum(expr);

    print([str]string);
    len(expr);
    getcp([str]string, [num]pos);
    substr([str]string, [num]begin, [num]end);
    instr([str]string, [str]sub_string);
    upper([str]string);
    lower([str]string);

    sin([float]angle_in_radians);
    cos([float]angle_in_radians);
    tan([float]angle_in_radians);
    atan([float]angle_in_radians);
    log([float]x);
    exp([float]x);
    pow([float]x, [float]y);
    pi();
    sqrt([float]x);
    rnd();

    gc([num] to_free);
    ver();
    dbg([num] level);