#include "updates.h"
#include "database.h"

#include <QDebug>

#include <bits/shared_ptr.h>

Updates::Updates(PamacUpdates *upd)
    :m_updates(std::shared_ptr<PamacUpdates>(upd,g_object_unref))
{
}

GPtrArray* Updates::get()
{
    return pamac_updates_get_repos_updates(m_updates.get());
}
