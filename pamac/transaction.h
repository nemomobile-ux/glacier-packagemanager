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

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QObject>
#include <pamac.h>

#include "database.h"

class Transaction : public QObject
{
    Q_OBJECT
public:
    explicit Transaction(QObject *parent = nullptr);
    ~Transaction();
    Q_INVOKABLE void getAuthorization();
    Q_INVOKABLE void install(QStringList packages);
    Q_INVOKABLE void remove(QStringList packages);
    Q_INVOKABLE void upgrade();

Q_SIGNALS:
    void getAuthorizationReady(bool authorized);
    void authorizationFail();
    void transactionStarted();
    void transactionFinished();
    void emitActionProgress(const QString& emitAction,const QString& status, double progress);
    void emitAction(const QString& emitAction);
    void emitError(const QString& error);

    void startWaiting();
    void stopWaiting();
    void startDownloading();
    void stopDownloading();
    void startPreparing();
    void stopPreparing();

private:
    DataBase* m_database = nullptr;
    PamacTransaction* m_pmTransaction;
    PamacTransactionSummary* m_pmTransactionSummary;

    static void getAuthorizationFinish(GObject *source_object, GAsyncResult *res, gpointer user_data);
    static void transactionFinish(GObject *source_object, GAsyncResult *res, gpointer user_data);

    void debug(const QString& emitAction,const QString& status, double progress);
    void run(bool auth);

    bool m_authInProgress = false;
};

#endif // TRANSACTION_H
