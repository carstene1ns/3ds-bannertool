# bannertool

A tool for creating 3DS banners.

## How to build

```shell
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo # generate
cmake --build build                              # building (optional)
cmake --install build                            # local install (with building)
cmake --build build --target package             # packaging (with building)
```
