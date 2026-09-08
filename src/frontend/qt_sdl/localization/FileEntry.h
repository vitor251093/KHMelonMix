#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

struct NdsFileEntry
{
    QString path;
    QString type;
    quint64 size = 0;
};

using NdsFileEntryList = QVector<NdsFileEntry>;

Q_DECLARE_METATYPE(NdsFileEntry)
Q_DECLARE_METATYPE(NdsFileEntryList)
