#ifndef _MACRO_VARARG_H
#define _MACRO_VARARG_H

/* This file contains exclusively preprocessor directives for facilitating
 * variadic macros. The usage is fairly simple: define any callable (macro
 * or function) with the number of arguments as a suffix--for example:
 *
 *     #define do_thing2(a, b) ((a)+(b))
 *     int do_thing3(int a, int b, int c) {return a+b+c;}
 *
 * Then, define the actual macro to use MVA_DISPATCH:
 *
 *     #define do_thing(...) MVA_DISPATCH(do_thing, __VA_ARGS__)
 *
 * Things should just work after that.
 *
 * This library has the following caveats:
 * - A zero-argument call is not possible without GCC extensions.
 * - The maximum is presently 8 arguments, but can be extended.
 * - If the argument number is unsupported (read: undefined), you'll get an
 *   illegal symbol compiler error.
 */

#define _MVA_CONCAT(a, b) a##b
#define MVA_CONCAT(a, b) _MVA_CONCAT(a, b)
#define MVA_ARGN(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define MVA_NARGS(...) MVA_ARGN(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define MVA_DISPATCH(base, ...) MVA_CONCAT(base, MVA_NARGS(__VA_ARGS__))(__VA_ARGS__)

#endif
