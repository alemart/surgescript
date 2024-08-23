# Release Notes

## 0.6.1 - August 23rd, 2024

* Introduced switch statements
* Made optional the expressions in for loops
* Added an optional message to assertions
* Bugfix in continue statements
* Updated docs
* General improvements

## 0.6.0 - May 17th, 2024

* Optimized function calls with self-modifying SurgeScript bytecode that recognizes opportunities for optimization
* Optimized tag tests with faster hashes and bitsets
* Optimized the allocation and the deallocation of strings with a pool of managed strings
* Introduced do-while loops
* Changed `Math.mod()` so that it returns the modulo instead of the remainder
* Made various updates and additions to the standard library
* Updated the documentation
* Updated the CLI
* Added support for reading scripts in virtual files
* Removed support for emoticons
* General improvements to the code. Bugfixes

## 0.5.6.1 - September 22nd, 2022

* Tweaks to the build system

## 0.5.6 - September 1st, 2022

* Improved the SurgeScript CLI with a time limit option, the ability to run scripts from stdin and optional multithreading support
* Added Visual Studio support (Cody Licorish)
* Added Emscripten support
* Updated docs
* General improvements

## 0.5.5 - January 22nd, 2021

* Added the ability to pause the SurgeScript VM
* Added utility macros for checking the SurgeScript version at compile time
* Introduced a dedicated module for keeping track of time
* Renamed Object.childCount to Object.__childCount
* Updated docs

## 0.5.4.4 - April 16th, 2020

* Added LIB_SUFFIX compilation option
* Updated docs

## 0.5.4.3 - February 16th, 2020

* SurgeScript is now available as a shared library
* Added pkg-config files to make it easy to link a program to libsurgescript
* Improved interoperability with C++
* Included AppStream metadata for Linux and for the free software ecosystem
* Built-in iterables and iterators are now tagged "iterable" and "iterator", respectively

## 0.5.4.2 - January 5th, 2020

* Added support for UTF-8 filenames on Windows

## 0.5.4.1 - December 31st, 2019

* Included Transform utilities
* Updated the documentation
* Added Math.lerpAngle(), Math.deltaAngle()
* General improvements

## 0.5.4 - September 23rd, 2019

* Performance optimizations
* Improved the built-in pseudo-random number generator
* Introduced the @Package annotation
* Trailing commas are now accepted when declaring Arrays & Dictionaries
* Added tree traversal routines: Object.findObjects() and similar others
* Added Object.__arity(), Math.signum(), Time.now
* New option flags for the parser
* Updated the documentation
* Bugfixes and general improvements

## 0.5.3 - March 20th, 2019

* Introduced the remainder operator (%)
* Introduced the 'caller' keyword
* Introduced the 'readonly' keyword
* Improved the 'timeout' feature
* Added the 'assert' feature
* Added Object.__file, Math.NaN
* Added utility functions for Arrays and Strings
* Removed Transform2D from the core lib
* The parser now accepts option flags
* Bugfixes and performance optimizations
* Updated documentation
* General improvements

## 0.5.2 - May 31st, 2018

* Introduced the Plugin system
* Introduced the Date object for date and time routines
* Now representing numbers as 64-bit floating point values
* Performance optimizations
* Added Object.__invoke()
* Updated the naming convention of getters/setters
* Bugfixes and general improvements

## 0.5.1 - May 5th, 2018

* SurgeScript now compiles as a static library
* Introduced the <surgescript.h> header for ease of use
* Improved compiling-related data
* Improved documentation
* Small improvements to the language

## 0.5.0 - April 15th, 2018

* Initial release
