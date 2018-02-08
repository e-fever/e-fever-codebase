#ifndef SNAPSHOTTESTINGRENDERER_H
#define SNAPSHOTTESTINGRENDERER_H

#include <QQmlEngine>
#include <QImage>
#include <QPointer>
#include <QQuickWindow>
#include <QOffscreenSurface>
#include <QQuickRenderControl>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <private/snapshottestingoptions.h>
#include <QFuture>
#include <asyncfuture.h>

namespace SnapshotTesting {

    class Renderer {

    public:
        Renderer(QQmlEngine* engine);
        ~Renderer();

        QPointer<QQmlEngine> engine() const;

        bool load(const QString& source);

        QFuture<void> whenStill();

        void waitWhenStill(int timeout = -1);

        QString capture(SnapshotTesting::CaptureOptions options = SnapshotTesting::CaptureOptions());

        QImage grabScreenshot();

        /// The loaded item
        QObject *item() const;

    private:
        QImage render();

        QPointer<QQmlEngine> m_engine;

        /* Internal variables */
        AsyncFuture::Deferred<void> initialized;
        QWindow *owner;
        QQuickWindow* window;
        QQuickRenderControl* renderControl;
        QOffscreenSurface* surface;
        QOpenGLContext *context;
        QOpenGLFramebufferObject *fbo;

        QObject* m_item;
    };

}

#endif // SNAPSHOTTESTINGRENDERER_H
