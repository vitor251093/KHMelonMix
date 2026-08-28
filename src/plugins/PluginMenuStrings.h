#ifndef MELONDS_PLUGINMENUSTRINGS_H
#define MELONDS_PLUGINMENUSTRINGS_H

namespace Plugins
{

struct MenuStrings {
    const char* title;
    const char* cont;
    const char* skip;
    const char* withdraw;
    const char* yes;
    const char* no;
    const char* areYouSure;
};

const MenuStrings& menuStrings(int language);

}

#endif //MELONDS_PLUGINMENUSTRINGS_H
