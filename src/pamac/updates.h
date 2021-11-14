#ifndef UPDATES_H
#define UPDATES_H

#include <QObject>
#include "pamac.h"

class Updates : public QObject
{
    Q_GADGET
public:
    Updates()=default;
    Updates(PamacUpdates* upd);
    GPtrArray *get();

private:
    PamacUpdates* m_updates;
};

#endif // UPDATES_H
