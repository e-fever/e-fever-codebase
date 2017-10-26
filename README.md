A Collection of Project Wizard for Qt Creator
=============================================

It is a repository of a collection of Project Wizard used by E-Fever for its open source projects and in-house projects. 

![Image](docs/screenshot1.png)

Flux
----

[Flux Application(Simple)](Flux-Application-Simple)

Creates a simple Quick Flux based project with qpm packages

Specification

1. Quick Flux
2. Backtrace on crash (Linux/Mac only)
3. QPM

[Flux Application(Full)](Flux-Application-Full)


Library Project
-----

[Qt/Qml-Library-Project](Qt-Qml-Library-Project)

Creates a library project with Testable

Specification

 1. .travis.yml
 1. appveyor.yml
 1. Testable Unit Test 
 1. Backtrace on crash (Linux/Mac only)


Unit Test
----

[Tesable Unit Itest](Testable-Unit-Test)

Creates a Testable based unit test for C++ and QML features.

Specification

1. Qt Test
2. Qt Quick Test
3. Testable
4. Backtrace on crash (Linux/Mac only)
5. QPM packages

[Qt-Quick-Unit-Test](Qt-Quick-Unit-Test)

Creates a simple Qt Quick Test based unit test for a set of feature. Unit tests allow you to verify that the code is fit for use and that there have no regressions.

Specification

 1. Qt Quick Unit Test
 2. Backtrace on crash (Linux/Mac only)
 3. No QPM

Files:

```
.
├── %{ProjectName}.pro
├── main.cpp
└── tst_Sample.qml
```
