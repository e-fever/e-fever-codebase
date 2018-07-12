QuickFlux Project Template
==========================

That is a project template to simplify the creation of QuickFlux application with unit tests.

It includes:

1. qpm.json - Install QuickFlux via qpm
2. The template of ActionTypes and AppActions
3. Templates of Store
4. AppView C++ Class - Initialize QML environment and listen to a message from AppDispatcher
5. Unit Tests  using the Testable framework
6. Script to build Mac DMG and Linux snap package

File Structure:

```
.
├── LICENSE
├── README.md
├── app
│   └── quickfluxapp
│       ├── App
│       │   ├── actions
│       │   │   ├── ActionTypes.qml
│       │   │   ├── AppActions.qml
│       │   │   └── qmldir
│       │   ├── adapters
│       │   │   ├── StoreAdapter.qml
│       │   │   └── qmldir
│       │   ├── constants
│       │   │   ├── Constants.qml
│       │   │   └── qmldir
│       │   ├── stores
│       │   │   ├── MainStore.qml
│       │   │   └── qmldir
│       │   └── storeworkers
│       │       ├── MainStoreWorker.qml
│       │       ├── StoreWorker.qml
│       │       └── qmldir
│       ├── README.md
│       ├── app.qrc
│       ├── appview.cpp
│       ├── appview.h
│       ├── deployment.pri
│       ├── main.cpp
│       ├── main.qml
│       ├── qpm.json
│       ├── quickfluxapp.pri
│       └── quickfluxapp.pro
└── tests
    └── quickfluxappunittests
        ├── main.cpp
        ├── qmltests
        │   └── tst_QmlTests.qml
        ├── qpm.json
        ├── quickfluxappunittests.pro
        ├── tests.cpp
        └── tests.h

```
