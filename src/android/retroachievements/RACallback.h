#ifndef RACALLBACK_H
#define RACALLBACK_H

namespace RetroAchievements
{

class RACallback
{
public:
    virtual void onAchievementPrimed(long achievementId) = 0;
    virtual void onAchievementTriggered(long achievementId) = 0;
    virtual void onAchievementUnprimed(long achievementId) = 0;
};

}

#endif //RACALLBACK_H
