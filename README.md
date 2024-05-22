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
| Exponentiation | `^` | Used for exponents, not bitwise XOR; Not working | 
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

### Built-in Functions
| Signature | Docs | 
| :---  | :----- |
| `print[a]` | Prints `a` to console |
| `input[a] -> string` | Prints `a` and then waits for user input, when the user presses enter, the function returns the string input. Using this function will cause a memory leak :(

### Syntax

#### Define Variables

```
let string = "plugh";
let number = 123.456;
let array = [ 1, 2, 3 ]; // Not working
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
## Examples

#### Hello world
```
print["Hello World!"];
```

#### Using Functions
```
let foo[a, b] = {
	print[a * b];
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
    print[i];
    i = i + 1;
};
```

#### Recursive Loop
```
let loop[a] = {
	print[a];
	if (a > 0) { loop[a-1]; };
};

loop[7];
```