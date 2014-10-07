# Install
This section describes how to easy install Blackhole on your computer. This doesn't involves much work, because the library is header-only.

## Header Only
In the context of the C or C++ programming languages, a library is called header-only if the full definitions of all macros, functions and classes comprising the library are visible to the compiler in a header file form. Header-only libraries do not need to be separately compiled, packaged and installed in order to be used. Another advantage is that the compiler's optimizer can do a much better job when all the libraries source code is available.

The disadvantages include:

 * brittleness – most changes to the library will require recompilation of all compilation units using that library
 * longer compilation times – the compilation unit must see the implementation of all components in the included files, rather than just their interfaces
 * code-bloat (this may be disputed) – the necessary use of inline statements in non-class functions can lead to code bloat by over-inlining.

Blackhole includes much dark template magic inside, so including the definitions in header is the only way to compile, since the compiler needs to know the full definition of the templates in order to instantiate.

## Manually
You can easily install Blackhole manually by switching to the project's root and typing:

~~~ bash
git clone https://github.com/3Hren/blackhole.git
cd blackhole
cmake
sudo make install
~~~

## Packaging

### Debian Packages

### RHEL Spec

## Includes

## Configuring

## Platforms And Dependencies
