#ifndef RETROACHIEVEMENTS_H
#define RETROACHIEVEMENTS_H

#include <list>
#include "RAAchievement.h"
#include "RACallback.h"
#include "Savestate.h"

namespace RetroAchievements
{

void Init(RACallback* callback);
bool LoadAchievements(std::list<RetroAchievements::RAAchievement> achievements);
void UnloadAchievements(std::list<RetroAchievements::RAAchievement> achievements);
void SetupRichPresence(std::string richPresenceScript);
std::string GetRichPresenceStatus();
bool DoSavestate(Savestate* savestate);
void Reset();
void DeInit();
void FrameUpdate();
}

#endif //RETROACHIEVEMENTS_H
