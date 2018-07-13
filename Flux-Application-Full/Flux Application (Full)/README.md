A QuickFlux Project Template
==========================

Prerequisite
--------

1. qpm
2. node/npm [Required for Mac deployment, but optional for another platform]

Features
--------

1. Quick Flux Project Skeleton - ActionTypes / Actions / Middlewares
2. Unit Tests
3. DMG (Mac), Snap(Linux) deployment scripts

File Structure:

```
.
├── %{ProjectName}.pro
├── README.md
├── app
│   ├── %{ProjectName}
│   ├── app.pro
│   ├── main.cpp
│   ├── qpm.json
│   └── qpm.pri
├── cpp
│   ├── appview.cpp
│   ├── appview.h
│   └── cpp.pri
├── deployment
│   ├── deployment.pri
│   ├── linux
│   │   ├── %{ProjectName}.desktop
│   │   ├── create_snap.sh
│   │   ├── docker-compose.yml
│   │   ├── icon.png
│   │   ├── linux.pri
│   │   ├── qt.docker
│   │   ├── qt5-launch
│   │   ├── run_docker.sh
│   │   └── snapcraft.yaml
│   └── mac
│       ├── Info.plist
│       ├── create_dmg.sh
│       ├── create_icns.sh
│       ├── dmg.json
│       ├── icon.icns
│       ├── icon.png
│       ├── increase-macos-build-number.sh
│       └── mac.pri
├── generator.json
├── package.json
├── qml
│   ├── %{Package}
│   │   ├── MainWindow.qml
│   │   ├── actions
│   │   │   ├── ActionTypes.qml
│   │   │   ├── Actions.qml
│   │   │   └── qmldir
│   │   ├── constants
│   │   │   ├── Constants.qml
│   │   │   └── qmldir
│   │   ├── main.qml
│   │   ├── middlewares
│   │   │   ├── SystemMiddleware.qml
│   │   │   └── qmldir
│   │   ├── stores
│   │   │   ├── MainStore.qml
│   │   │   ├── RootStore.qml
│   │   │   └── qmldir
│   │   └── uikit
│   │       └── qmldir
│   ├── qml.pri
│   └── qml.qrc
└── unittests
   ├── main.cpp
   ├── qmltests
   │   └── tst_QmlTests.qml
   ├── qpm.json
   ├── qpm.pri
   ├── snapshot.json
   ├── testcases.h
   ├── tests.cpp
   └── unittests.pro
```

Deplopyment
===========

DMG (Mac)
--------

```
npm install # Download appdmg , a tool for packing .dmg file
npm run mac_create_dmg
```

Snap (Linux)
-----

```
npm run linux_create_snap # or ./deployment/linux/run_docker.sh
```
