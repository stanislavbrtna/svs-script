# SVS Builtin functions

#### Type of

    typeof(expression);

Function *typeof()* returns type of its parameter.

    if (typeof(88) == TYPE_NUM) {
	  print("88 is a number")
    }
You can use defines *TYPE_NUM*, *TYPE_STR*, *TYPE_FLOAT* and *TYPE_ARRAY*, to identify the type of data.

#### Num

    num(expression);
Function *num()* accepts string or float and tries to convert it to the type *num*.

#### Float
    float(expr);
Function *float()* accepts string or num and tries to convert it to the type *float*.

#### Is Num
    isnum(expr);
Function isnum() takes a string and returns non-zero value if the string can be converted to num. Returns 1 if string can be converted to num, 2 if it can be converted to float. It also can take num or float type argument, in that case it returns 1 for num or 2 for float.

#### Len
    len(expr);
Function *len()* returns length of given string or an array.

    return len("abcd");
Result of this will be 4. Since v. 1.4 this function also returns lenght of given array.

#### Get char at posiotion (getcp)
    getcp([str]string, [num]pos);
Return: [str] char at given position

Function *getcp(string, position)* returns new string containing only one char that is at given position in a given string. If the position is invalid, then it returns empty string.

#### Get substring (substr)
    substr([str]string, [num]begin, [num]end);
Return: [str] substring

Function *substr* returns new string containing portion of a given string begining at index *begin* and ending at index *end*, both included. If the position is invalid, then it returns empty string.

#### Get if string contains string (instr)
    instr([str]string, [str]sub_string);
Return: [num] substr position

Function *instr*. If substring is contained in the string, 1 + sub_str position in the string is returned, otherwise zero.

#### Get upper case string (upper)
    upper([str]string);
Return: [str] upper cased string

Function *upper* converts strings to upper case.

#### Get lower case string (lower)
    lower([str]string);
Return: none

Function *lower* converts strings to lower case.

#### Print
    print([str]string);
Return: none

Function *print* prints its parameter on standard output. Since SVS version 1.1 print support second optional string argument that is used instead of the default newline character at the end of the printed string.

#### Get SVS version
    ver();
Return: [num] version

Since SVS 0.8.8 you can use *[num]ver()* to get current version of SVS. Version is returned as number, for example version 1.0.0 will return 1000, version 1.5.3 will be 1530.


### Built-in math functions
Since version 0.8.6 SVS contains optional built-in math functions. These functions are enabled by default with SVS_USE_ADV_MATH define.

#### Sin
    sin([float]angle_in_radians);
Return: [float] result
Function *sin* returns the sine of given angle.

#### Cos
    cos([float]angle_in_radians);
Return: [float] result
Function ** returns the cosine of given angle.

#### Tan
    tan([float]angle_in_radians);
Return: [float] result
Function *tan* returns the tangent of given angle.

#### Atan
    atan([float]angle_in_radians);
Return: [float] result
Function *atan* returns the arc tangent of given angle.

#### Log
    log([float]x);
Return: [float] result
Function *log* returns the natural logarithm of x.

#### Exp
    exp([float]x);
Return: [float] result
Function *exp* returns e^x.

#### Pow
    pow([float]x, [float]y);
Return: [float] result
Function *pow* returns x^y.

#### Pi
    pi();
Return: [float] result
Function *[float]pi()* returns the value of pi.

#### Sqrt
    sqrt([float]x);
Return: [float] result
Function *sqrt* returns the square root of x.

#### Rnd
    rnd();
Return: [num] result
Function *rnd()* returns random number.
Note: depends on implememtation of SVS_RND_FUNCTION

### Built-in system functions

#### Dbg
    dbg([num] level);
Result: none
Function *dbg* enables expression and command debug output.

#### GC
    gc([num] to_free);
Result: none
Function *gc([num] to_free)* performs garbage collection of unused strings. Argument to_free specifies how many chars will be collected, if zero is passed, full garbage collection occurs.

    gc([num] to_free);

When your program seemingly randomly runs out of string memory inside a function call, try calling garbage collection before that function call.

### SYS statement functions
SVS can be extended through its C function wrapper API, these functions can the be called with *sys* command.
Sys function call example:

    sys.test(1, 2, 3, 4, 5);

In SVS version 1.1.7 and below *sys* statement uses different syntax. This variant of *sys* function call is still supported in current version of SVS interpreter.

    # in versions 1.1.7 and below
    sys test(1, 2, 3, 4, 5);

#### Useful sys commands in the default  wrapper
**Debug:**

    sys.profiler([num] enable); # Enables garbage collector profiling
    sys.dbgCache([num] enable); # Enables token cache debug output

**Info:**
Prints information about SVS interpreter.

    sys.info();