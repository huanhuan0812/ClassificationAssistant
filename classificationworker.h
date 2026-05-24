// classificationworker.h
#ifndef CLASSIFICATIONWORKER_H
#define CLASSIFICATIONWORKER_H

#include <QObject>
#include <QStringList>
#include <QJsonArray>

class ClassificationWorker : public QObject {
    Q_OBJECT

public:
    QString apiUrl;
    QStringList filePaths;
    
signals:
    void finished(const QJsonArray& results);
    void error(const QString& message);
    void progress(int current, int total);
    void fileResult(int index, const QString& filename, bool success, 
                    const QString& className, double confidence, 
                    const QString& errorMsg, const QStringList& keywords);
    
public slots:
    void process();
};

#endif // CLASSIFICATIONWORKER_H