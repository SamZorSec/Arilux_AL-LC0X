libtest is only here so we can unit test our library dir.

To run the tests make sure you have a working build environment with cmake

```
# From base directory
cd libtest
mkdir -p build
cd build
cmake ../
make
./tests
```
