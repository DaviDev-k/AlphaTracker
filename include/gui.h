#ifndef GUI_H
#define GUI_H

#include "settings.h"


struct Gui {

    void homeLoading(Settings set);

    void homeUpdate(Settings set, int idx, int source);

    void homeSelect(Settings set, int idx);

    void settingsStable(Settings set);

    void settingsSelect(Settings set, int slc);

    void guiStable(Settings set);

    void instructions();

};


#endif // GUI_H