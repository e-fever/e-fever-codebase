#ifndef DUMMY_H
#define DUMMY_H

#include <QObject>

/// A dummy class do nothing. You may remove it from your library.

class Dummy : public QObject
{
    Q_OBJECT
public:
    Dummy(QObject* parent = 0);
};

#endif // DUMMY_H
