# `Urandom class`

`Urandom` is a simple wrapper around the `/dev/urandom` interface on Linux systems. The class also implements template specialization for generating floating point types.

## Common functions

```c++
Urandom urand;

urand.open(); // Open urandom File
urand.is_open(); // Returns the open status of the file
urand.close(); // Close the file

urand.get<unsigned int>(); //Read sizeof(unsigned int) number of random bytes

```

## TODO 

1. Floating point  template specialization.