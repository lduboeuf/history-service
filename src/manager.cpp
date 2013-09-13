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

#include "manager.h"
#include "manager_p.h"
#include "managerdbus_p.h"
#include "eventview.h"
#include "intersectionfilter.h"
#include "itemfactory.h"
#include "pluginmanager_p.h"
#include "plugin.h"
#include "textevent.h"
#include "thread.h"
#include "voiceevent.h"
#include <QDebug>

#define HISTORY_INTERFACE "com.canonical.HistoryService"

namespace History
{

// ------------- ManagerPrivate ------------------------------------------------

ManagerPrivate::ManagerPrivate(const QString &theBackend)
    : dbus(new ManagerDBus())
{
}

ManagerPrivate::~ManagerPrivate()
{
}

// ------------- Manager -------------------------------------------------------

Manager::Manager(const QString &backendPlugin)
    : d_ptr(new ManagerPrivate(backendPlugin))
{
    Q_D(Manager);

    if (!PluginManager::instance()->plugins().isEmpty()) {
        d->backend = PluginManager::instance()->plugins().first();
    }

    // Propagate the signals from the event watcher
    connect(d->dbus.data(),
            SIGNAL(threadsAdded(History::Threads)),
            SIGNAL(threadsAdded(History::Threads)));
    connect(d->dbus.data(),
            SIGNAL(threadsModified(History::Threads)),
            SIGNAL(threadsModified(History::Threads)));
    connect(d->dbus.data(),
            SIGNAL(threadsRemoved(History::Threads)),
            SIGNAL(threadsRemoved(History::Threads)));
    connect(d->dbus.data(),
            SIGNAL(eventsAdded(History::Events)),
            SIGNAL(eventsAdded(History::Events)));
    connect(d->dbus.data(),
            SIGNAL(eventsModified(History::Events)),
            SIGNAL(eventsModified(History::Events)));
    connect(d->dbus.data(),
            SIGNAL(eventsRemoved(History::Events)),
            SIGNAL(eventsRemoved(History::Events)));
}

Manager::~Manager()
{
}

Manager *Manager::instance()
{
    static Manager *self = new Manager();
    return self;
}

ThreadViewPtr Manager::queryThreads(EventType type,
                                    const SortPtr &sort,
                                    const FilterPtr &filter)
{
    Q_D(Manager);
    if (d->backend) {
        return d->backend->queryThreads(type, sort, filter);
    }

    return ThreadViewPtr();
}

EventViewPtr Manager::queryEvents(EventType type,
                                  const SortPtr &sort,
                                  const FilterPtr &filter)
{
    Q_D(Manager);
    if (d->backend) {
        return d->backend->queryEvents(type, sort, filter);
    }

    return EventViewPtr();
}

EventPtr Manager::getSingleEvent(EventType type, const QString &accountId, const QString &threadId, const QString &eventId, bool useCache)
{
    Q_D(Manager);

    EventPtr event;
    if (useCache) {
        event = ItemFactory::instance()->cachedEvent(accountId, threadId, eventId, type);
    }

    if (event.isNull() && d->backend) {
        event = d->backend->getSingleEvent(type, accountId, threadId, eventId);
    }

    return event;
}

ThreadPtr Manager::threadForParticipants(const QString &accountId,
                                         EventType type,
                                         const QStringList &participants,
                                         MatchFlags matchFlags,
                                         bool create)
{
    Q_D(Manager);

    if (!d->backend) {
        return ThreadPtr();
    }

    ThreadPtr thread = d->backend->threadForParticipants(accountId, type, participants, matchFlags);

    // if the thread is null, create a new if possible/desired.
    if (thread.isNull() && create) {
        thread = d->backend->createThreadForParticipants(accountId, type, participants);
        d->dbus->notifyThreadsAdded(Threads() << thread);
    }

    return thread;
}

ThreadPtr Manager::getSingleThread(EventType type, const QString &accountId, const QString &threadId, bool useCache)
{
    Q_D(Manager);

    // try to use the cached instance to avoid querying the backend
    ThreadPtr thread;
    if (useCache) {
        thread = ItemFactory::instance()->cachedThread(accountId, threadId, type);
    }

    // and if it isn´t there, get from the backend
    if (thread.isNull() && d->backend) {
        thread = d->backend->getSingleThread(type, accountId, threadId);
    }

    return thread;
}

bool Manager::writeTextEvents(const TextEvents &textEvents)
{
    Q_D(Manager);

    if (!d->backend) {
        return false;
    }

    d->backend->beginBatchOperation();

    Events events;
    Threads threads;
    Q_FOREACH(const TextEventPtr &textEvent, textEvents) {
        // save the thread so that the thread updated signal can be emitted
        ThreadPtr thread = getSingleThread(EventTypeText, textEvent->accountId(), textEvent->threadId());
        if (!threads.contains(thread)) {
            threads << thread;
        }
        d->backend->writeTextEvent(textEvent);
        events << textEvent.staticCast<Event>();
    }

    d->backend->endBatchOperation();

    d->dbus->notifyEventsAdded(events);
    d->dbus->notifyThreadsModified(threads);
    return true;
}

bool Manager::writeVoiceEvents(const VoiceEvents &voiceEvents)
{
    Q_D(Manager);

    if (!d->backend) {
        return false;
    }

    d->backend->beginBatchOperation();

    Events events;
    Threads threads;
    Q_FOREACH(const VoiceEventPtr &voiceEvent, voiceEvents) {
        // save the thread so that the thread updated signal can be emitted
        ThreadPtr thread = getSingleThread(EventTypeVoice, voiceEvent->accountId(), voiceEvent->threadId());
        if (!threads.contains(thread)) {
            threads << thread;
        }
        d->backend->writeVoiceEvent(voiceEvent);
        events << voiceEvent.staticCast<Event>();
    }

    d->backend->endBatchOperation();

    d->dbus->notifyEventsAdded(events);
    d->dbus->notifyThreadsModified(threads);

    return true;
}

bool Manager::removeThreads(const Threads &threads)
{
    Q_D(Manager);

    if (!d->backend) {
        return false;
    }

    Events events;
    Threads removedThreads;

    d->backend->beginBatchOperation();

    // remove all the threads that are empty.
    // the threads that are not empty will be removed
    // once their items are removed
    Q_FOREACH(const ThreadPtr &thread, threads) {
        if (thread->count() > 0) {
            IntersectionFilterPtr filter(new IntersectionFilter());
            filter->append(FilterPtr(new Filter("accountId", thread->accountId())));
            filter->append(FilterPtr(new Filter("threadId", thread->threadId())));
            EventViewPtr eventView = queryEvents(thread->type(), SortPtr(), filter);
            Events page = eventView->nextPage();
            while (page.count()) {
                events << page;
                page = eventView->nextPage();
            }
        } else {
            if (!d->backend->removeThread(thread)) {
                d->backend->rollbackBatchOperation();
                return false;
            }
            removedThreads << thread;
        }
    }

    d->backend->endBatchOperation();
    if (!removedThreads.isEmpty()) {
        d->dbus->notifyThreadsRemoved(removedThreads);
    }

    return removeEvents(events);
}

bool Manager::removeEvents(const Events &events)
{
    Q_D(Manager);

    if (!d->backend) {
        return false;
    }

    Threads threadsToUpdate;

    d->backend->beginBatchOperation();

    Q_FOREACH(const EventPtr &event, events) {
        QString accountId = event->accountId();
        QString threadId = event->threadId();
        EventType type = event->type();

        TextEventPtr textEvent;
        VoiceEventPtr voiceEvent;

        switch (type) {
        case EventTypeText:
            textEvent = event.staticCast<TextEvent>();
            if (!d->backend->removeTextEvent(textEvent)) {
                d->backend->rollbackBatchOperation();
                return false;
            }
            break;
        case EventTypeVoice:
            voiceEvent = event.staticCast<VoiceEvent>();
            if (!d->backend->removeVoiceEvent(voiceEvent)) {
                d->backend->rollbackBatchOperation();
                return false;
            }
            break;
        }

        // get the thread from the item to check if it needs to update
        ThreadPtr thread = getSingleThread(type, accountId, threadId, false);
        if (!threadsToUpdate.contains(thread)) {
            threadsToUpdate << thread;
        }
    }

    d->backend->endBatchOperation();

    d->dbus->notifyEventsRemoved(events);

    // now update or remove threads
    Threads threadsToRemove;
    Threads threadsModified;

    Q_FOREACH(const ThreadPtr &thread, threadsToUpdate) {
        if (thread->count() == 0) {
            threadsToRemove << thread;
        } else {
            threadsModified << thread;
        }
    }

    if (!threadsToRemove.isEmpty()) {
        removeThreads(threadsToRemove);
    }

    if (!threadsModified.isEmpty()) {
        d->dbus->notifyThreadsModified(threadsModified);
    }

    return true;
}

}

