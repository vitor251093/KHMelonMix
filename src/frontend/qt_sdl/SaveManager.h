/*
    Copyright 2016-2023 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <string>
#include <unistd.h>
#include <time.h>
#include <atomic>
#include <memory>
#include <QThread>
#include <QMutex>

#include "types.h"

class SaveManager : public QThread
{
    Q_OBJECT
    void run() override;

public:
    SaveManager(const std::string& path);
    ~SaveManager();

    std::string GetPath();
    void SetPath(const std::string& path, bool reload);

    void RequestFlush(const khDaysMM::u8* savedata, khDaysMM::u32 savelen, khDaysMM::u32 writeoffset, khDaysMM::u32 writelen);
    void CheckFlush();

    bool NeedsFlush();
    void FlushSecondaryBuffer(khDaysMM::u8* dst = nullptr, khDaysMM::u32 dstLength = 0);

private:
    std::string Path;

    std::atomic_bool Running;

    std::unique_ptr<khDaysMM::u8[]> Buffer;
    khDaysMM::u32 Length;
    bool FlushRequested;

    QMutex* SecondaryBufferLock;
    std::unique_ptr<khDaysMM::u8[]> SecondaryBuffer;
    khDaysMM::u32 SecondaryBufferLength;

    time_t TimeAtLastFlushRequest;

    // We keep versions in case the user closes the application before
    // a flush cycle is finished.
    khDaysMM::u32 PreviousFlushVersion;
    khDaysMM::u32 FlushVersion;
};

#endif // SAVEMANAGER_H
