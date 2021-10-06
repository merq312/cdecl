# cdecl

Parses any C declaration

## Sample output

```
$ int num
$ num is int
```

```
$ char arg[]
$ arg is array[] of char
```

```
$ void myFunc(int num)
$ myFunc is function taking (int a) returning void

```

```
$ char** arr[]
$ arr is array[] of pointer to pointer to char

```

```
$ void (*signal(int sig, void (*func)(int)) ) (int)
$ signal is function taking (int sig, void (*func)(int)) returning pointer to function taking (int) returning void
```
