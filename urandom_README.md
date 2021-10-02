# `Urandom class`

`Urandom` is a simple wrapper around the `/dev/urandom` interface on Linux systems. The class also implements template specialization for generating floating point types.

## Why not use `std::random_device` ?

`std::random_device` is not guarenteed to use a non-deterministic source and other problems.
Read more: https://www.pcg-random.org/posts/cpps-random_device.html


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