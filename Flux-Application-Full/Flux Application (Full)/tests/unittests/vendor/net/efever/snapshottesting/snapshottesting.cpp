#include <QDebug>
#include <QtShell>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStack>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QBuffer>
#include <QTest>
#include <asyncfuture.h>
#include "snapshottesting.h"
#include <private/qqmldata_p.h>
#include <private/qqmlcontext_p.h>
#include <private/snapshottesting_p.h>
#include <aconcurrent.h>
#include <functional>
#include <QQuickItemGrabResult>
#include <QOpenGLFunctions>

using namespace SnapshotTesting;
using namespace SnapshotTesting::Private;

/* For dtl */
using namespace std;
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <QFontDatabase>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QImageWriter>
#include <QPainter>
#include "dtl/Sequence.hpp"
#include "dtl/Lcs.hpp"
#include "dtl/variables.hpp"
#include "dtl/functors.hpp"
#include "dtl/Ses.hpp"
#include "dtl/Diff.hpp"

/** Terminology
 *
 * Class - C++ Class
 *
 * Component - QML Component
 *
 */

/* Options */

static QString m_snapshotFile;
static QString m_screenshotImagePath;
static QVariantMap m_snapshots;
static bool m_snapshotsDirty = false;
static bool m_interactiveEnabled = true;
static bool m_ignoreAllMismatched = false;
static bool m_acceptAllMismatched = false;
static QStringList m_qtInternalContextUrls;

/* End of Options */


static QStringList knownComponentList;
static QMap<QString,QString> classNameToComponentNameTable;

/// The default values of components
static QMap<QString, QVariantMap> classDefaultValues;

/// A list of ignored properties according to the class of the component
static QMap<QString, QStringList> classIgnoredProperties;

/// A list of ignored properties according to the package and component name
static QMap<QString, QStringList> componentIgnoredProperties;

/// List of data type should not be processed in term of their meta type id
static QList<int> forbiddenDataTypeList;

std::function<QImage(const QImage&, const QImage&)> m_screenshotImageCombinator;

#define DEHYDRATE_FONT(dest, property, original, current, field) \\
    if (original.field() != current.field()) { \\
        dest[property + "." + #field] = current.field(); \\
    }

static QString removeDynamicClassSuffix(const QString &name) {
    QString res = name;
    QStringList list;

    list << "_QML_[0-9]+$" << "_QMLTYPE_[0-9]+$";

    foreach (QString pattern, list) {
        QRegExp rx(pattern);

        if (rx.indexIn(res) >= 0) {
            res = res.replace(rx, "");
        }
    }

    return res;
}

static QString obtainClassName(QObject* object) {
    const QMetaObject* meta = object->metaObject();
    return meta->className();
}

/// Obtain the class name of QObject which is known to the system
static QString obtainKnownClassName(QObject* object) {
  const QMetaObject* meta = object->metaObject();
  QString res;

  while (!classNameToComponentNameTable.contains(res) && meta != 0) {
      res =  SnapshotTesting::Private::classNameToComponentName(meta->className());
      meta = meta->superClass();
  }

  return res;
}

static QList<QmlType> obtainQmlTypeList(bool all = true) {
    QList<QmlType> ret;

    auto createQmlType = [](const QQmlType* ty) {
        QmlType item;
        item.elementName = ty->elementName();
        item.meta = ty->metaObject();
        item.isCreatable = ty->isCreatable();
        item.module = ty->module();
        item.majorVersion = ty->majorVersion();
        item.minorVersion = ty->minorVersion();
        item.isNull = false;
        if (ty->metaObject()) {
            item.className = ty->metaObject()->className();
        }
        return item;
    };

#if (QT_VERSION < QT_VERSION_CHECK(5,9,2))
    QList<QQmlType*> types;

    if (all) {
        types = QQmlMetaType::qmlAllTypes();
    } else {
        types = QQmlMetaType::qmlTypes();
    }

    foreach (const QQmlType *ty, types) {
       ret << createQmlType(ty);
    }
#else
    QList<QQmlType> types;

    if (all) {
        types = QQmlMetaType::qmlAllTypes();
    } else {
        types = QQmlMetaType::qmlTypes();
    }

    foreach (const QQmlType ty, types) {
        ret << createQmlType(&ty);
    }

#endif

    return ret;
};

static const QmlType findQmlType(const QMetaObject* meta, bool all = true) {
    QmlType ret;
    ret.isNull = true;

    QList<QmlType> types = obtainQmlTypeList(all);

    foreach (const QmlType ty, types) {
        if (ty.meta == meta) {
            ret = ty;
            ret.isNull = false;
            break;
        }
    }

    return ret;
}

/// Copy from QImmutable project
static void assign(QVariantMap &dest, const QObject *source)
{
    const QMetaObject* meta = source->metaObject();

    for (int i = 0 ; i < meta->propertyCount(); i++) {
        const QMetaProperty property = meta->property(i);
        QString p = property.name();

        QVariant value = source->property(property.name());
        dest[p] = value;
    }
}

static bool inQtInternalContextUrls(const QUrl& url) {
    return m_qtInternalContextUrls.indexOf(QtShell::dirname(url.toString())) >= 0;
};

/*
static void printContextList(QList<QQmlContext*> list) {
    for (int i = 0 ; i < list.size();i++) {
        qDebug() << list[i]->baseUrl();
    }
}
*/

static QList<QQmlContext*> filterContextWithNullBaseUrl(QList<QQmlContext*> list) {
    QList<QQmlContext*> res;
    for (int i = 0 ; i < list.size();i++) {
        if (list[i]->baseUrl().isEmpty())
            continue;
        res << list[i];
    }
    return res;
}

QByteArray SnapshotTesting::Private::toBase64(const QImage& image) {

    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QImageWriter writer(&buffer, "PNG");
    writer.write(image);

    return buffer.data().toBase64();
}

static void saveScreenshotImage(const QString& name, const QImage& image) {
    if (image.isNull()) {
        return;
    }

    QtShell::mkdir("-p", m_screenshotImagePath);

    QString file = QtShell::realpath_strip(m_screenshotImagePath, name + ".png");

    image.save(file);
}

QString SnapshotTesting::Private::classNameToComponentName(const QString &className)
{
    QString res = className;

    QList<QmlType> types;

    QmlType type;

    types = obtainQmlTypeList(false);

    foreach (QmlType ty, types) {
        if (ty.className == className) {
            type = ty;
            break;
        }
    }

    if (!type.isNull) {
        return type.elementName;
    }

    if (res.indexOf("QQuick") == 0) {
        res = res.replace("QQuick", "");
    }

    res = removeDynamicClassSuffix(res);

    return res;
}

QQmlContext* SnapshotTesting::Private::obtainCurrentScopeContext(QObject *object)
{
    QList<QQmlContext*> list;

    QQmlContext* context = obtainCreationContext(object);

    QQmlContext* result = 0;

    while (context != 0 && !context->baseUrl().isEmpty()) {
        list << context;
        context = context->parentContext();
    }

    if (list.size() == 0) {
        return 0;
    }

    if (object->parent() && list.size() > 0) {
        if (qmlContext(object->parent()) == list.last()) {
            list.takeLast();
        }
    }

    QQuickItem* item = qobject_cast<QQuickItem*>(object);
    if (item && item->parentItem() && list.size() > 0) {
        if (qmlContext(item->parentItem()) == list.last()) {
            list.takeLast();
        }
    }

    if (list.size() > 0) {
        result = list.last();
    }

    return result;
}

/// Obtain the bottom-most context of a QObject
QQmlContext *SnapshotTesting::Private::obtainCreationContext(QObject *object)
{
    QQmlContext* result = 0;
    QQmlData *ddata = QQmlData::get(object, false);
    if (ddata && ddata->context) {
        // obtain the inner context name
        result  = ddata->context->asQQmlContext();
    }
    return result;
}

QString SnapshotTesting::Private::obtainComponentNameByBaseUrl(const QUrl &baseUrl)
{
    QString path = QtShell::realpath_strip(baseUrl.toString());
    QFileInfo info(path);

    return info.baseName();
}

QString SnapshotTesting::Private::obtainComponentNameByCreationContext(QObject *object)
{
    QQmlContext* creationContext = SnapshotTesting::Private::obtainCreationContext(object);

    return SnapshotTesting::Private::obtainComponentNameByBaseUrl(creationContext->baseUrl());
}

QString SnapshotTesting::Private::obtainComponentNameByInheritedContext(QObject *object)
{
    QList<QUrl> urls;
    QQmlContext* creationContext = SnapshotTesting::Private::obtainCreationContext(object);
    QQmlContext* currentScopeContext = obtainCurrentScopeContext(object);
    QQmlContext* context = creationContext;

    while (context && context != currentScopeContext) {
        urls << context->baseUrl();
        context = context->parentContext();
    }

    QString res;

    if (urls.size() <= 0) {
        res = SnapshotTesting::Private::obtainComponentNameByClass(object);

        QString currentScopeContextName = obtainComponentNameByBaseUrl(currentScopeContext->baseUrl());

        if (res == currentScopeContextName) {
            res = obtainComponentNameByInheritedClass(object);
        }

    } else {
        res = SnapshotTesting::Private::obtainComponentNameByBaseUrl(urls.last());
    }
    return res;
}

QString SnapshotTesting::Private::obtainComponentNameByCurrentScopeContext(QObject *object)
{
    QQmlContext * context = obtainCurrentScopeContext(object);
    if (context) {
        return obtainComponentNameByBaseUrl(context->baseUrl());
    } else {
        return obtainComponentNameByQuickClass(object);
    }
}

QString SnapshotTesting::Private::obtainComponentNameOfQtType(QObject *object)
{
    QList<QQmlContext*> list = listOwnedContext(object);
    QString res;


    while (list.size() > 0) {
        QQmlContext* context = list.takeFirst();
        QUrl url = context->baseUrl();
        if (inQtInternalContextUrls(url)) {
            res = obtainComponentNameByBaseUrl(url);
        }
    }

    if (res.isNull()) {

        const QMetaObject* meta = object->metaObject();

        while (meta) {

            QString className = meta->className();

            // A dirty hack.
            if (className.indexOf("QQuick") == 0 ||
                className == "QObject") {
                res = classNameToComponentName(className);
                break;
            }

            meta = meta->superClass();
        }
    }

    return res;
}

QString SnapshotTesting::Private::obtainComponentNameByClass(QObject *object)
{
    QString result;
    QString className = obtainClassName(object);

    result = classNameToComponentName(className);

    if (result.isNull()) {
        QString knownClassName = obtainKnownClassName(object);

        result = classNameToComponentNameTable[knownClassName];
    }
    return result;
}

QString SnapshotTesting::Private::obtainComponentNameByInheritedClass(QObject *object)
{
    QString result;

    const QMetaObject* meta = object->metaObject();
    if (meta->superClass()) {
        const QMetaObject* superClass = meta->superClass();
        result = classNameToComponentName(superClass->className());
    }

    return result;
}

QString SnapshotTesting::Private::obtainComponentNameByQuickClass(QObject *object)
{
    QString result;

    const QMetaObject* meta = object->metaObject();

    while (meta && !(result.indexOf("QQuick") == 0 || result.indexOf("QObject")  == 0 )  ) {
        result = meta->className();
        meta = meta->superClass();
    }

    result = classNameToComponentName(result);
    return result;
}


QString SnapshotTesting::Private::obtainSourceComponentName(QObject *object, bool expandAll)
{
    QString res;
    if (expandAll) {
        res = obtainComponentNameByQuickClass(object);
    } else {
        QList<QQmlContext*> list = listOwnedContext(object);
        list = filterContextWithNullBaseUrl(list);
        QQmlContext* context = 0;

        if (list.size() >= 2) {
            context = list[1];
        }

        if (context) {
            res = obtainComponentNameByBaseUrl(context->baseUrl());
        }

        if (res.isNull()) {
            res = obtainComponentNameOfQtType(object);
        }

    }

    return res;
}

static bool inherited(QObject *object, QString className) {
    bool res = false;

    const QMetaObject *metaObject = object->metaObject();

    while (metaObject) {
        if (metaObject->className() == className) {
            res = true;
            break;
        }
        metaObject = metaObject->superClass();
    }

    return res;
}

static QVariantMap dehydrate(QObject* source, const SnapshotTesting::CaptureOptions& options) {
    QString topLevelContextName;
    bool captureVisibleItemOnly = options.captureVisibleItemOnly;
    bool expandAll = options.expandAll;
    QList<QQmlContext*> topLevelContexts;
    QList<QUrl> topLevelBaseUrlList;

    class Header {
    public:
        QString name;
        QString comment;
    };

    {
        QQmlContext* context = qmlContext(source);
        if (context) {
            topLevelContexts << context;
            topLevelBaseUrlList<< context->baseUrl();
        }
        context = SnapshotTesting::Private::obtainCreationContext(source);
        if (context) {
            topLevelContexts << context;
            topLevelBaseUrlList<< context->baseUrl();
        }
    }

    auto obtainContextName = [=](QObject *object) {
        QString result;
        QQmlData *ddata = QQmlData::get(object, false);
        if (ddata && ddata->context) {
            // obtain the inner context name
            QUrl fileUrl = ddata->context->url();

            if (!fileUrl.isEmpty()) {
                result = SnapshotTesting::Private::obtainComponentNameByBaseUrl(fileUrl.toString());
            }
        }
        return result;
    };

    topLevelContextName = obtainContextName(source);

    auto obtainId = [=](QObject* object) -> QString {
        if (topLevelContexts.size() == 0) {
            return "";
        }
        QString res = topLevelContexts.first()->nameForObject(object);
        if (res.isEmpty()) {
            QQmlContext* context = qmlContext(object);
            if (context) {
                res = context->nameForObject(object);
            }
        }
        return res;
    };

    auto obtainBaseUrl = [](QObject* object) {
        QUrl baseUrl;
        QQmlContext *context = qmlContext(object);
        if (context) {
            baseUrl = context->baseUrl();
        }
        return baseUrl;
    };

    /// Obtain the item name in QML
    auto obtainItemHeader = [=,&topLevelContextName](QObject* object) {
        Header header;
        QString name;

        if (object == source) {
            header.name = SnapshotTesting::Private::obtainSourceComponentName(object, options.expandAll);
            QString contextName = obtainComponentNameByCurrentScopeContext(object);
            if (header.name != contextName) {
                header.comment = contextName;
            }
            return header;
        }

        QQmlContext* context = obtainBaseContext(object);
        if (context) {
            name = obtainComponentNameByBaseUrl(context->baseUrl());
        }
        if (name.isNull()) {
            name = obtainComponentNameByQuickClass(object);
        }


        header.name = name;

        if (options.expandAll) {
            QString rawTypename = obtainComponentNameByQuickClass(object);

            if (header.name != rawTypename) {
                header.comment = header.name;
                header.name = rawTypename;
            }
        }

        return header;
    };

    auto obtainDefaultValuesMap = [=](QObject* object) {
        const QMetaObject* meta = object->metaObject();
        QVariantMap result = obtainDynamicDefaultValues(meta);

        QList<const QMetaObject*> classes;

        while (meta != 0) {
            classes << meta;
            meta = meta->superClass();
        }

        // Reverse the order
        while (classes.size() > 0) {
            meta = classes.takeLast();
            QString className = removeDynamicClassSuffix(meta->className());
            QList<QVariantMap> pending;
            pending << obtainDynamicDefaultValues(meta);

            if (classDefaultValues.contains(className)) {
                pending << classDefaultValues[className];
            }

            for (int i = 0 ; i < pending.size(); i++) {
                QVariantMap map = pending[i];
                QStringList keys = map.keys();
                foreach (QString key, keys) {
                    result[key] = map[key];
                }
            }
        }

        return result;
    };

    auto obtainIgnoreList = [=](QObject* object) {
        return findIgnorePropertyList(object, classIgnoredProperties, componentIgnoredProperties);
    };

    auto _dehydrateFont = [=](QVariantMap& dest, QString property, QFont original , QFont current) {
        DEHYDRATE_FONT(dest,property,original,current, pixelSize);
        DEHYDRATE_FONT(dest,property,original,current, bold);
        DEHYDRATE_FONT(dest,property,original,current, capitalization);
        DEHYDRATE_FONT(dest,property,original,current, family);
        DEHYDRATE_FONT(dest,property,original,current, hintingPreference);
        DEHYDRATE_FONT(dest,property,original,current, italic);
        DEHYDRATE_FONT(dest,property,original,current, letterSpacing);

        DEHYDRATE_FONT(dest,property,original,current, pointSize);
        DEHYDRATE_FONT(dest,property,original,current, strikeOut);
        DEHYDRATE_FONT(dest,property,original,current, styleName);
        DEHYDRATE_FONT(dest,property,original,current, underline);
        DEHYDRATE_FONT(dest,property,original,current, weight);
        DEHYDRATE_FONT(dest,property,original,current, wordSpacing);
    };

    auto isQMetaObject = [](QJSValue value) {
#if (QT_VERSION < QT_VERSION_CHECK(5,8,0))
        return false;
#else
        return value.isQMetaObject();
#endif
    };

    auto _dehydrate = [=](QObject* object, QString componentName) {
        Q_UNUSED(componentName);

        QVariantMap dest;
        QVariantMap defaultValues = obtainDefaultValuesMap(object);
        QStringList ignoreList = obtainIgnoreList(object);

        const QMetaObject* meta = object->metaObject();

        QString id = obtainId(object);
        if (!id.isNull() && !options.hideId) {
            dest["id"] = id;
        }

        for (int i = 0 ; i < meta->propertyCount(); i++) {
            const QMetaProperty property = meta->property(i);
            const char* name = property.name();
            QString stringName = name;

            if (ignoreList.indexOf(stringName) >= 0) {
                continue;
            }

            QVariant value = object->property(name);

            if (value == defaultValues[stringName]) {
                // ignore default value
                continue;
            }

            if (forbiddenDataTypeList.indexOf(value.userType()) >= 0) {
                continue;
            } else if (value.type() == QVariant::Font) {
                _dehydrateFont(dest, stringName, defaultValues[stringName].value<QFont>(), value.value<QFont>());
                continue;
            } else if (value.canConvert<QObject*>()) {
                // ignore object value
                continue;
            } else if (value.userType() == qMetaTypeId<QJSValue>()) {
                QJSValue jsValue = value.value<QJSValue>();
                if (jsValue.isQObject() || isQMetaObject(jsValue)) {
                    continue;
                }
                value = jsValue.toVariant();
            }

            if (property.isEnumType()) {
                QMetaEnum enumerator = property.enumerator();
                EnumString enumValue;                
                enumValue.componentName = classNameToComponentName(enumerator.scope());
                enumValue.key = enumerator.valueToKey(value.toInt());
                value = QVariant::fromValue<EnumString>(enumValue);
            }

            dest[stringName] = value;
        }
        return dest;
    };

    auto isVisible = [=](QObject* object) {
        QQuickItem* item = qobject_cast<QQuickItem*>(object);

        if (!item) {
            return false;
        }

        if (item->opacity() == 0 ||
            !item->isVisible()) {
            return false;
        }

        return true;
    };

    auto allowTravel = [=](QObject* object) {
        if (!captureVisibleItemOnly) {
            return true;
        }

        return isVisible(object);
    };

    std::function<QVariantMap(QObject*)> travel;

    travel = [=, &travel](QObject* object) {
        if (!allowTravel(object)) {
            return QVariantMap();
        }

        Header header = obtainItemHeader(object);

        QVariantMap dest;
        dest = _dehydrate(object, header.name);

        QObjectList children = obtainChildrenObjectList(object);
        QVariantList childrenDataList;

        foreach (QObject* child, children) {
            QVariantMap childData = travel(child);
            childrenDataList << childData;
        }

        if (childrenDataList.size() > 0) {
            dest["$children"] = childrenDataList;
        }

        dest["$class"] = obtainKnownClassName(object);
        dest["$name"] = header.name;

        if (!header.comment.isNull()) {
            dest["$comment"] = header.comment;
        }

        QUrl baseUrl = obtainBaseUrl(object);

        if ( (!expandAll && topLevelBaseUrlList.indexOf(baseUrl) < 0) ||
             (inQtInternalContextUrls(baseUrl))) {
            // Skip condition
            // 1) It don't have any context relatd to the top level context url
            // 2) The context belong to Qt's internal context

            dest["$skip"] = true;
        }

        return dest;
    };

    if (captureVisibleItemOnly && !isVisible(source)) {
        qDebug() << "SnapshotTesting::capture(): The capture target is not visible";
    }

    return travel(source);
}

static QString prettyText(QVariantMap snapshot, SnapshotTesting::CaptureOptions& options) {
    QStringList priorityFields;

    priorityFields << "objectName" << "x" << "y" << "width" << "height";

    /// Convert num to string
    auto numberToString = [](qreal value) {
        QString res;
        double intpart;
        double fractpart = modf(value, &intpart);
        if (fractpart != 0) {
            res = QString("%1").arg(value,0,'f',2,'0');
        } else {
            res = QString("%1").arg((int) value);
        }
        return res;
    };

    auto _prettyField = [=](QString className, QString field, QVariant v, int indent) {
        QString res;
        QString format = "%1: %2";
        QString quotedFormat = "%1: \\"%2\\"";

        //@TODO Change to a template function and allow customization by user

        if (v.type() == QVariant::Bool) {
            res = QString(format).arg(field).arg(v.toBool() ? "true" : "false");
        } else if (v.type() == QVariant::Double) {
            res = QString(format).arg(field).arg(numberToString(v.toDouble()));
        } else if (v.type() == QVariant::String) {
            res = QString(quotedFormat).arg(field).arg(v.toString());
        } else if (v.type() == QVariant::Int) {
            res = QString(format).arg(field).arg(v.toInt());
        } else if (v.type() == QVariant::Color) {
            res = QString(quotedFormat).arg(field).arg(v.value<QColor>().name());
        } else if (v.type() == QVariant::Url) {
            res = QString(quotedFormat).arg(field).arg(v.toUrl().toString());
        } else if (v.type() == QVariant::Size) {
            QSize size = v.toSize();
            res = QString("%1: Qt.size(%2,%3)").arg(field).arg(size.width()).arg(size.height());
        } else if (v.type() == QVariant::SizeF) {
            QSizeF size = v.toSizeF();
            res = QString("%1: Qt.size(%2,%3)").arg(field).arg(numberToString(size.width())).arg(numberToString(size.height()));
        } else if (v.type() == QVariant::RectF) {
            QRectF rect = v.toRectF();
            res = QString("%1: Qt.rect(%2,%3,%4,%5)").arg(field).arg(numberToString(rect.x())).arg(numberToString(rect.y())).arg(numberToString(rect.width())).arg(numberToString(rect.height()));
        } else if (v.type() == QVariant::Map || v.type() == QVariant::List) {
            res = QString(format).arg(field).arg(stringify(v));
        } else if (v.userType() == qMetaTypeId<EnumString>()) {
            EnumString enumString = v.value<EnumString>();
            res = QString("%1: %2.%3").arg(field).arg(enumString.componentName).arg(enumString.key);
        } else {
            qDebug() << QString("Non-supported type: %1 from %2 of %3 field").arg(v.typeName()).arg(className).arg(field);
            return QString("");
        }

        res = indentText(res, indent);

        return res;
    };

    std::function<QString(QVariantMap, int)> _prettyText;

    _prettyText = [=, &_prettyText](QVariantMap snapshot, int indent) {
        if (snapshot.isEmpty()) {
            return QString("");
        }

        QString className = snapshot["$class"].toString();
        QStringList lines;

        if (snapshot.contains("$skip")) {
            QVariantList children = snapshot["$children"].toList();
            for (int i = 0 ; i < children.size() ;i++) {
                QVariantMap data = children[i].toMap();
                QString line = _prettyText(data, indent);
                if (!line.isEmpty()) {
                    lines << "" << line;
                }
            }
            return lines.join("\\n");
        }

        int currentIndent = indent + options.indentSize;

        if (!snapshot.contains("$comment")) {
            lines << QString().fill(' ',indent) + snapshot["$name"].toString() + " {";
        } else {
            lines << QString().fill(' ',indent) + snapshot["$name"].toString() + QString(" { // %1").arg(snapshot["$comment"].toString());
        }

        QStringList keys = snapshot.keys();

        if (keys.indexOf("id") >= 0) {
            lines << _prettyField(className, "id", snapshot["id"], currentIndent).replace("\\"","");
            keys.removeOne("id");
        }

        for (int i = 0 ; i < priorityFields.size() ; i++) {
            QString key = priorityFields[i];
            if (keys.indexOf(key) >= 0) {
                QString line = _prettyField(className, key, snapshot[key], currentIndent);
                if (!line.isEmpty()) {
                    lines << line;
                }
                keys.removeOne(key);
            }
        }

        for (int i = 0 ; i < keys.size();i++) {
            QString key = keys[i];
            if (key.indexOf("$") == 0) {
                continue;
            }
            QString line = _prettyField(className, key, snapshot[key], currentIndent);
            if (!line.isEmpty())
                lines << line;
        }

        QVariantList children = snapshot["$children"].toList();
        for (int i = 0 ; i < children.size() ;i++) {
            QVariantMap data = children[i].toMap();
            QString line = _prettyText(data, currentIndent);
            if (!line.isEmpty()) {
                lines << "" << line;
            }
        }

        lines << QString().fill(' ',indent) +  QString("}");

        return lines.join("\\n");
    };

    return _prettyText(snapshot, 0);
}


void SnapshotTesting::setSnapshotsFile(const QString &file)
{
    m_snapshotFile = QtShell::realpath_strip(file);
}

QString SnapshotTesting::snapshotsFile()
{
    return m_snapshotFile;
}

QVariantMap SnapshotTesting::loadStoredSnapshots()
{
    if (!m_snapshots.isEmpty()) {
        return m_snapshots;
    }

    QVariantMap result;
    if (!QFile::exists(m_snapshotFile)) {
        return result;
    }

    QString content = QtShell::cat(m_snapshotFile);

    if (content.isNull()) {
        return result;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(),&error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << QString("SnapshotTesting::loadSnapshots: Failed to parse file: %1: %2").arg(m_snapshotFile).arg(error.errorString());
    }

    QVariantMap data = doc.object().toVariantMap();

    m_snapshots = data["content"].toMap();

    return m_snapshots;
}


void SnapshotTesting::saveSnapshots()
{
    if (m_snapshots.isEmpty()) {
        loadStoredSnapshots();
    }

    if (!m_snapshotsDirty) {
        return;
    }

    m_snapshotsDirty = false;

    QVariantMap data;

    data["content"] = m_snapshots;

    QJsonObject object = QJsonObject::fromVariantMap(data);

    QJsonDocument doc;
    doc.setObject(object);
    QByteArray bytes = doc.toJson(QJsonDocument::Indented);

    QFile file;
    file.setFileName(m_snapshotFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << QString("SnapshotTesting::saveSnapshots: Failed to save snapshos file: %1").arg(file.errorString());
        return;
    }

    file.write(bytes);
    file.close();
}

void SnapshotTesting::setSnapshotText(const QString &name, const QString &content)
{
    m_snapshots[name] = content;
    m_snapshotsDirty = true;
}

void SnapshotTesting::setInteractiveEnabled(bool value)
{
    m_interactiveEnabled = value;
}

bool SnapshotTesting::interactiveEnabled()
{
    return m_interactiveEnabled;
}

void SnapshotTesting::setIgnoreAllMismatched(bool value)
{
    m_ignoreAllMismatched = value;
}

bool SnapshotTesting::ignoreAllMismatched()
{
    return m_ignoreAllMismatched;
}

QString SnapshotTesting::capture(QObject *object, SnapshotTesting::CaptureOptions options)
{
    if (options.captureOnReady) {
        Private::waitUntilReady(object);
    }

    QVariantMap data = dehydrate(object, options);
    return prettyText(data, options);
}

bool SnapshotTesting::matchStoredSnapshot(const QString &name, const QString &snapshot) {
    return matchStoredSnapshot(name, snapshot, QImage());
}

bool SnapshotTesting::matchStoredSnapshot(const QString &name, const QString &snapshot, const QImage& screenshot)
{
    QVariantMap snapshots = SnapshotTesting::loadStoredSnapshots();
    Q_UNUSED(screenshot);

    QString originalVersion = snapshots[name].toString();

    static int tabIndex = 0;

    if (originalVersion == snapshot) {
        // Save the screenshot if absent
        if (!m_screenshotImagePath.isNull() && !screenshot.isNull()) {

            QString file = QtShell::realpath_strip(m_screenshotImagePath, name + ".png");

            if (!QFile::exists(file)) {
                saveScreenshotImage(name, screenshot);
            }
        }
        return true;
    }

    QString diff = SnapshotTesting::diff(originalVersion, snapshot);

    qDebug().noquote() << "matchStoredSnapshot: The snapshot is different:";
    qDebug().noquote() << diff;

    if (m_acceptAllMismatched) {
        SnapshotTesting::setSnapshotText(name, snapshot);
        SnapshotTesting::saveSnapshots();
        return true;
    }

    if (SnapshotTesting::interactiveEnabled() && !SnapshotTesting::ignoreAllMismatched()) {
        QQmlApplicationEngine engine;
        engine.addImportPath("qrc:///");

        engine.load(QUrl("qrc:///qt-project.org/imports/SnapshotTesting/Matcher.qml"));

        QObject* dialog = engine.rootObjects()[0];
        Q_ASSERT(dialog);

        QString monospaceFont =  QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
        dialog->setProperty("monospaceFont" ,monospaceFont);
        dialog->setProperty("diff", diff);
        dialog->setProperty("previousSnapshot", originalVersion);
        dialog->setProperty("snapshot", snapshot);
        dialog->setProperty("title", name);
        dialog->setProperty("tabIndex", tabIndex);

        if (!screenshot.isNull()) {
            dialog->setProperty("screenshot", QString(toBase64(screenshot)));
        }

        QImage previousScreenshot;

        if (!m_screenshotImagePath.isNull()) {
            QString previosScreenshotFile;
             previosScreenshotFile = QtShell::realpath_strip(m_screenshotImagePath, name + ".png");
            if (QFile::exists(previosScreenshotFile)) {
                if (previousScreenshot.load(previosScreenshotFile)) {
                    dialog->setProperty("previousScreenshot", toBase64(previousScreenshot));
                }
            }
        }

        if (!previousScreenshot.isNull() && !screenshot.isNull() && m_screenshotImageCombinator != nullptr) {
            QImage combinedScreenshot = m_screenshotImageCombinator(screenshot, previousScreenshot);
            dialog->setProperty("combinedScreenshot", toBase64(combinedScreenshot));
        }

        QMetaObject::invokeMethod(dialog, "open");
        QCoreApplication::exec();

        tabIndex = dialog->property("tabIndex").toInt();

        int button = dialog->property("clickedButton").value<int>();
        switch (button) {
        // Use hex code to avoid the dependence to QtWidget
        case 0x00020000: // No to all
            SnapshotTesting::setIgnoreAllMismatched(true);
            break;
        case 0x00008000: // Yes to all
            m_acceptAllMismatched = true;
        case 0x00004000: // Yes
        case 0x02000000:
            SnapshotTesting::setSnapshotText(name, snapshot);
            SnapshotTesting::saveSnapshots();
            saveScreenshotImage(name, screenshot);
            return true;
            break;
        }
    }

    return false;
}

static void init() {
    qRegisterMetaType<SnapshotTesting::Private::EnumString>();
    if (m_snapshotFile.isNull()) {
        m_snapshotFile = QtShell::realpath_strip(QtShell::pwd(), "snapshots.json");
    }

    {
        /* Configuration Loading */

    QString text = QtShell::cat(":/qt-project.org/imports/SnapshotTesting/config/snapshot-config.json");

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(),&error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON::parse() error: "<< error.errorString();
    }

    QVariantMap map = doc.object().toVariantMap();
    knownComponentList = map.keys();
    for (int i = 0 ; i < knownComponentList.size() ; i++) {
        QString key = knownComponentList[i];
        QVariantMap record =  map[key].toMap();

        if (!key.contains("@")) {
            classNameToComponentNameTable[key] = record["name"].toString();
            classDefaultValues[key] = record["defaultValues"].toMap();
            classIgnoredProperties[key] = record["ignoreProperties"].toStringList();
        } else {
            componentIgnoredProperties[key] = record["ignoreProperties"].toStringList();
        }
    }

    forbiddenDataTypeList << qMetaTypeId<QQmlListProperty<QQuickItem>>()
                      << qMetaTypeId<QQmlListProperty<QObject>>()
                      << qMetaTypeId<QByteArray>()
                      << qMetaTypeId<void*>();
    }

    /* Dynamic Configuration */
    {
        QQmlEngine engine;

        {
            QObject* button = createQmlComponent(&engine, "Button", "QtQuick.Controls", 2 ,0);
            QStringList urls = listContextUrls(button);
            m_qtInternalContextUrls << QtShell::dirname(urls[0]);
            button->deleteLater();
        }

        {
            QObject* button = createQmlComponent(&engine, "RadioButton", "QtQuick.Controls", 1 ,0);
            QStringList urls = listContextUrls(button);

            foreach (QString url, urls) {
                m_qtInternalContextUrls << QtShell::dirname(url);
            }
            button->deleteLater();
        }

        {
            QObject* object = createQmlComponent(&engine, "RadioButtonStyle", "QtQuick.Controls.Styles", 1,0);
            QStringList urls = listContextUrls(object);

            foreach (QString url, urls) {
                m_qtInternalContextUrls << QtShell::dirname(url);
            }
            object->deleteLater();
        }
    }

}


QString SnapshotTesting::diff(QString original, QString current)
{
    auto toVector = [=](QString text) {
        vector<string> res;

        QStringList lines = text.split("\\n");
        for (int i = 0 ; i < lines.size() ;i++) {
            res.push_back(lines[i].toStdString());
        }

        return res;
    };

    std::vector<string> text1, text2;

    text1 = toVector(original);
    text2 = toVector(current);
    dtl::Diff<std::string> diff(text1, text2);

    diff.onHuge();
    diff.compose();
    diff.composeUnifiedHunks();

    std::stringstream stream;

    diff.printUnifiedFormat(stream);

    return QString::fromStdString(stream.str());
}

QString SnapshotTesting::Private::stringify(QJSEngine *engine, QJSValue value)
{
    QString code = "function(value, indent) { return JSON.stringify(value,null,indent)}";
    QJSValue program = engine->evaluate(code);

    QJSValueList arguments;
    arguments << value << 4;

    QJSValue result = program.call(arguments);

    return result.toString();
}

QString SnapshotTesting::Private::stringify(QVariant value)
{
    QJSEngine engine;
    QString code = "function(value, indent) { return JSON.stringify(value,null,indent)}";
    QJSValue program = engine.evaluate(code);

    QJSValue input;

    if (value.type() == QVariant::Map || value.type() == QVariant::List) {
         input = engine.toScriptValue<QVariant>(value);
    } else {
        return "";
    }

    QJSValueList arguments;
    arguments << input << 4;

    QJSValue result = program.call(arguments);
    return result.toString();
}


QString SnapshotTesting::Private::leftpad(QString text, int pad)
{
    return QString("").fill(' ', pad) + text;
}

QString SnapshotTesting::Private::indentText(QString text, int pad)
{
    if (text.indexOf("\\n") < 0) {
        return leftpad(text, pad);
    }

    QStringList lines = text.split("\\n");

    QString first = lines[0];

    int index = first.indexOf(":");

    QStringList indentedLines;

    indentedLines << leftpad(first, pad);

    for (int i = 1 ; i < lines.size();i++) {
        indentedLines << leftpad(lines[i], pad + index + 2);
    }

    return indentedLines.join("\\n");
}

QObjectList SnapshotTesting::Private::obtainChildrenObjectList(QObject *object)
{
    QObjectList children = object->children();

    QString className = removeDynamicClassSuffix(obtainClassName(object));

    if (className == "QQuickRepeater") {
        int count = object->property("count").toInt();
        for (int i = 0 ; i < count; i++) {
            QQuickItem* child;
            QMetaObject::invokeMethod(object,"itemAt",Qt::DirectConnection,
                                      Q_RETURN_ARG(QQuickItem*,child),
                                      Q_ARG(int,i));

            children << child;
        }
    } else if (inherited(object, "QQuickFlickable")) {

        QQuickItem* contentItem = object->property("contentItem").value<QQuickItem*>();

        if (contentItem) {
            QList<QQuickItem *>items = contentItem->childItems() ;

            for (int i = 0 ;  i < items.size() ; i++) {
                children << items.at(i);
            }
        }
    }

    return children;
}


QFuture<void> SnapshotTesting::Private::whenReady(QObject *object)
{
    if (object == 0) {
        return QFuture<void>();
    }

    auto onStatusChanged = [=](QObject* object) mutable {
        return AsyncFuture::observe(object,SIGNAL(statusChanged())).future();
    };

    auto onImageStatusChanged = [=](QObject* object) mutable {
        SignalProxy* proxy = new SignalProxy(object);
        QObject::connect(object, SIGNAL(statusChanged(QQuickImageBase::Status)),
                         proxy, SIGNAL(proxy()));
        // A dirty hack to vvoid the "cannot queue arguments of type qquickimagebase::Status"

        return AsyncFuture::observe(proxy,&SignalProxy::proxy).future();
    };

    QList<QFuture<void>> futures;

    walk(object, [&](QObject* object, QObject* parent) {
        Q_UNUSED(parent);

        QString className = removeDynamicClassSuffix(obtainClassName(object));

        if (className == "QQuickLoader") {
            bool asynchronous = object->property("asynchronous").toBool();
            int status = object->property("status").toInt();
            //@TODO - check is source / sourceComponent set?

            if (asynchronous && status == 2) { // Loading
                futures << onStatusChanged(object);
            }
        } else if (className == "QQuickImage") {
            bool asynchronous = object->property("asynchronous").toBool();
            int status = object->property("status").toInt();
            QString source = object->property("source").toString();

            if (!source.isNull() && asynchronous && status == 2 ) { // Loading
                futures << onImageStatusChanged(object);
            }
        }

        return true;
    });

    if (futures.size() == 0) {
        auto defer = AsyncFuture::deferred<void>();
        defer.complete();
        return defer.future();
    }

    auto combinator = AsyncFuture::combine();
    for (int i = 0 ; i < futures.size() ;i++) {
        combinator << futures[i];
    }

    return combinator.future();
}

bool SnapshotTesting::Private::waitUntilReady(QObject *object, int timeout)
{
    auto defer = AsyncFuture::deferred<void>();

    QFuture<void> ready = whenReady(object);

    defer.complete(ready);

    QTimer::singleShot(timeout, [=]() mutable {
        defer.cancel();
    });

    auto future = defer.future();
    AConcurrent::await(future);

    if (future.isCanceled()) {
        qDebug() << "Timeout!";
    }

    return !future.isCanceled();
}

void SnapshotTesting::Private::walk(QObject *object, std::function<bool (QObject *, QObject *)> predicate)
{
    QMap<QObject*, bool> map;

    std::function<bool(QObject*, QObject*)> _walk;

    _walk = [&](QObject* object, QObject* parent) -> bool {

        if (!object || map[object]) {
            return true;
        }

        map[object] = true;

        if (!predicate(object, parent)) {
            return false;
        }


        QObjectList children = obtainChildrenObjectList(object);
        foreach (QObject* child , children) {
            if (!_walk(child, object)) {
                return false;
            }
        }

        return true;
    };

    _walk(object, 0);
}

void SnapshotTesting::addClassIgnoredProperty(const QString &className, const QString &property)
{
    QStringList list = classIgnoredProperties[className];
    if (list.indexOf(property) < 0) {
        list.append(property);
    }
    classIgnoredProperties[className] = list;
}

void SnapshotTesting::removeClassIgnoredProperty(const QString &className, const QString &property)
{
    QStringList list = classIgnoredProperties[className];
    list.removeAll(property);
    classIgnoredProperties[className] = list;
}

QString SnapshotTesting::Private::obtainQmlPackage(QObject *object)
{
    const QMetaObject* meta = object->metaObject();

    QmlType type = findQmlType(meta);
    return type.module;
}

QVariantMap SnapshotTesting::Private::obtainDynamicDefaultValues(QObject *object)
{
    return obtainDynamicDefaultValues(object->metaObject());
}

QVariantMap SnapshotTesting::Private::obtainDynamicDefaultValues(const QMetaObject *meta)
{
    static QMap<const QMetaObject*, QVariantMap> storage;

    if (storage.contains(meta)) {
        return storage[meta];
    }

    QVariantMap res;
    const QmlType type = findQmlType(meta, false); // it will only return public component

    if (type.isNull || !type.isCreatable) {
        return res;
    }

    auto getDefaultValues = [](QString componentName, QString package, int major, int minor) {

        QQmlEngine engine;

        QObject* holder = createQmlComponent(&engine, componentName, package, major, minor);
        QVariantMap res;

        if (holder) {
            assign(res, holder);
            delete holder;
        }

        return res;
    };

    QString module = type.module;

    // dirty hack
    if (module == "QtQuick.Templates") {
        module = "QtQuick.Controls";
    }

    res = getDefaultValues(type.elementName, module, type.majorVersion, type.minorVersion);

    storage[meta] = res;

    return res;
}


QObject *SnapshotTesting::Private::createQmlComponent(QQmlEngine* engine, QString componentName, QString package, int major, int minor)
{
    QStringList packages;
    packages << QString("import %1 %2.%3").arg(package).arg(major).arg(minor);

    QString qml  = QString("%2\\n %1 {}").arg(componentName).arg(packages.join("\\n"));

    QQmlComponent comp (engine);
    comp.setData(qml.toUtf8(),QUrl());
    QObject* ret = comp.create();

    if (!ret) {
        qDebug() << comp.errorString();
    }

    return ret;
}

QStringList SnapshotTesting::Private::listContextUrls(QObject *object)
{
    QStringList list;
    QQmlContext* context = obtainCreationContext(object);

    while (context) {
        QUrl url  = context->baseUrl();
        if (!url.isEmpty()) {
            list << url.toString();
        }
        context = context->parentContext();
    }

    return list;
}

QFuture<QImage> SnapshotTesting::Private::grabImage(QQuickItem *item)
{
    QSize size = QSize(item->width(), item->height());
    QSharedPointer<QQuickItemGrabResult> grabber = item->grabToImage(size);

    if (grabber.isNull()) {
        return QFuture<QImage>();
    }

    auto defer = AsyncFuture::deferred<QImage>();

    AsyncFuture::observe(grabber.data(), &QQuickItemGrabResult::ready).subscribe([=]() mutable {
        defer.complete(grabber->image());
    });

    return defer.future();
}

bool SnapshotTesting::tryMatchStoredSnapshot(const QString &name, const QString &snapshot)
{
    QVariantMap snapshots = SnapshotTesting::loadStoredSnapshots();

    QString originalVersion = snapshots[name].toString();

    return (originalVersion == snapshot);
}

void SnapshotTesting::setScreenshotImagePath(const QString &path)
{
    m_screenshotImagePath = path;
}

QImage SnapshotTesting::Private::combineImages(const QImage &prev, const QImage &curr)
{
    QSize size(qMax(prev.width(), curr.width()), qMax(prev.height(), curr.height()));

    QList<QImage> images;
    images << prev << curr;

    QImage canvas(size, QImage::Format_RGB32);
    canvas.fill(QColor(0,0,0,0));

    QPainter painter(&canvas);
    painter.setOpacity(0.5);
    foreach (QImage image, images) {
        QSize s = image.size();
        int x = (size.width() - s.width()) / 2;
        int y = (size.height() - s.height()) / 2;
        painter.drawImage(x,y, image);
    }
    painter.end();

    return canvas;
}

QList<QQmlContext*> SnapshotTesting::Private::listOwnedContext(QObject* object) {
    QList<QQmlContext*> result;



    QQmlContext* context = obtainCreationContext(object);

    auto inContext = [=](QQmlContext* context, QObject* object) {
        QQmlContext* c = obtainCreationContext(object);

        while (c) {

            if (c == context || (c->baseUrl() == context->baseUrl() && !c->baseUrl().isEmpty())) {
                return true;
            }
            c = c->parentContext();
        }
        return false;
    };

    std::function<bool(QQmlContext* context, QObject* object)> _containsContext;

    _containsContext = [&_containsContext, inContext](QQmlContext* context, QObject* object) {
        if (inContext(context, object)) {
            return true;
        }

        if (object->parent()) {
            if (_containsContext(context, object->parent())) {
                return true;
            }
        }

        QQuickItem* item = qobject_cast<QQuickItem*>(object);
        if (item && item->parentItem()) {
            if (_containsContext(context, item->parentItem())) {
                return true;
            }
        }

        return false;
    };

    auto isBaseContext = [=](QQmlContext* context, QObject* object) {

        if (object->parent()) {
            if (_containsContext(context, object->parent())) {
                return false;
            }
        }

        QQuickItem* item = qobject_cast<QQuickItem*>(object);
        if (item && item->parentItem()) {
            if (_containsContext(context, item->parentItem())) {
                return false;
            }
        }

        return true;
    };

    QList<QQmlContext*> list;
    QQmlContext* c = context;
    while (c) {
        list << c;
        c = c->parentContext();
    }

    while (list.size() > 0) {
        c = list.takeLast();
        if (isBaseContext(c, object)) {
            result << c;
        }
    }

    return result;
}


QQmlContext *SnapshotTesting::Private::obtainBaseContext(QObject *object)
{
    QQmlContext* res = 0;

    QList<QQmlContext*> list = listOwnedContext(object);

    if (list.size() > 0) {
        res = list.first();
    }

    return res;
}

QString SnapshotTesting::Private::converToPackageNotation(QUrl url)
{
    QString input = url.path();
    QStringList token = input.split("/");

    QStringList parts;
    for (int i = 0 ; i < token.size(); i++) {
        QString str = token[i];
        if (str.isEmpty()) {
            continue;
        }

        str.replace(QRegExp("\\\\.[0-9]+$"), "");

        parts << str;
    }

    parts.takeLast();
    return parts.join(".");
}

QStringList SnapshotTesting::Private::findIgnorePropertyList(QObject *object, QMap<QString, QStringList> ignoreListForClasses, QMap<QString, QStringList> ignoreListForComponent)
{
    const QMetaObject* meta = object->metaObject();
    QStringList result;
    while (meta != 0) {
        QString className = meta->className();
        if (ignoreListForClasses.contains(className)) {
            QStringList list = ignoreListForClasses[className];
            result.append(list);
        }

        meta = meta->superClass();
    }


    QStringList baseUrls = listContextUrls(object);

    while (baseUrls.size() > 0) {
        QString baseUrl = baseUrls.takeLast();
        QString package = converToPackageNotation(QUrl(baseUrl));
        QString name = obtainComponentNameByBaseUrl(QUrl(baseUrl)) + "@";

        QMapIterator<QString, QStringList> iter(ignoreListForComponent);

        while (iter.hasNext()) {
            iter.next();
            if (!iter.key().startsWith(name)) {
                continue;
            }

            QStringList token = iter.key().split("@");
            if (package.endsWith(token.last())) {
                result.append(iter.value());
            }
        }
    }

    return result;
}

void SnapshotTesting::addComponentIgnoreProperty(const QString &componentName, const QString &package, const QString &property)
{
    QString key = componentName + "@" + package;

    QStringList list = componentIgnoredProperties[key];
    if (list.indexOf(property) < 0) {
        list.append(property);
    }
    componentIgnoredProperties[key] = list;
}


void SnapshotTesting::removeComponentIgnoreProperty(const QString &componentName, const QString &package, const QString &property)
{
    QString key = componentName + "@" + package;

    QStringList list = componentIgnoredProperties[key];
    list.removeAll(property);
    componentIgnoredProperties[key] = list;
}


SnapshotTesting::Test SnapshotTesting::createTest()
{
    SnapshotTesting::Test test;
    test.setName(QTest::currentTestFunction());
    return test;
}

QString SnapshotTesting::replaceLines(const QString &input, QRegExp regexp, QString replace)
{
    QStringList token = input.split("\\n");

    for (int i = 0; i < token.size() ;i++) {
        token[i] = token[i].replace(regexp, replace);
    }
    return token.join("\\n");
}

Q_COREAPP_STARTUP_FUNCTION(init)
