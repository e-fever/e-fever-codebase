#include <QOpenGLFunctions>
#include <QQuickItem>
#include <asyncfuture.h>
#include <aconcurrent.h>
#include "snapshottesting.h"
#include "private/snapshottesting_p.h"

SnapshotTesting::Renderer::Renderer(QQmlEngine* engine) : m_engine(engine)
{

    class RenderControl : public QQuickRenderControl
    {
    public:
        RenderControl(QWindow *w) : m_window(w) { }
        QWindow *renderWindow(QPoint *offset) Q_DECL_OVERRIDE {
            if (offset)
                *offset = QPoint(0, 0);
            return m_window;
        }

    private:
        QWindow *m_window;
    };

    owner = new QWindow();
    renderControl = new RenderControl(owner);
    window = new QQuickWindow(renderControl);

    surface = new QOffscreenSurface();
    context = new QOpenGLContext();

    QSurfaceFormat format;
    format.setDepthBufferSize(16);
    format.setStencilBufferSize(8);
    context->setFormat(format);
    context->create();

    surface->setFormat(format);
    surface->create();

    m_item = 0;
    fbo = 0;
}

SnapshotTesting::Renderer::~Renderer()
{
    context->makeCurrent(surface);

    if (m_item) {
        delete m_item;
    }

    delete owner;
    delete window;
    delete renderControl;

    if (fbo) {
        delete fbo;
    }

    context->doneCurrent();

    delete surface;
    delete context;
}

bool SnapshotTesting::Renderer::load(const QString &source)
{
    // Create the object
    auto _create = [=](const QString &source) -> QObject* {
        QUrl url = QUrl::fromLocalFile(source);
        QQmlComponent component(m_engine.data(), url);

        if (component.isError()) {
            const QList<QQmlError> errorList = component.errors();

            for (const QQmlError &error : errorList) {
                qWarning() << error.url() << error.line() << error;
            }
            return 0;
        }

        return component.create();
    };

    auto _init = [=](QObject* rootObject) {
        QQuickItem* rootItem = qobject_cast<QQuickItem *>(rootObject);

        if (!rootItem) {
            return false;
        }

        QObject::connect(window, &QQuickWindow::sceneGraphInitialized, [=]() mutable {

            fbo = new QOpenGLFramebufferObject(window->size(), QOpenGLFramebufferObject::CombinedDepthStencil);
            window->setRenderTarget(fbo);

            // This is a dummy render which is needed for some components (e.g ListView) to create their content.
            render();
            initialized.complete();
        });

        auto updateGeom = [=]() {
            qreal width = rootItem->width();
            qreal height = rootItem->height();
            if (width == 0 || height == 0) {
                width = 10;
                height = 10;
            }
            window->setGeometry(0,0,width, height);
        };

        rootItem->setParentItem(window->contentItem());
        updateGeom();
        context->makeCurrent(surface);
        renderControl->initialize(context);

        QObject::connect(rootItem , &QQuickItem::widthChanged, updateGeom);
        QObject::connect(rootItem , &QQuickItem::heightChanged, updateGeom);

        return true;
    };

    QObject* rootObject = _create(source);
    rootObject->setParent(owner);

    if (!rootObject) {
        return false;
    }

    m_item = rootObject;

    QQuickItem* rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!rootItem) {
        return true;
    }

    _init(rootItem);
    return true;
}

QFuture<void> SnapshotTesting::Renderer::whenStill()
{
    return SnapshotTesting::Private::whenReady(m_item);
}

void SnapshotTesting::Renderer::waitWhenStill(int timeout)
{
    QFuture<void> future = whenStill();
    AConcurrent::await(future, timeout);
}

QString SnapshotTesting::Renderer::capture(SnapshotTesting::CaptureOptions options)
{
    if (!m_item) {
        return QString();
    }

    return SnapshotTesting::capture(m_item, options);
}

QImage SnapshotTesting::Renderer::grabScreenshot()
{
    QQuickItem* quickItem = qobject_cast<QQuickItem*>(m_item);

    if (!quickItem || quickItem->width() == 0 || quickItem->height() == 0) {
        return QImage();
    }

    auto future = initialized.subscribe([=]() {
        return render();
    }).future();

    AConcurrent::await(future);

    QImage res;
    if (!future.isCanceled()) {
        res = future.result();
    }

    return res;
}

QImage SnapshotTesting::Renderer::render()
{
    if (!context->makeCurrent(surface)) {
        qDebug() << "Failed to render";
        return QImage();
    }

    if (window->width() == 0 || window->height() == 0) {
        return QImage();
    }

    renderControl->polishItems();
    renderControl->sync();
    renderControl->render();

    window->resetOpenGLState();

    QOpenGLFramebufferObject::bindDefault();

    context->functions()->glFlush();

    return fbo->toImage();
}

QObject *SnapshotTesting::Renderer::item() const
{
    return m_item;
}

QPointer<QQmlEngine> SnapshotTesting::Renderer::engine() const
{
    return m_engine;
}
