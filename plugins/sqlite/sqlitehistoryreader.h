#ifndef SQLITEHISTORYREADER_H
#define SQLITEHISTORYREADER_H

#include <HistoryReader>

class SQLiteHistoryReader : public HistoryReader
{
    Q_OBJECT
public:
    explicit SQLiteHistoryReader(QObject *parent = 0);
};

#endif // SQLITEHISTORYREADER_H
