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
| Return | `return` | Not working |
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


### Syntax

#### Define Variables

```
let string = "plugh";
let number = 123.456;
let array = [ 1, 2, 3 ]; // Not working
```

#### Define Function

 - All functions MUST be defined at the top of a file.
 - The order functions are defined does matter, so in this example `funcName` could be used in `funcName2` but not the other way around

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
## Examples

#### Hello world
 - When transpilied to C, `<stdio.h>` is imported for you so you have accsess to all of those functions.
```
printf["Hello World!"];
```

#### Using Functions
```
let foo[a, b] = {
	printf["%i\n", a * b];
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

#### Recursive Loop
```
let loop[a] = {
	printf["I is: %i\n", a];
	if (a > 0) { loop[a-1]; };
};

loop[7];
```