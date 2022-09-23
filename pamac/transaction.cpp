/*
 * Copyright (C) 2021 Chupligin Sergey <neochapay@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "transaction.h"
#include <QDebug>
#include <QEventLoop>

Transaction::Transaction(QObject* parent)
    : QObject(parent)
{
    m_database = new DataBase();
    m_pmTransaction = pamac_transaction_new(m_database->db());

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "emit_action",
        reinterpret_cast<GCallback>(+([](GObject* obj, char* action, Transaction* t) {
            Q_UNUSED(obj);
            t->emitAction(QString::fromUtf8(action));
        })),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "emit_action_progress",
        reinterpret_cast<GCallback>(+[](GObject* obj, char* action, char* status, double progress, Transaction* t) {
            Q_UNUSED(obj);
            t->emitActionProgress(QString::fromUtf8(action), QString::fromUtf8(status), progress);
        }),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "emit_error",
        reinterpret_cast<GCallback>(+[](GObject* obj, char* message, char** details, int size, Transaction* t) {
            Q_UNUSED(obj);
            qWarning() << "Error" << message << details;
        }),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "emit_warning",
        reinterpret_cast<GCallback>(+[](GObject* obj, char* warning, Transaction* t) {
            Q_UNUSED(obj);
            qWarning() << "Warning" << warning;
        }),
        this);
    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "start_preparing",
        reinterpret_cast<GCallback>(+[](GObject* obj, Transaction* t) {
            Q_UNUSED(obj);
            Q_EMIT t->startPreparing();
        }),
        this);
    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "stop_preparing",
        reinterpret_cast<GCallback>(+[](GObject* obj, Transaction* t) {
            Q_UNUSED(obj);
            Q_EMIT t->stopPreparing();
        }),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "emit_script_output",
        reinterpret_cast<GCallback>(+[](GObject* obj, char* message, Transaction* t) {
            Q_UNUSED(obj);
            qDebug() << "emitScriptOutput" << message;
        }),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "important_details_outpout",
        reinterpret_cast<GCallback>(+[](GObject* obj, bool message, Transaction* t) {
            Q_UNUSED(obj);
            qDebug() << "importantDetailsOutput" << message;
        }),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "start_waiting",
        reinterpret_cast<GCallback>(+[](GObject* obj, Transaction* t) {
            Q_EMIT t->startWaiting();
        }),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "stop_waiting",
        reinterpret_cast<GCallback>(+[](GObject* obj, Transaction* t) {
            Q_EMIT t->stopWaiting();
        }),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "start_downloading",
        reinterpret_cast<GCallback>(+[](GObject* obj, Transaction* t) {
            Q_EMIT t->startDownloading();
        }),
        this);

    g_signal_connect(static_cast<PamacTransaction*>(m_pmTransaction), "stop_downloading",
        reinterpret_cast<GCallback>(+[](GObject* obj, Transaction* t) {
            Q_EMIT t->stopDownloading();
        }),
        this);

    connect(this, &Transaction::getAuthorizationReady, this, &Transaction::run);
}

Transaction::~Transaction()
{
    pamac_transaction_quit_daemon(m_pmTransaction);
}

void Transaction::getAuthorization()
{
    if (!m_authInProgress) {
        m_authInProgress = true;
        pamac_transaction_get_authorization_async(m_pmTransaction, getAuthorizationFinish, this);
    }
}

void Transaction::install(QStringList packages)
{
    for (int i = 0; i < packages.size(); i++) {
        pamac_transaction_add_pkg_to_install(m_pmTransaction, packages.at(i).toUtf8());
    }
    getAuthorization();
}

void Transaction::remove(QStringList packages)
{
    for (int i = 0; i < packages.size(); i++) {
        pamac_transaction_add_pkg_to_remove(m_pmTransaction, packages.at(i).toUtf8());
    }
    getAuthorization();
}

void Transaction::upgrade()
{
    pamac_transaction_add_pkgs_to_upgrade(m_pmTransaction, true);
    Q_EMIT transactionStarted();
}

void Transaction::getAuthorizationFinish(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    Transaction* transaction = static_cast<Transaction*>(user_data);
    transaction->m_authInProgress = false;
    Q_EMIT transaction->getAuthorizationReady(pamac_transaction_get_authorization_finish(transaction->m_pmTransaction, res));
}

void Transaction::transactionFinish(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    Transaction* transaction = static_cast<Transaction*>(user_data);
    pamac_transaction_quit_daemon(transaction->m_pmTransaction);
    transaction->m_database->refresh();

    Q_EMIT transaction->transactionFinished();
}

void Transaction::run(bool auth)
{
    if (!auth) {
        Q_EMIT authorizationFail();
        pamac_transaction_cancel(m_pmTransaction);
    } else {
        pamac_transaction_run_async(m_pmTransaction, transactionFinish, this);
        Q_EMIT transactionStarted();
    }
}
