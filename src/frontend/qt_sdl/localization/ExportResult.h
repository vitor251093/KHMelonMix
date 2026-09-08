#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

struct NdsExportedFile
{
    QString path;
    int stringCount = 0;
};

struct NdsExportResult
{
    QVector<NdsExportedFile> files;
    bool success = false;

    int totalStrings() const
    {
        int total = 0;
        for (const NdsExportedFile& file : files)
            total += file.stringCount;
        return total;
    }
};

Q_DECLARE_METATYPE(NdsExportedFile)
Q_DECLARE_METATYPE(NdsExportResult)
