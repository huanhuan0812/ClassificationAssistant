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
    
    // 批量模式：使用 /predict/batch 接口一次性处理所有文件
    if (useBatchMode && filePaths.size() > 1) {
        processBatch(results, timeoutMs);
    } 
    // 单文件模式：逐个处理每个文件
    else {
        processSingle(results, timeoutMs, delayMs);
    }
    
    emit finished(results);
}

void ClassificationWorker::processSingle(QJsonArray& results, int timeoutMs, int delayMs) {
    QNetworkAccessManager manager;
    
    for (int i = 0; i < filePaths.size(); i++) {
        emit progress(i + 1, filePaths.size());
        
        QString filepath = filePaths[i];
        QString filename = QFileInfo(filepath).fileName();
        
        // 检查文件是否存在
        if (!QFile::exists(filepath)) {
            QString errorMsg = QString("文件不存在: %1").arg(filepath);
            emit fileResult(i, filename, false, "", 0.0, errorMsg, QStringList());
            
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["filename"] = filename;
            errorObj["filepath"] = filepath;
            errorObj["error"] = errorMsg;
            results.append(errorObj);
            
            if (delayMs > 0 && i < filePaths.size() - 1) {
                QThread::msleep(delayMs);
            }
            continue;
        }
        
        // 检查文件扩展名
        if (!filepath.toLower().endsWith(".pptx") && !filepath.toLower().endsWith(".ppt")) {
            QString errorMsg = "不支持的文件格式，请上传PPTX或PPT文件";
            emit fileResult(i, filename, false, "", 0.0, errorMsg, QStringList());
            
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["filename"] = filename;
            errorObj["filepath"] = filepath;
            errorObj["error"] = errorMsg;
            results.append(errorObj);
            
            if (delayMs > 0 && i < filePaths.size() - 1) {
                QThread::msleep(delayMs);
            }
            continue;
        }
        
        // 构建JSON请求体
        QJsonObject requestBody;
        requestBody["filepath"] = filepath;
        
        QJsonDocument doc(requestBody);
        QByteArray postData = doc.toJson();
        
        // 创建请求
        QNetworkRequest request(QUrl(apiUrl + "/predict"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QNetworkReply* reply = manager.post(request, postData);
        
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(timeoutMs);
        
        loop.exec();
        
        if (timer.isActive()) {
            timer.stop();
            processReply(reply, i, filename, filepath, results);
        } else {
            // 超时处理
            QString errorMsg = "请求超时";
            emit fileResult(i, filename, false, "", 0.0, errorMsg, QStringList());
            
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["filename"] = filename;
            errorObj["filepath"] = filepath;
            errorObj["error"] = errorMsg;
            results.append(errorObj);
            reply->abort();
        }
        
        reply->deleteLater();
        
        // 请求延迟
        if (delayMs > 0 && i < filePaths.size() - 1) {
            QThread::msleep(delayMs);
        }
    }
}

void ClassificationWorker::processBatch(QJsonArray& results, int timeoutMs) {
    emit progress(0, 1);
    
    // 构建批量请求的JSON体
    QJsonObject requestBody;
    QJsonArray filepathsArray;
    
    for (const QString& filepath : filePaths) {
        // 检查文件是否存在
        if (!QFile::exists(filepath)) {
            QString errorMsg = QString("文件不存在: %1").arg(filepath);
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["filename"] = QFileInfo(filepath).fileName();
            errorObj["filepath"] = filepath;
            errorObj["error"] = errorMsg;
            results.append(errorObj);
            continue;
        }
        
        // 检查文件扩展名
        if (!filepath.toLower().endsWith(".pptx") && !filepath.toLower().endsWith(".ppt")) {
            QString errorMsg = "不支持的文件格式，请上传PPTX或PPT文件";
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["filename"] = QFileInfo(filepath).fileName();
            errorObj["filepath"] = filepath;
            errorObj["error"] = errorMsg;
            results.append(errorObj);
            continue;
        }
        
        filepathsArray.append(filepath);
    }
    
    // 如果没有有效文件，直接返回
    if (filepathsArray.isEmpty()) {
        QJsonObject errorObj;
        errorObj["success"] = false;
        errorObj["error"] = "没有有效的文件路径";
        results.append(errorObj);
        emit progress(1, 1);
        return;
    }
    
    requestBody["filepaths"] = filepathsArray;
    
    QJsonDocument doc(requestBody);
    QByteArray postData = doc.toJson();
    
    // 创建请求
    QNetworkRequest request(QUrl(apiUrl + "/predict/batch"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.post(request, postData);
    
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    
    loop.exec();
    
    emit progress(1, 1);
    
    if (timer.isActive()) {
        timer.stop();
        
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject responseObj = doc.object();
                bool success = responseObj.value("success").toBool(false);
                
                if (success && responseObj.contains("results")) {
                    QJsonArray batchResults = responseObj["results"].toArray();
                    
                    // 转换批量结果为单文件结果格式
                    for (int i = 0; i < batchResults.size(); i++) {
                        QJsonObject resultObj = batchResults[i].toObject();
                        bool fileSuccess = resultObj.value("success").toBool(false);
                        QString filename = resultObj.value("filename").toString();
                        QString filepath = resultObj.value("filepath").toString();
                        
                        if (fileSuccess) {
                            QString className = resultObj.value("predicted_class").toString();
                            double confidence = resultObj.value("confidence").toDouble(0.0);
                            
                            QStringList keywords;
                            if (resultObj.contains("keywords")) {
                                QJsonArray keywordsArray = resultObj["keywords"].toArray();
                                for (const QJsonValue& kw : keywordsArray) {
                                    keywords.append(kw.toString());
                                }
                            }
                            
                            // 找到原始索引
                            int originalIndex = findFileIndex(filepath);
                            if (originalIndex >= 0) {
                                emit fileResult(originalIndex, filename, true, className, confidence, "", keywords);
                            }
                            
                            QJsonObject successObj;
                            successObj["success"] = true;
                            successObj["filename"] = filename;
                            successObj["filepath"] = filepath;
                            successObj["predicted_class"] = className;
                            successObj["confidence"] = confidence;
                            results.append(successObj);
                        } else {
                            QString errorMsg = resultObj.value("error").toString();
                            
                            int originalIndex = findFileIndex(filepath);
                            if (originalIndex >= 0) {
                                emit fileResult(originalIndex, filename, false, "", 0.0, errorMsg, QStringList());
                            }
                            
                            QJsonObject errorObj;
                            errorObj["success"] = false;
                            errorObj["filename"] = filename;
                            errorObj["filepath"] = filepath;
                            errorObj["error"] = errorMsg;
                            results.append(errorObj);
                        }
                    }
                    
                    // 添加摘要信息到结果中（可选）
                    if (responseObj.contains("summary")) {
                        QJsonObject summaryObj;
                        summaryObj["summary"] = responseObj["summary"];
                        summaryObj["type"] = "batch_summary";
                        results.append(summaryObj);
                    }
                } else {
                    QString errorMsg = responseObj.value("error").toString("批量预测失败");
                    emit error(errorMsg);
                    
                    QJsonObject errorObj;
                    errorObj["success"] = false;
                    errorObj["error"] = errorMsg;
                    results.append(errorObj);
                }
            } else {
                emit error("无效的JSON响应");
                
                QJsonObject errorObj;
                errorObj["success"] = false;
                errorObj["error"] = "无效的JSON响应";
                results.append(errorObj);
            }
        } else {
            QString errorMsg = reply->errorString();
            emit error(errorMsg);
            
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["error"] = errorMsg;
            results.append(errorObj);
        }
    } else {
        emit error("批量请求超时");
        
        QJsonObject errorObj;
        errorObj["success"] = false;
        errorObj["error"] = "批量请求超时";
        results.append(errorObj);
        reply->abort();
    }
    
    reply->deleteLater();
}

void ClassificationWorker::processReply(QNetworkReply* reply, int index, 
                                         const QString& filename, 
                                         const QString& filepath,
                                         QJsonArray& results) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject resultObj = doc.object();
            bool success = resultObj.value("success").toBool(true);
            
            if (success) {
                QString className = resultObj.value("predicted_class").toString();
                double confidence = resultObj.value("confidence").toDouble(0.0);
                QString errorMsg = resultObj.value("error").toString();
                
                QStringList keywords;
                if (resultObj.contains("keywords")) {
                    QJsonArray keywordsArray = resultObj["keywords"].toArray();
                    for (const QJsonValue& kw : keywordsArray) {
                        keywords.append(kw.toString());
                    }
                }
                
                emit fileResult(index, filename, true, className, confidence, errorMsg, keywords);
                
                QJsonObject successObj;
                successObj["success"] = true;
                successObj["filename"] = filename;
                successObj["filepath"] = filepath;
                successObj["predicted_class"] = className;
                successObj["confidence"] = confidence;
                results.append(successObj);
            } else {
                QString errorMsg = resultObj.value("error").toString("预测失败");
                emit fileResult(index, filename, false, "", 0.0, errorMsg, QStringList());
                
                QJsonObject errorObj;
                errorObj["success"] = false;
                errorObj["filename"] = filename;
                errorObj["filepath"] = filepath;
                errorObj["error"] = errorMsg;
                results.append(errorObj);
            }
        } else {
            emit fileResult(index, filename, false, "", 0.0, "无效的JSON响应", QStringList());
            
            QJsonObject errorObj;
            errorObj["success"] = false;
            errorObj["filename"] = filename;
            errorObj["filepath"] = filepath;
            errorObj["error"] = "无效的JSON响应";
            results.append(errorObj);
        }
    } else {
        QString errorMsg = reply->errorString();
        emit fileResult(index, filename, false, "", 0.0, errorMsg, QStringList());
        
        QJsonObject errorObj;
        errorObj["success"] = false;
        errorObj["filename"] = filename;
        errorObj["filepath"] = filepath;
        errorObj["error"] = errorMsg;
        results.append(errorObj);
    }
}

int ClassificationWorker::findFileIndex(const QString& filepath) const {
    for (int i = 0; i < filePaths.size(); i++) {
        if (filePaths[i] == filepath) {
            return i;
        }
    }
    return -1;
}