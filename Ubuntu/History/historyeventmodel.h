/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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

#ifndef HISTORYEVENTMODEL_H
#define HISTORYEVENTMODEL_H

#include "historymodel.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QStringList>

class HistoryEventModel : public HistoryModel
{
    Q_OBJECT
    Q_ENUMS(EventRole)
public:
    enum EventRole {
        EventIdRole = HistoryModel::LastRole,
        SenderIdRole,
        TimestampRole,
        DateRole,
        NewEventRole,
        TextMessageRole,
        TextMessageTypeRole,
        TextMessageStatusRole,
        TextReadTimestampRole,
        TextReadSubjectRole,
        TextMessageAttachmentsRole,
        CallMissedRole,
        CallDurationRole,
        LastEventRole
    };

    explicit HistoryEventModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    QVariant eventData(const History::Event &event, int role) const;

    Q_INVOKABLE virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE virtual void fetchMore(const QModelIndex &parent = QModelIndex());

    virtual QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE bool removeEvent(const QString &accountId, const QString &threadId, const QString &eventId, int eventType);
    Q_INVOKABLE bool markEventAsRead(const QString &accountId, const QString &threadId, const QString &eventId, int eventType);
    Q_INVOKABLE bool removeEventAttachment(const QString &accountId, const QString &threadId, const QString &eventId, int eventType, const QString &attachmentId);

protected Q_SLOTS:
    virtual void updateQuery();
    virtual void onEventsAdded(const History::Events &events);
    virtual void onEventsModified(const History::Events &events);
    virtual void onEventsRemoved(const History::Events &events);

protected:
    void timerEvent(QTimerEvent *event);
    History::Events fetchNextPage();

private:
    History::EventViewPtr mView;
    History::Events mEvents;
    bool mCanFetchMore;
    QHash<int, QByteArray> mRoles;
    mutable QMap<History::TextEvent, QList<QVariant> > mAttachmentCache;
    History::Events mEventWritingQueue;
    int mEventWritingTimer;
};

#endif // HISTORYEVENTMODEL_H
