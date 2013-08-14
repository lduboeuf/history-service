/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef HISTORY_TEXT_EVENT_ATTACHMENT_P_H
#define HISTORY_TEXT_EVENT_ATTACHMENT_P_H

#include <QString>
#include "types.h"

namespace History
{

class TextEventAttachment;

class TextEventAttachmentPrivate
{
public:
    TextEventAttachmentPrivate(const QString &theAccountId,
                         const QString &theThreadId,
                         const QString &theEventId,
                         const QString &theAttachmentId,
                         const QString &theContentType,
                         const QString &theFilePath);
    virtual ~TextEventAttachmentPrivate();

    QString accountId;
    QString threadId;
    QString eventId;
    QString attachmentId;
    QString contentType;
    QString filePath;
};

}

#endif // HISTORY_TEXT_EVENT_ATTACHMENT_P_H
