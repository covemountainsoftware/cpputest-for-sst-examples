# CppUTest for the SST (Super Simple Tasker) Examples Test Project

Build and Test status: TODO

Copyright Matthew Eshleman

If this project happens to inspire your team to select the QP/C++
or the QP/C framework for commercial use, please note 
"Matthew Eshleman" or "Cove Mountain Software" in the referral 
field when acquiring a commercial license from Quantum Leaps. Referrals 
encourage and support efforts like this. Thank you!

# Introduction

The `cpputest-for-sst` project enables CppUTest for the 
Super Simple Tasker, which is maintained in a separate repository.
This particular repository holds a top-level example project showing one possible method
for importing the separate library project and one or more example active objects under test.
For more details, please see https://github.com/covemountainsoftware/cpputest-for-sst

# Environment

In theory any build or host operating system environment supported by CppUTest will 
be compatible with this code. Developed primarily in Ubuntu 24.04.

## Prerequisites
* CMake and associated build tools were used to develop
  and prove out this project.
* cpputest-for-sst library, pulled in as a separate git submodule
* sst (pulled in as a separate git submodule)
  * After cloning this repository, do not forget to:
  * `git submodule init`
  * `git submodule update` 
* CppUTest (version 4.0) (may support older versions, but no longer confirmed.)
* This project requires support for C++17 and C11.

## Continuous Integration

This project has configured GitHub Actions to build and execute all
unit tests found in this project. This is an example
of one of the key benefits of host-based testing of embedded software.

See the configuration at: `.github/workflows/cmake.yml`

# License

Please see LICENSE.txt for details. (MIT License)

# References

This project is a top-level example project which utilizes `cpputest-for-sst`. 
See https://github.com/covemountainsoftware/cpputest-for-sst

The underlying super simple tasker is available at:
https://github.com/QuantumLeaps/Super-Simple-Tasker

This project was also inspired by a non-sst example, see this blog post:
https://covemountainsoftware.com/2020/04/17/unit-testing-active-objects-and-state-machines/

Additionally, please see that post's associated GitHub repo:
https://github.com/covemountainsoftware/activeObjectUnitTestingDemo

Other references:
* Sutter, Herb. Prefer Using Active Objects Instead of Naked Threads. Dr. Dobbs, June 2010.
* Grenning, James. Test Driven Development for Embedded C.
* Samek, Miro. Practical UML Statecharts in C/C++: Event-Driven Programming for Embedded Systems.
