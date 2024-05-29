# Eso Lang 3

A small language that transpiles to C.

## Usage
 - Must have gcc installed
```
> make
> .\es3 input.es3 out
> .\out.exe
Hello World!
```


## Docs

### Symbols

| Name | Value | Notes | 
| :--- | :--- | :----- |
| Equals | `=` | Used for assignment not comparison |
| End Line | `;` | 
| Addition | `+` |
| Subtraction | `-` | 
| Multiplication | `*` | 
| Division | `/` | 
| Exponentiation | `^` | Used for exponents, not bitwise XOR | 
| Definition | `let` |
| Condititional | `if` |
| Loop | `while` |
| Return | `return` |  |
| Comparison | `==` | 
| Greater Than | `>` | 
| Less Than | `<` |
| Greater Than or Equal to | `>=` | 
| Less Than or Equal to | `<=` | 
| Begin Parenthasis | `(` |
| End Parenthasis | `)` |
| Begin Code Block | `{` |
| End Code Block | `}` |
| Begin Array | `[` |
| End Array | `]` |
| Array Seperator | `,` |
| True | `true` |
| False | `false` |

 - A `#` will ignore all characters untill the next newline 

### Built-in Functions
| Signature | Docs | 
| :---  | :----- |
| `print[a]` | Prints `a` to the console |
| `input[a] -> string` | Prints `a` and then waits for user input, when the user presses enter, the function returns the string input. Using this function will cause a memory leak :( |
| `sqrt[a] -> number` | Gets the square root of a number |
| `println[a]` | Prints `a` and a newline to the console |

### Syntax

#### Define Variables

```
let string = "plugh";
let number = 123.456;
let bool = true;
let array = [ 1, 2, 3 ];
```

#### Define Function

 - All functions MUST be defined at the top of a file.

```
let funcName[param1, param2, param3] = {
	...
};

let funcName2[] = {
	...
}

...
```

#### If Statement
```
if (a > b) {
	...
};
```

#### While Statement
```
while (a > b) {
	...
};
```

#### Arrays
```
let a = [1, 2, 3];

println[a];
# [1, 2, 3]

println[a{0}];
# 1
println[a{1}];
# 2
println[a{2}];
# 3

a{0} = "Hello";

println[a];
# ["Hello", 2, 3]

a{1} = [4, 5, 6];

println[a];
# ["Hello", [4, 5, 6], 3]
```


## Examples

#### Hello world
```
print["Hello World!\n"];
```

#### Using Functions
```
let foo[a, b] = {
	println[a * b];
};

let bar[c, d] = {
	if (c > d) {
		foo[c, c];
	};
	if (d > c) {
		foo[d, d];
	};
};

bar[7, 12];
bar[4, 3];
```

#### Iterative Loop
```
let i = 0;
while (i < 20) {
	println[i];
	i = i + 1;
};
```

#### Recursive Loop
```
let loop[a] = {
	println[a];
	if (a > 0) { loop[a-1]; };
};

loop[7];
```