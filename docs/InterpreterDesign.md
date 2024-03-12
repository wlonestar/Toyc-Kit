# Interpreter Design

**Currently load all std library firstly, ignore preprocessor stage.**

## Declaration

1. Variable: All see as local variable

```bash
> int a;
```

```c
void __wrapped_var_1() {
  int a;
}
```

---

```bash
> int b = 2;
```

```c
void __wrapped_var_2() {
  int b = 2;
}
```

2. Function: Same as compiler

```bash
> int add(int a, int b) { return a + b; }
```

```c
int add(int a, int b) { return a + b; }
```

## Statement

wrap into function body

```bash
> a = 1;
```

```c
void __wrapped_stmt_1() {
  a = 1;
}
/* ----> */
void __wrapped_compound_1() {
  int a;
  a = 1;
}
```

---

```bash
> if (a) { print("1"); } else { print("2"); }
```

```c
void __wrapped_stmt_2() {
  if (a) { print("1"); } else { print("2"); }
}
/* ----> */
void __wrapped_compound_2() {
  int a;
  a = 1;
  if (a) { print("1"); } else { print("2"); }
}
```

---

```bash
> a + 1;
```

ignore

---

```bash
> a++;
```

```c
void __wrapped_stmt_3() {
  a++;
}
// ...
```

## Expression

print its value

```bash
> a
```

```c
void __wrapped_print_1() {
  printi64ln(a);
}
```

```bash
> 1 + 2.0
```

```c
void __wrapped_print_2() {
  printf64ln(1 + 2.0);
}
```
 