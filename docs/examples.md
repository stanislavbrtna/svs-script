# Example code snipplets
Here are some basic examples. They are also included as svs files in the examples folder.

## Hello world
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

Or even simpler:

    print("Hello world!");

## Usefull things

### Random string of given lenght

    function rnd_string #*lenght*# {
      arg1 = "";
      for(arg2 = 0; x < arg0; arg2++;) {
        arg1 += getcp("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789_!@$%&*",
                        rnd() % len("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789_!@$%&*"));
      }
      return arg1;
    }

Pro tip: You can use the unused function arguments as variables to free some of the precious VAR_TABLE_L realestate.

### Array print

    function array_print #* array *# {
      print("Array:");
      for (arg1 = 0; arg1 < len(arg0); arg1++;) {
        print("  " + arg1 + ": " + arg0[arg1]);
      }
    }

### Array sort

    function array_sort_str #* array, optional: sortstring *# {
      if(arg1 == 0) {
        arg1 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      }
      arg2 = 0;
      for(arg3 = 1; arg3 < len(arg1); arg3++;) {
        arg4 = getcp(arg1, arg3);
        for(arg5 = arg2; arg5 < len(arg0); arg5++;){
          if (getcp(arg0[arg5], 1) == arg4) {
              arg6 = arg0[arg5];
            arg0[arg5] = arg0[arg2];
            arg0[arg2] = arg6;
            arg2++;
          }
        }
      }
    }


## Basic stuff

### 99 bottles
    
    bottles = 99;

	while(bottles > 0) {
	  print( bottles + " bottles of beer on the wall.");
	  print("Take one down and pass it around.");
	  bottles = bottles - 1;
	  print( bottles + " bottles of beer on the wall.");
	}

### FizzBuzz

    function main {
      for(i = 1; i < 200; i++;){
        text = "";
        if (i % 3 == 0) {
            text = "Fizz ";
        }
        if (i % 5  == 0) {
          text = text + "Buzz";
        }
        if (text == "") {
           text = "" + i;
        }
        print(text);
      }
    }

### Loops
The *break* statement can be used both in for and in while loop. 

#### For

    function main {
      for(a = 10; a > 0; a = a - 1;) {
        print("loop, a = " + a);
      }
    }

#### While
    function main {

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
    }

### Functions
Functions, arguments and return values.

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

### If else

    function main {
      if (1) {
        print("true");
      }
      if ( b < 10) {
        b = b + 1;
      } else {
        b = 0;
      }
    }

### Variables

    function main {
      a = 5;
      {
        local a;
        a = 8;
        {
          # another local variable initialized inside a block
          local a = "Some text";
          print("a = " + a);
        }
        print("a = " + a);
      }
      print("a = " + a);
    }

