# bannertool

A tool for creating 3DS banners.

## How to build

```shell
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo # generate
cmake --build build                              # building (optional)
cmake --install build                            # local install (with building)
cmake --build build --target package             # packaging (with building)
```

## Releases

You can find automatically built versions of the tool for Windows (7+) and some
GNU/Linux distributions (glibc 2.39+) at the [Releases][Releases] section.

## History

Originally created and maintained by Steveice10.

### Notable changes:
 - changed build system to CMake
 - updated libraries for ogg vorbis and png file loading
 - added library for wave file loading for more supported formats

[Releases]: https://github.com/carstene1ns/3ds-bannertool/releases
