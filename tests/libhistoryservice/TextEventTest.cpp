/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include "textevent.h"

class TextEventTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreateNewEvent_data();
    void testCreateNewEvent();
    void testCastToEventAndBack();
    void testFromProperties_data();
    void testFromProperties();
    void testFromNullProperties();
    void testProperties_data();
    void testProperties();
    void testSetProperties();
    void testNoSentDateTime();

private:
    History::Participants participantsFromIdentifiers(const QString &accountId, const QStringList &identifiers);
};

void TextEventTest::testCreateNewEvent_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<QDateTime>("sentTime");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<QString>("message");
    QTest::addColumn<int>("messageType");
    QTest::addColumn<int>("messageStatus");
    QTest::addColumn<QDateTime>("readTimestamp");
    QTest::addColumn<QString>("subject");
    QTest::addColumn<int>("informationType");
    QTest::addColumn<QStringList>("participants");

    QTest::newRow("unread message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10) << QDateTime::currentDateTime()
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject" << (int) History::InformationTypeJoined
                                    << (QStringList() << "testParticipant");
    QTest::newRow("read message")   << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-10) << QDateTime::currentDateTime()
                                    << false << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject 2" << (int) History::InformationTypeLeaving
                                    << (QStringList() << "testParticipant2");
    QTest::newRow("message status") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10) << QDateTime::currentDateTime()
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << (int)History::MessageStatusAccepted
                                    << QDateTime::currentDateTime().addDays(-5) << "Test Subject 3" << (int) History::InformationTypeSelfLeaving
                                    << (QStringList() << "testParticipant");
    QTest::newRow("multi party message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10) << QDateTime::currentDateTime()
                                    << true << "One Test Message" << (int)History::MessageTypeMultiPart
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << QString() << (int) History::InformationTypeNone
                                    << (QStringList() << "testParticipant");
    QTest::newRow("multiple participants") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-7) << QDateTime::currentDateTime()
                                    << true << "One Test Message 2" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-4) << QString() << (int) History::InformationTypeNone
                                    << (QStringList() << "one" << "two" << "three" << "four");
}

void TextEventTest::testCreateNewEvent()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(QDateTime, sentTime);
    QFETCH(bool, newEvent);
    QFETCH(QString, message);
    QFETCH(int, messageType);
    QFETCH(int, messageStatus);
    QFETCH(QDateTime, readTimestamp);
    QFETCH(QString, subject);
    QFETCH(int, informationType);
    QFETCH(QStringList, participants);
    History::TextEvent event(accountId, threadId, eventId, senderId, timestamp, sentTime, newEvent,
                             message, (History::MessageType)messageType, (History::MessageStatus)messageStatus,
                             readTimestamp, subject, (History::InformationType) informationType, History::TextEventAttachments(),
                             participantsFromIdentifiers(accountId, participants));

    // check that the values are properly set
    QCOMPARE(event.accountId(), accountId);
    QCOMPARE(event.threadId(), threadId);
    QCOMPARE(event.eventId(), eventId);
    QCOMPARE(event.senderId(), senderId);
    QCOMPARE(event.timestamp(), timestamp);
    QCOMPARE(event.sentTime(), sentTime);
    QCOMPARE(event.newEvent(), newEvent);
    QCOMPARE(event.message(), message);
    QCOMPARE(event.messageType(), (History::MessageType)messageType);
    QCOMPARE(event.messageStatus(), (History::MessageStatus)messageStatus);
    QCOMPARE(event.readTimestamp(), readTimestamp);
    QCOMPARE(event.subject(), subject);
    QCOMPARE(event.participants().identifiers(), participants);
}

void TextEventTest::testCastToEventAndBack()
{
    History::TextEvent textEvent("oneAccountId", "oneThreadId", "oneEventId", "oneSender", QDateTime::currentDateTime(), QDateTime::currentDateTime(),
                                 true, "Hello", History::MessageTypeText);

    // test the copy constructor
    History::Event historyEvent(textEvent);
    QVERIFY(historyEvent == textEvent);
    History::TextEvent castBack(historyEvent);
    QVERIFY(castBack == textEvent);

    // and now the assignment operator
    History::Event anotherEvent;
    anotherEvent = textEvent;
    QVERIFY(anotherEvent == textEvent);
    History::TextEvent backAgain;
    backAgain = anotherEvent;
    QVERIFY(backAgain == textEvent);
}

void TextEventTest::testFromProperties_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<QString>("message");
    QTest::addColumn<int>("messageType");
    QTest::addColumn<int>("messageStatus");
    QTest::addColumn<QDateTime>("readTimestamp");
    QTest::addColumn<QString>("subject");
    QTest::addColumn<int>("informationType");
    QTest::addColumn<QStringList>("participants");

    QTest::newRow("unread message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject" << (int) History::InformationTypeJoined
                                    << (QStringList() << "testParticipant");
    QTest::newRow("read message")   << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-10)
                                    << false << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject 2" << (int) History::InformationTypeNone
                                    << (QStringList() << "testParticipant2");
    QTest::newRow("message status") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << (int)History::MessageStatusAccepted
                                    << QDateTime::currentDateTime().addDays(-5) << "Test Subject 3" << (int) History::InformationTypeNone
                                    << (QStringList() << "testParticipant");
    QTest::newRow("multi party message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeMultiPart
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << QString() << (int) History::InformationTypeNone
                                    << (QStringList() << "testParticipant");
    QTest::newRow("multiple participants") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-7)
                                    << true << "One Test Message 2" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-4) << QString() << (int) History::InformationTypeNone
                                    << (QStringList() << "one" << "two" << "three" << "four");
}

void TextEventTest::testFromProperties()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(bool, newEvent);
    QFETCH(QString, message);
    QFETCH(int, messageType);
    QFETCH(int, messageStatus);
    QFETCH(QDateTime, readTimestamp);
    QFETCH(QString, subject);
    QFETCH(int, informationType);
    QFETCH(QStringList, participants);

    QVariantMap properties;
    properties[History::FieldAccountId] = accountId;
    properties[History::FieldThreadId] = threadId;
    properties[History::FieldEventId] = eventId;
    properties[History::FieldSenderId] = senderId;
    properties[History::FieldTimestamp] = timestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz");
    properties[History::FieldSentTime] = timestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz");
    properties[History::FieldNewEvent] = newEvent;
    properties[History::FieldMessage] = message;
    properties[History::FieldMessageType] = messageType;
    properties[History::FieldMessageStatus] = messageStatus;
    properties[History::FieldReadTimestamp] = readTimestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz");
    properties[History::FieldSubject] = subject;
    properties[History::FieldInformationType] = informationType;
    properties[History::FieldParticipants] = participantsFromIdentifiers(accountId, participants).toVariantList();

    History::TextEvent textEvent = History::TextEvent::fromProperties(properties);
    QCOMPARE(textEvent.accountId(), accountId);
    QCOMPARE(textEvent.threadId(), threadId);
    QCOMPARE(textEvent.eventId(), eventId);
    QCOMPARE(textEvent.senderId(), senderId);
    QCOMPARE(textEvent.timestamp().toString(Qt::ISODate), timestamp.toString(Qt::ISODate));
    QCOMPARE(textEvent.sentTime().toString(Qt::ISODate), timestamp.toString(Qt::ISODate));
    QCOMPARE(textEvent.newEvent(), newEvent);
    QCOMPARE(textEvent.message(), message);
    QCOMPARE(textEvent.messageType(), (History::MessageType) messageType);
    QCOMPARE(textEvent.messageStatus(), (History::MessageStatus) messageStatus);
    QCOMPARE(textEvent.readTimestamp().toString(Qt::ISODate), readTimestamp.toString(Qt::ISODate));
    QCOMPARE(textEvent.subject(), subject);
    QCOMPARE(textEvent.informationType(), (History::InformationType) informationType);
    QCOMPARE(textEvent.participants().identifiers(), participants);
}

void TextEventTest::testFromNullProperties()
{
    // just to make sure, test that calling ::fromProperties() on an empty map returns a null event
    History::Event nullEvent = History::TextEvent::fromProperties(QVariantMap());
    QVERIFY(nullEvent.isNull());
    QCOMPARE(nullEvent.type(), History::EventTypeNull);
}

void TextEventTest::testProperties_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<QDateTime>("sentTime");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<QString>("message");
    QTest::addColumn<int>("messageType");
    QTest::addColumn<int>("messageStatus");
    QTest::addColumn<QDateTime>("readTimestamp");
    QTest::addColumn<QString>("subject");
    QTest::addColumn<int>("informationType");
    QTest::addColumn<QStringList>("participants");

    QTest::newRow("unread message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10) << QDateTime::currentDateTime()
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject"  << (int) History::InformationTypeNone
                                    << (QStringList() << "testParticipant");
    QTest::newRow("read message")   << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-10) << QDateTime::currentDateTime()
                                    << false << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject 2"  << (int) History::InformationTypeNone
                                    << (QStringList() << "testParticipant2");
    QTest::newRow("message status") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10) << QDateTime::currentDateTime()
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << (int)History::MessageStatusAccepted
                                    << QDateTime::currentDateTime().addDays(-5) << "Test Subject 3" << (int) History::InformationTypeNone
                                    << (QStringList() << "testParticipant");
    QTest::newRow("multi party message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10) << QDateTime::currentDateTime()
                                    << true << "One Test Message" << (int)History::MessageTypeMultiPart
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << QString() << (int) History::InformationTypeNone
                                    << (QStringList() << "testParticipant");
    QTest::newRow("multiple participants") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-7) << QDateTime::currentDateTime()
                                    << true << "One Test Message 2" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-4) << QString() << (int) History::InformationTypeNone
                                    << (QStringList() << "one" << "two" << "three" << "four");
}

void TextEventTest::testProperties()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(QDateTime, sentTime);
    QFETCH(bool, newEvent);
    QFETCH(QString, message);
    QFETCH(int, messageType);
    QFETCH(int, messageStatus);
    QFETCH(QDateTime, readTimestamp);
    QFETCH(QString, subject);
    QFETCH(int, informationType);
    QFETCH(QStringList, participants);
    History::TextEvent event(accountId, threadId, eventId, senderId, timestamp, sentTime, newEvent,
                             message, (History::MessageType)messageType, (History::MessageStatus)messageStatus,
                             readTimestamp, subject, History::InformationTypeNone, History::TextEventAttachments(),
                             participantsFromIdentifiers(accountId, participants));

    QVariantMap properties = event.properties();
    QCOMPARE(properties[History::FieldAccountId].toString(), accountId);
    QCOMPARE(properties[History::FieldThreadId].toString(), threadId);
    QCOMPARE(properties[History::FieldEventId].toString(), eventId);
    QCOMPARE(properties[History::FieldSenderId].toString(), senderId);
    QCOMPARE(properties[History::FieldTimestamp].toString(), timestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz"));
    QCOMPARE(properties[History::FieldSentTime].toString(), sentTime.toString("yyyy-MM-ddTHH:mm:ss.zzz"));
    QCOMPARE(properties[History::FieldNewEvent].toBool(), newEvent);
    QCOMPARE(properties[History::FieldMessage].toString(), message);
    QCOMPARE(properties[History::FieldMessageType].toInt(), messageType);
    QCOMPARE(properties[History::FieldMessageStatus].toInt(), messageStatus);
    QCOMPARE(properties[History::FieldReadTimestamp].toString(), readTimestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz"));
    QCOMPARE(properties[History::FieldSubject].toString(), subject);
    QCOMPARE(properties[History::FieldInformationType].toInt(), informationType);
    QCOMPARE(History::Participants::fromVariantList(properties[History::FieldParticipants].toList()).identifiers(), participants);
}

void TextEventTest::testSetProperties()
{
    History::TextEvent textEvent("oneAccountId", "oneThreadId", "oneEventId", "oneSender", QDateTime::currentDateTime(), QDateTime::currentDateTime(),
                                 true, "Hello", History::MessageTypeText);
    QDateTime readTimestamp = QDateTime::currentDateTime();
    QDateTime sentTime = QDateTime::currentDateTime();
    History::MessageStatus status = History::MessageStatusDelivered;
    bool newEvent = false;
    textEvent.setReadTimestamp(readTimestamp);
    textEvent.setMessageStatus(status);
    textEvent.setNewEvent(newEvent);
    textEvent.setSentTime(sentTime);
    QCOMPARE(textEvent.readTimestamp(), readTimestamp);
    QCOMPARE(textEvent.messageStatus(), status);
    QCOMPARE(textEvent.newEvent(), newEvent);
    QCOMPARE(textEvent.sentTime(), sentTime);
}

void TextEventTest::testNoSentDateTime()
{
    History::TextEvent textEvent("oneAccountId", "oneThreadId", "oneEventId", "oneSender", QDateTime::currentDateTime(),
                                 true, "Hello", History::MessageTypeText);
    // expect sentTime to be equals to receivedTime if not set
    QCOMPARE(textEvent.sentTime(), textEvent.timestamp());
}

History::Participants TextEventTest::participantsFromIdentifiers(const QString &accountId, const QStringList &identifiers)
{
    History::Participants participants;
    Q_FOREACH(const QString &identifier, identifiers) {
        participants << History::Participant(accountId, identifier);
    }
    return participants;
}

QTEST_MAIN(TextEventTest)
#include "TextEventTest.moc"
