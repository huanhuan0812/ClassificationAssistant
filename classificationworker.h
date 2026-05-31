// classificationworker.h
#ifndef CLASSIFICATIONWORKER_H
#define CLASSIFICATIONWORKER_H

#include <QObject>
#include <QStringList>
#include <QJsonArray>
#include <QNetworkReply>

class ClassificationWorker : public QObject {
    Q_OBJECT

public:
    QString apiUrl;
    QStringList filePaths;      // 文件路径列表（用于单文件模式）
    bool useBatchMode = false;  // 是否使用批量模式
    
signals:
    void finished(const QJsonArray& results);
    void error(const QString& message);
    void progress(int current, int total);
    void fileResult(int index, const QString& filename, bool success, 
                    const QString& className, double confidence, 
                    const QString& errorMsg, const QStringList& keywords);
    
public slots:
    void process();

private:
    void processSingle(QJsonArray& results, int timeoutMs, int delayMs);
    void processBatch(QJsonArray& results, int timeoutMs);
    void processReply(QNetworkReply* reply, int index, const QString& filename, 
                      const QString& filepath, QJsonArray& results);
    int findFileIndex(const QString& filepath) const;
};

#endif // CLASSIFICATIONWORKER_H