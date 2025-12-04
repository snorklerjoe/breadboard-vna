// Some utilities for complex number math
#ifndef COMPLEX_MATH_H
#define COMPLEX_MATH_H

// Math helpers
// Complex Number struct
typedef struct {
    double a, b;  // Real, Imaginary (rectangular form)
} double_cplx_t;

// Complex number constants
#define cplx_unity (double_cplx_t){1.0, 0.0}  // Just 1
#define cplx_zero (double_cplx_t){0.0, 0.0}   // Just 0

// Macros for complex number arithmetic
#define cplx_div(x, y) (double_cplx_t) {(x.a*y.a + x.b*y.b) / (y.a*y.a + y.b*y.b), (y.a*x.b - x.a*y.b) / (y.a*y.a + y.b*y.b)}
#define cplx_mult(x, y) (double_cplx_t) {(x.a*y.a)-(x.b*y.b), (x.a*y.b + x.b*y.a)}
#define cplx_add(x, y) (double_cplx_t) {(x.a+y.a), (x.b+y.b)}
#define cplx_sub(x, y) (double_cplx_t) {(x.a-y.a), (x.b-y.b)}
#define cplx_scale(x, s) (double_cplx_t) {s*x.a, s*x.b}

#define cplx_mag(num) (double) sqrt(num.a*num.a + num.b*num.b)
#define cplx_ang(num) (double) atan2(num.b, num.a)
#define cplx_ang_deg(num) (double) 180.0/MATH_PI*atan2(num.b, num.a)


#endif