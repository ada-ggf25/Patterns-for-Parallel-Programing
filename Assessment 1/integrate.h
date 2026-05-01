// Assignment 1 — shared declarations.
#pragma once

// Integrand — do not modify the cost structure. Variable per-iteration cost
// is deliberate: it's what makes schedule choice interesting.
double f(double x);

// Reference serial implementation in integrate_serial.cpp — do not modify.
double integrate_serial(double a, double b, long n);

// Student-authored parallel implementation in integrate.cpp.
// MUST return the same value as integrate_serial to within smart_diff tolerance.
double integrate_parallel(double a, double b, long n);
