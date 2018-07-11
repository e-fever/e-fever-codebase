unix:!android {
    isEmpty(target.path) {
        qnx {
            target.path = /tmp/$${TARGET}/bin
        } else {
            target.path = /opt/$${TARGET}/bin
        }
        export(target.path)
    }
    INSTALLS += target
}

export(INSTALLS)

include($$PWD/mac/mac.pri)

mac {
    # Quick and dirty way to create
    APP=$${OUT_PWD}/$${TARGET}.app
    DMG_FOLDER=$${OUT_PWD}/dmg

    DISTRIBUTION_CN=""
    INSTALLER_CN=""

    dmg.target = dmg
    dmg.commands = cp $${PWD}/dmg.json $${OUT_PWD}; appdmg $${OUT_PWD}/dmg.json %{ApplicationProjectName}.dmg
    dmg.depends = $${APP} sign

    sign.target = sign
    sign.path = $${APP}/Contents/_CodeSignature/CodeResources
    sign.commands = dsymutil $${APP}/Contents/MacOS/$${TARGET} -o  $${TARGET}.dSYM; \\
                   macdeployqt $${APP} -verbose=1 -qmldir=$${PWD}/%{ApplicationProjectName} -appstore-compliant; \\
                   cp $${ROOTDIR}/assets/plugins/libqcocoa.dylib $${APP}/Contents/PlugIns/platforms/ ; \\
                   find $${APP} -name "\\"*.dSYM\\"" -exec rm -rf {} \\; ; \\
                   codesign --deep -s "\\"$${DISTRIBUTION_CN}\\"" -f --entitlements $${PWD}/entitlement.plist $${APP};
    sign.depends = $${APP}

    pkg.target = pkg
    pkg.path = $${TARGET}.pkg
    pkg.commands = productbuild --component $${APP} /Applications --sign "\\"$${INSTALLER_CN}\\"" $${TARGET}.pkg
    pkg.depends = $${APP}

    QMAKE_EXTRA_TARGETS += dmg sign pkg
}
