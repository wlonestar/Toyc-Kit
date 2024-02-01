# symbol tables for Syntax and semantic analysis

## variable declaration

VarTable: <<name>:<type>:<value(init=Default)>>

```c++
struct VariableTuple {
  std::string name;
  std::string type;
  std::string value;
};
```

## function declaration

<<name>:<return_type>:<parameters>>

```c++
struct FunctionTuple {
  std::string name;
  std::string returnType;
  std::vector<VariableTuple> parameters;
};
```
