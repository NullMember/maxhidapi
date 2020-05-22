# max-hidapi
Cycling '74 Max wrapper for the hidapi

# What is this?
This external is wrapper for the [libusb/hidapi](https://github.com/libusb/hidapi) which provides cross platform hid communication api. Maxhidapi is low-level external compared to most Max objects. Please be careful, [writing random data to your HID devices can break them.](https://github.com/libusb/hidapi/issues/105)

# Why?
I want to use my 8bitdo N30 Pro 2 device with Max because it has Nintendo Pro Controller mode and 6-axis MEMS sensor. I love MEMS sensors! That's why I wrote this external.

# Where? 
[Go to releases.](https://github.com/NullMember/max-hidapi/releases)

# Known Limitations:
Take a look at [libusb/hidapi/issues](https://github.com/libusb/hidapi/issues) for limitations. There are lot of them. Low-level communication with HID devices ([mice and keyboards for example](https://github.com/libusb/hidapi/issues/136#issuecomment-576044796)) mostly prevented by operating systems because of security.

# Building maxhidapi
This project uses [multiarch/crossbuild](https://github.com/multiarch/crossbuild) to build cross platform binaries with a simple makefile. If you want to build this project without [multiarch/crossbuild](https://github.com/multiarch/crossbuild) check instructions below.

## Windows target
On Windows machines I've used mingw but cygwin should work too. Either clang or gcc, no matter. Check Makefile to used options for compiler. Don't forget to change "cc" to your compiler path/name and put c74support from max sdk into libs/max folder.

## MacOS target
On MacOS machines you can install either Xcode or Command Line Tools for Xcode. Check Makefile to used options for clang. Don't forget to change "cc" to "clang" and put c74support from max sdk into libs/max folder.  
Note: You need to manually edit "ext_path.h" file in libs/max/c74support/max-includes folder. Find "#include <Files.h>" line and replace it by "#include <Carbon/Carbon.h>". I cannot figure out how to automate it (damn!).

# Use package
First copy externals folder into Package/hidapi folder then copy Package/hidapi folder to any of the Max search paths. By default you can use Documents/Max 7 (or 8)/Packages folder.