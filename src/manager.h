/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <QObject>
#include <QString>
#include "types.h"
#include "filter.h"

namespace History
{

class ManagerPrivate;

class Manager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Manager)

public:
    ~Manager();

    static Manager *instance();

    ThreadViewPtr queryThreads(EventType type,
                               const SortPtr &sort = SortPtr(),
                               const Filter &filter = Filter());

    EventViewPtr queryEvents(EventType type,
                             const SortPtr &sort = SortPtr(),
                             const Filter &filter = Filter());

    EventPtr getSingleEvent(EventType type, const QString &accountId, const QString &threadId, const QString &eventId, bool useCache = true);

    ThreadPtr threadForParticipants(const QString &accountId,
                                    EventType type,
                                    const QStringList &participants,
                                    History::MatchFlags matchFlags = History::MatchCaseSensitive,
                                    bool create = false);
    ThreadPtr getSingleThread(EventType type, const QString &accountId, const QString &threadId, bool useCache = true);

    bool writeEvents(const History::Events &events);
    bool removeThreads(const Threads &threads);
    bool removeEvents(const Events &events);

Q_SIGNALS:
    void threadsAdded(const History::Threads &threads);
    void threadsModified(const History::Threads &threads);
    void threadsRemoved(const History::Threads &threads);

    void eventsAdded(const History::Events &events);
    void eventsModified(const History::Events &events);
    void eventsRemoved(const History::Events &events);

private:
    Manager();
    QScopedPointer<ManagerPrivate> d_ptr;
};

}

#endif
