# How to SVS
Well, there is not much documentation at the moment.
### Hello world
This is how Hello world in SVS look like:

    function main {
      # this is a comment
      sys print("Hello world program:");
      variable = "hello";
      variable = variable + " world";
      sys print(variable + "!");
    }

 ### Keywords
All keywords, variables and functions are case sensitive. Here is a table of all keywords (in alphabetical order).
|Keyword| Meaing  |
|--|--|
|arg0, arg1 ... arg9| Function arguments.
|end|Specifies the end of file. Not mandatory if EOF is present.
|for|For loop.
|break|Breaks the for or while loop.
|function|Defines new function.
|if, else|If and else, standard conditional branching.
|local|Specifies local variable.
|return|Return from function.
|sys|System function call
|while|while loop.

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
| + | Addition for strings and numbers. If there is addition between string and number, the number is converted to string and the two strings are added together. |
| - | subtract |
| * | multiply |
| / | division |
| % | modulus |
| == | equal |
| != | not equal |
| <= | lower or equal |
| < | lower |
| >= | greater or equal |
| > | greater |

### Operators priority
| Priority <br /> (highest to lowest.) | Operator(s) |
|--|--|
| 1. | *, /, % |
| 2. | +, - |
| 3. | + (between strings or strings and numbers) |
| 4. | ==, !=, <=, >=, <, > |

Operators of the same priority are executed from left to right. Operations inside brackets are executed first.

### Types of values
| Type | Example | Description |
|--|--|--|
| int | a = 5; <br /> b = -97; <br /> c = 0x3ff;| 32 bit integer by default,<br /> can be writen in hexa.  |
| float | a = 6.22; <br /> b = 8.5; <br /> c = 0.0; | 32 bit float.* |
| string | a = "car";  <br /> b = "bed"; | String,<br />maximal length of string is limited by the string memory. |

If you need to use # symbol in a text constant, you need to write it twice (##).
#### Text constants allows these escape sequences
|Sequence| Meaning |
|--|--|
| \\n | newline |
| \\\\ | backslash |
| \\" | double quote |
| ## | # |

#### Operations across types
|  | string | int | float |
|--|--|--|--|
| string | +, ==, != | + | + |
| int |  | all operators|  |
| float |  |  | all operators |

Conversion from *float* to *string* is NOT IEEE 754 comatible and  its results are only informational.

### Where are logic operators?
At the moment, there are no logic operators, use +, * and a lot of brackets instead.

### Naming of functions and variables
Name of function (or variable) must start with a upper-case or lower-case letter or underscore symbol (_), then it may continue with another 13 (can be adjusted with NAME_LENGTH define) letters, numbers or underscores. Functions and variables do not share the same namespace.

###  Functions
SVS functions can be called from another SVS function or from the script interpreter. SVS for command line calls function *main* (this behaviour can be changed).

    function main {
      hello();
      sys print("" + add(1, 1));
    }

    function hello {
      sys print("Hello");
      return;
    }

    function add {
      return arg0 + arg1;
    }
The output looks like:

    Hello
    2

Maximum of functions in one file is limited by FUNCTION_TABLE_L define. Return command and return value are both optional.
### Variables
Variables are by default all global (accessible across different functions), and value of uninitialised variable is (*int*) 0.
Keyword *local* allows to add local variable to the current block of code. Local variables are destroyed at the end of block.

    function main {
      a = 5;
      {
        local a;
        a = 8;
        {
          local a;
          a = "Some text";
          sys print("a = " + a);
        }
        sys print("a = " + a);
      }
      sys print("a = " + a);
    }
   Output:

    a = Some text
    a = 8
    a = 5
Maximum number of variables is also limited (VAR_TABLE_L define).

 ### Loops and branching
SVS supports simple branching with *if* and *else* and two types of loops.

#### If and else
Statement if must be followed by brackets containing an expression. If the expression is zero, the next command (or block of code) will be skipped, otherwise it will be executed. Else branch is optional.

Do not use "else if" construction, it does not work in current version of SVS.

Example:

	if (1) {
	  sys print("true");
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
      sys print("loop");
    }

    b = 5;
    while(b) {
      b = b - 1;
      sys print("loop 2");
    }

#### For
For loop is deone by *for* keyword, followed by brackets conatining statement as following:
for (< initialisation statement >; < expression >; < usually increment >;)
For will execute first staement once, evaluate expression and at the end of loop the increment/decrement statement is executed. For loop can be exited with *break* keyword.

Example:

    for (a = 10; a > 0; a = a - 1;) {
      sys print("loop");
    }
Note: Do not forget the semicolon at the end of second statement. It is required by the current version of SVS.

