// classificationworker.cpp
#include "classificationworker.h"
#include "settings.h"
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>
#include <QThread>

void ClassificationWorker::process() {
    QJsonArray results;
    Settings& settings = Settings::instance();
    int timeoutMs = settings.getTimeoutSeconds() * 1000;
    int delayMs = settings.getRequestDelayMs();
    
    for (int i = 0; i < filePaths.size(); i++) {
        emit progress(i + 1, filePaths.size());
        
        QFile file(filePaths[i]);
        QString filename = QFileInfo(filePaths[i]).fileName();
        
        if (!file.open(QIODevice::ReadOnly)) {
            emit fileResult(i, filename, false, "", 0.0, "无法打开文件", QStringList());
            
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["filename"] = filename;
            errorObj["error"] = "无法打开文件";
            results.append(errorObj);
            continue;
        }
        
        // 创建multipart请求
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        
        QHttpPart filePart;
        QString disposition = QString("form-data; name=\"file\"; filename=\"%1\"")
            .arg(filename);
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
        filePart.setBodyDevice(&file);
        file.setParent(multiPart);
        
        multiPart->append(filePart);
        
        QNetworkRequest request(QUrl(apiUrl + "/predict"));
        
        QNetworkAccessManager manager;
        QNetworkReply *reply = manager.post(request, multiPart);
        
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(timeoutMs);
        
        loop.exec();
        
        if (timer.isActive()) {
            timer.stop();
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                
                if (!doc.isNull() && doc.isObject()) {
                    QJsonObject resultObj = doc.object();
                    bool success = resultObj.value("success").toBool(true);
                    
                    if (success) {
                        QString className = resultObj.value("predicted_class").toString();
                        double confidence = resultObj.value("confidence").toDouble(0.0);
                        
                        QStringList keywords;
                        if (resultObj.contains("keywords")) {
                            QJsonArray keywordsArray = resultObj["keywords"].toArray();
                            for (const QJsonValue& kw : keywordsArray) {
                                keywords.append(kw.toString());
                            }
                        }
                        
                        emit fileResult(i, filename, true, className, confidence, "", keywords);
                        
                        QJsonObject successObj;
                        successObj["success"] = true;
                        successObj["filename"] = filename;
                        successObj["predicted_class"] = className;
                        successObj["confidence"] = confidence;
                        results.append(successObj);
                    } else {
                        QString errorMsg = resultObj.value("error").toString();
                        emit fileResult(i, filename, false, "", 0.0, errorMsg, QStringList());
                        
                        QJsonObject errorObj;
                        errorObj["success"] = false;
                        errorObj["filename"] = filename;
                        errorObj["error"] = errorMsg;
                        results.append(errorObj);
                    }
                } else {
                    emit fileResult(i, filename, false, "", 0.0, "无效的JSON响应", QStringList());
                    
                    QJsonObject errorObj;
                    errorObj["success"] = false;
                    errorObj["filename"] = filename;
                    errorObj["error"] = "无效的JSON响应";
                    results.append(errorObj);
                }
            } else {
                QString errorMsg = reply->errorString();
                emit fileResult(i, filename, false, "", 0.0, errorMsg, QStringList());
                
                QJsonObject errorObj;
                errorObj["success"] = false;
                errorObj["filename"] = filename;
                errorObj["error"] = errorMsg;
                results.append(errorObj);
            }
        } else {
            emit fileResult(i, filename, false, "", 0.0, "请求超时", QStringList());
            
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["filename"] = filename;
            errorObj["error"] = "请求超时";
            results.append(errorObj);
            reply->abort();
        }
        
        reply->deleteLater();
        multiPart->deleteLater();
        file.close();
        
        // 请求延迟
        if (delayMs > 0 && i < filePaths.size() - 1) {
            QThread::msleep(delayMs);
        }
    }
    
    emit finished(results);
}