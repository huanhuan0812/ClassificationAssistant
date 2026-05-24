// mainwindow.cpp
#include "mainwindow.h"
#include "settings.h"
#include "settingsdialog.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QTimer>
#include <QThread>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_processingInProgress(false)
    , m_currentThread(nullptr)
    , m_currentWorker(nullptr)
    , m_currentProgress(nullptr) {
    setupUi();
    setupMenuBar();
    setupConnections();
    updateStatus();
    
    // 从设置加载API地址到状态栏显示
    Settings& s = Settings::instance();
    statusBar()->showMessage(QString("就绪 | API: %1 | 请添加PPTX文件开始分类").arg(s.getApiUrl()));
}

MainWindow::~MainWindow() {
    if (m_currentThread) {
        m_currentThread->quit();
        m_currentThread->wait();
    }
}

void MainWindow::setupUi() {
    setWindowTitle("PPTX课件分类器 v1.0");
    setMinimumSize(1000, 700);
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    
    // 信息栏 - 显示当前API地址
    QHBoxLayout* infoLayout = new QHBoxLayout();
    QLabel* infoIcon = new QLabel("🔗");
    infoIcon->setStyleSheet("font-size: 14px;");
    infoLayout->addWidget(infoIcon);
    
    m_apiStatusLabel = new QLabel();
    m_apiStatusLabel->setStyleSheet("color: #666; font-size: 12px;");
    infoLayout->addWidget(m_apiStatusLabel);
    
    infoLayout->addStretch();
    
    QPushButton* settingsBtn = new QPushButton("⚙ 设置");
    settingsBtn->setFlat(true);
    settingsBtn->setCursor(Qt::PointingHandCursor);
    settingsBtn->setStyleSheet("QPushButton { color: #2196F3; } QPushButton:hover { text-decoration: underline; }");
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onShowSettings);
    infoLayout->addWidget(settingsBtn);
    
    mainLayout->addLayout(infoLayout);
    
    // 文件列表区域
    QGroupBox* fileGroup = new QGroupBox("文件列表");
    fileGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    
    // 工具栏
    QHBoxLayout* toolbar = new QHBoxLayout();
    m_addFilesBtn = new QPushButton("📁 添加文件");
    m_removeSelectedBtn = new QPushButton("🗑 移除选中");
    m_clearFilesBtn = new QPushButton("🗑 清空列表");
    m_clearResultsBtn = new QPushButton("🔄 清除结果");
    
    toolbar->addWidget(m_addFilesBtn);
    toolbar->addWidget(m_removeSelectedBtn);
    toolbar->addWidget(m_clearFilesBtn);
    toolbar->addWidget(m_clearResultsBtn);
    toolbar->addStretch();
    
    QLabel* tipLabel = new QLabel("💡 提示: 鼠标悬停在分类结果上可查看关键词");
    tipLabel->setStyleSheet("color: gray; font-size: 11px;");
    toolbar->addWidget(tipLabel);
    
    fileLayout->addLayout(toolbar);
    
    // 文件树形列表
    m_fileList = new QTreeWidget();
    m_fileList->setHeaderLabels({"文件名", "分类结果", "置信度/错误信息"});
    m_fileList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fileList->setAlternatingRowColors(true);
    m_fileList->setIndentation(0);
    m_fileList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_fileList->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_fileList->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_fileList->setSortingEnabled(true);
    m_fileList->sortByColumn(0, Qt::AscendingOrder);
    
    fileLayout->addWidget(m_fileList);
    mainLayout->addWidget(fileGroup);
    
    // 快捷操作按钮区域
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->addStretch();
    
    // 连接测试按钮
    m_testConnBtn = new QPushButton("🔌 测试连接");
    m_testConnBtn->setMinimumHeight(35);
    m_testConnBtn->setMinimumWidth(120);
    m_testConnBtn->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; font-weight: bold; border-radius: 5px; padding: 6px; }"
        "QPushButton:hover { background-color: #FB8C00; }"
    );
    
    m_classifyBtn = new QPushButton("▶ 开始分类");
    m_classifyBtn->setMinimumHeight(40);
    m_classifyBtn->setMinimumWidth(150);
    m_classifyBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-weight: bold; font-size: 14px; border-radius: 5px; padding: 8px; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3d8b40; }"
        "QPushButton:disabled { background-color: #cccccc; }"
    );
    
    m_exportBtn = new QPushButton("💾 导出结果");
    m_exportBtn->setMinimumHeight(40);
    m_exportBtn->setMinimumWidth(150);
    m_exportBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; font-weight: bold; font-size: 14px; border-radius: 5px; padding: 8px; }"
        "QPushButton:hover { background-color: #0b7dda; }"
    );
    
    actionLayout->addWidget(m_testConnBtn);
    actionLayout->addSpacing(20);
    actionLayout->addWidget(m_classifyBtn);
    actionLayout->addWidget(m_exportBtn);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);
    
    // 状态栏
    statusBar()->setStyleSheet("QStatusBar { color: #666; padding: 3px; }");
    
    // 更新API状态显示
    updateApiStatusLabel();
}

void MainWindow::updateApiStatusLabel() {
    Settings& s = Settings::instance();
    QString apiUrl = s.getApiUrl();
    if (apiUrl.isEmpty()) {
        m_apiStatusLabel->setText("⚠ 未设置API地址，请点击右侧设置按钮配置");
        m_apiStatusLabel->setStyleSheet("color: #ff9800; font-size: 12px;");
    } else {
        m_apiStatusLabel->setText(QString("当前API地址: %1").arg(apiUrl));
        m_apiStatusLabel->setStyleSheet("color: #4CAF50; font-size: 12px;");
    }
}

void MainWindow::setupMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu("文件");
    QAction* addAction = fileMenu->addAction("添加文件");
    addAction->setShortcut(QKeySequence("Ctrl+O"));
    connect(addAction, &QAction::triggered, this, &MainWindow::onAddFiles);
    
    fileMenu->addSeparator();
    QAction* exportAction = fileMenu->addAction("导出结果");
    exportAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportResults);
    
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction("退出");
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    QMenu* toolsMenu = menuBar()->addMenu("工具");
    QAction* settingsAction = toolsMenu->addAction("设置");
    settingsAction->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onShowSettings);
    
    toolsMenu->addSeparator();
    QAction* testAction = toolsMenu->addAction("测试API连接");
    testAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(testAction, &QAction::triggered, this, &MainWindow::onTestConnection);
    
    QAction* categoriesAction = toolsMenu->addAction("查看支持类别");
    connect(categoriesAction, &QAction::triggered, this, &MainWindow::onGetCategories);
    
    toolsMenu->addSeparator();
    QAction* clearAction = toolsMenu->addAction("清除所有结果");
    connect(clearAction, &QAction::triggered, this, &MainWindow::onClearResults);
    
    QMenu* helpMenu = menuBar()->addMenu("帮助");
    QAction* aboutAction = helpMenu->addAction("关于");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupConnections() {
    connect(m_addFilesBtn, &QPushButton::clicked, this, &MainWindow::onAddFiles);
    connect(m_removeSelectedBtn, &QPushButton::clicked, this, &MainWindow::onRemoveSelected);
    connect(m_clearFilesBtn, &QPushButton::clicked, this, &MainWindow::onClearFiles);
    connect(m_clearResultsBtn, &QPushButton::clicked, this, &MainWindow::onClearResults);
    connect(m_classifyBtn, &QPushButton::clicked, this, &MainWindow::onClassify);
    connect(m_exportBtn, &QPushButton::clicked, this, &MainWindow::onExportResults);
    connect(m_testConnBtn, &QPushButton::clicked, this, &MainWindow::onTestConnection);
}

void MainWindow::onAddFiles() {
    if (m_processingInProgress) {
        QMessageBox::warning(this, "提示", "正在处理中，请等待完成后再添加文件");
        return;
    }
    
    QStringList files = QFileDialog::getOpenFileNames(this, 
        "选择PPTX文件", 
        QString(),
        "PowerPoint文件 (*.pptx *.ppt)");
    
    if (!files.isEmpty()) {
        for (const QString& file : files) {
            bool exists = false;
            for (int i = 0; i < m_fileList->topLevelItemCount(); i++) {
                QTreeWidgetItem* item = m_fileList->topLevelItem(i);
                if (item->data(0, Qt::UserRole).toString() == file) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setText(0, QFileInfo(file).fileName());
                item->setText(1, "等待处理");
                item->setText(2, "-");
                item->setData(0, Qt::UserRole, file);
                m_fileList->addTopLevelItem(item);
            }
        }
        updateStatus();
    }
}

void MainWindow::onClearFiles() {
    if (m_processingInProgress) {
        QMessageBox::warning(this, "提示", "正在处理中，请等待完成后再清空列表");
        return;
    }
    
    if (m_fileList->topLevelItemCount() > 0) {
        int ret = QMessageBox::question(this, "确认", "确定要清空所有文件吗？",
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            m_fileList->clear();
            updateStatus();
        }
    }
}

void MainWindow::onRemoveSelected() {
    if (m_processingInProgress) {
        QMessageBox::warning(this, "提示", "正在处理中，请等待完成后再移除文件");
        return;
    }
    
    QList<QTreeWidgetItem*> selected = m_fileList->selectedItems();
    if (!selected.isEmpty()) {
        for (QTreeWidgetItem* item : selected) {
            delete item;
        }
        updateStatus();
    }
}

void MainWindow::onClassify() {
    if (m_processingInProgress) {
        QMessageBox::warning(this, "提示", "正在处理中，请等待当前任务完成");
        return;
    }
    
    if (m_fileList->topLevelItemCount() == 0) {
        QMessageBox::warning(this, "警告", "请先添加文件");
        return;
    }
    
    Settings& settings = Settings::instance();
    QString apiUrl = settings.getApiUrl();
    
    if (apiUrl.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先设置API地址 (工具 -> 设置)");
        return;
    }
    
    // 自动清除上次结果（如果启用）
    if (settings.getAutoClearResults()) {
        onClearResults();
    }
    
    m_processingInProgress = true;
    m_classifyBtn->setEnabled(false);
    m_addFilesBtn->setEnabled(false);
    m_clearFilesBtn->setEnabled(false);
    m_removeSelectedBtn->setEnabled(false);
    m_testConnBtn->setEnabled(false);
    
    // 收集文件路径并重置状态
    QStringList filePaths;
    m_fileItems.clear();
    
    for (int i = 0; i < m_fileList->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = m_fileList->topLevelItem(i);
        filePaths.append(item->data(0, Qt::UserRole).toString());
        item->setText(1, "处理中...");
        item->setText(2, "-");
        item->setForeground(1, QBrush(Qt::gray));
        m_fileItems.append(item);
    }
    
    // 创建进度对话框
    m_currentProgress = new QProgressDialog("正在分类文件...", "取消", 0, filePaths.size(), this);
    m_currentProgress->setWindowModality(Qt::WindowModal);
    m_currentProgress->setMinimumDuration(0);
    m_currentProgress->setAutoClose(false);
    m_currentProgress->setAutoReset(false);
    
    // 创建工作线程
    QThread* thread = new QThread();
    ClassificationWorker* worker = new ClassificationWorker();
    worker->apiUrl = apiUrl;
    worker->filePaths = filePaths;
    worker->moveToThread(thread);
    
    m_successCount = 0;
    m_failedCount = 0;
    m_completedCount = 0;
    m_totalCount = filePaths.size();
    
    // 连接信号
    connect(thread, &QThread::started, worker, &ClassificationWorker::process);
    connect(worker, &ClassificationWorker::fileResult, this, &MainWindow::onFileResult);
    connect(worker, &ClassificationWorker::progress, this, [this](int current, int total) {
        if (m_currentProgress) {
            m_currentProgress->setValue(current);
            m_currentProgress->setLabelText(QString("正在处理: %1 / %2").arg(current).arg(total));
        }
    });
    connect(worker, &ClassificationWorker::finished, this, &MainWindow::onClassificationFinished);
    connect(worker, &ClassificationWorker::error, this, [this](const QString& message) {
        QMessageBox::critical(this, "错误", message);
    });
    connect(m_currentProgress, &QProgressDialog::canceled, this, [this, thread]() {
        if (m_currentProgress) {
            m_currentProgress->setLabelText("正在取消...");
        }
        thread->quit();
    });
    
    m_currentWorker = worker;
    m_currentThread = thread;
    
    thread->start();
}

void MainWindow::onFileResult(int index, const QString& filename, bool success, 
                              const QString& className, double confidence, 
                              const QString& errorMsg, const QStringList& keywords) {
    if (index < m_fileItems.size()) {
        QTreeWidgetItem* item = m_fileItems[index];
        
        if (success) {
            item->setText(1, className);
            item->setText(2, QString::number(confidence * 100, 'f', 1) + "%");
            item->setForeground(1, QBrush(Qt::darkGreen));
            
            Settings& s = Settings::instance();
            if (s.getShowKeywordsTooltip() && !keywords.isEmpty()) {
                QString tooltip = "关键词: " + keywords.join(", ");
                item->setToolTip(1, tooltip);
                item->setToolTip(2, tooltip);
            }
            m_successCount++;
        } else {
            item->setText(1, "错误");
            item->setText(2, errorMsg.left(50));
            item->setForeground(1, QBrush(Qt::red));
            item->setToolTip(1, errorMsg);
            m_failedCount++;
        }
        
        m_completedCount++;
        updateStatus();
        
        if (m_currentProgress) {
            m_currentProgress->setLabelText(QString("已完成: %1 / %2 | 成功: %3 | 失败: %4")
                .arg(m_completedCount)
                .arg(m_totalCount)
                .arg(m_successCount)
                .arg(m_failedCount));
        }
    }
}

void MainWindow::onClassificationFinished() {
    if (m_currentProgress) {
        m_currentProgress->close();
        delete m_currentProgress;
        m_currentProgress = nullptr;
    }
    
    if (m_currentThread) {
        m_currentThread->quit();
        m_currentThread->wait();
        delete m_currentThread;
        m_currentThread = nullptr;
    }
    
    if (m_currentWorker) {
        delete m_currentWorker;
        m_currentWorker = nullptr;
    }
    
    m_processingInProgress = false;
    m_classifyBtn->setEnabled(true);
    m_addFilesBtn->setEnabled(true);
    m_clearFilesBtn->setEnabled(true);
    m_removeSelectedBtn->setEnabled(true);
    m_testConnBtn->setEnabled(true);
    
    m_fileItems.clear();
    
    QMessageBox::information(this, "分类完成", 
        QString("处理完成！\n\n总文件数: %1\n成功: %2 个\n失败: %3 个")
        .arg(m_totalCount)
        .arg(m_successCount)
        .arg(m_failedCount));
}

void MainWindow::onExportResults() {
    if (m_fileList->topLevelItemCount() == 0) {
        QMessageBox::warning(this, "警告", "没有结果可导出");
        return;
    }
    
    bool hasResults = false;
    for (int i = 0; i < m_fileList->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = m_fileList->topLevelItem(i);
        if (item->text(1) != "等待处理" && item->text(1) != "处理中...") {
            hasResults = true;
            break;
        }
    }
    
    if (!hasResults) {
        QMessageBox::warning(this, "警告", "没有可导出的结果，请先进行分类");
        return;
    }
    
    Settings& s = Settings::instance();
    QString defaultFormat = s.getDefaultExportFormat();
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    
    QString filter;
    if (defaultFormat == "csv") {
        filter = "CSV文件 (*.csv)";
    } else {
        filter = "JSON文件 (*.json)";
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, "导出结果", 
        QString("classification_results_%1").arg(timestamp),
        filter + ";;CSV文件 (*.csv);;JSON文件 (*.json)");
        
    if (filePath.isEmpty()) return;
    
    if (filePath.endsWith(".csv")) {
        exportToCSV(filePath);
    } else if (filePath.endsWith(".json")) {
        exportToJSON(filePath);
    }
}

void MainWindow::onTestConnection() {
    Settings& s = Settings::instance();
    QString apiUrl = s.getApiUrl();
    
    if (apiUrl.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先设置API地址 (工具 -> 设置)");
        return;
    }
    
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl(apiUrl + "/health"));
    
    QProgressDialog* progress = new QProgressDialog("正在测试连接...", "取消", 0, 0, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    
    QNetworkReply* reply = manager->get(request);
    QTimer timer;
    timer.setSingleShot(true);
    
    connect(&timer, &QTimer::timeout, [reply, progress]() {
        if (reply && reply->isRunning()) {
            reply->abort();
            progress->close();
            QMessageBox::warning(nullptr, "超时", "连接超时，请检查服务是否运行");
        }
    });
    timer.start(10000);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, progress, manager]() {
        progress->close();
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject obj = doc.object();
                QString status = obj["status"].toString();
                QJsonArray categories = obj["categories"].toArray();
                
                QStringList catList;
                for (const QJsonValue& cat : categories) {
                    catList.append(cat.toString());
                }
                
                QMessageBox::information(this, "连接成功", 
                    QString("服务状态: %1\n\nAPI地址: %2\n支持的类别 (%3):\n%4")
                    .arg(status)
                    .arg(Settings::instance().getApiUrl())
                    .arg(categories.size())
                    .arg(catList.join(", ")));
            } else {
                QMessageBox::warning(this, "响应无效", "服务器返回了无效的响应格式");
            }
        } else {
            QMessageBox::critical(this, "连接失败", 
                QString("错误: %1\n\n请确保API服务正在运行\nAPI地址: %2")
                .arg(reply->errorString())
                .arg(Settings::instance().getApiUrl()));
        }
        reply->deleteLater();
        manager->deleteLater();
        progress->deleteLater();
    });
}

void MainWindow::onGetCategories() {
    Settings& s = Settings::instance();
    QString apiUrl = s.getApiUrl();
    
    if (apiUrl.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先设置API地址 (工具 -> 设置)");
        return;
    }
    
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl(apiUrl + "/categories"));
    
    QProgressDialog* progress = new QProgressDialog("正在获取类别...", "取消", 0, 0, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    
    QNetworkReply* reply = manager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager, progress]() {
        progress->close();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            QJsonArray categories = obj["categories"].toArray();
            
            QStringList catList;
            for (const QJsonValue& cat : categories) {
                catList.append(cat.toString());
            }
            
            QMessageBox::information(this, "支持的类别", 
                QString("共 %1 个类别:\n\n%2")
                .arg(categories.size())
                .arg(catList.join("\n")));
        } else {
            QMessageBox::critical(this, "获取失败", 
                QString("错误: %1").arg(reply->errorString()));
        }
        reply->deleteLater();
        manager->deleteLater();
        progress->deleteLater();
    });
}

void MainWindow::onClearResults() {
    if (m_processingInProgress) {
        QMessageBox::warning(this, "提示", "正在处理中，请等待完成后再清除结果");
        return;
    }
    
    bool hasResults = false;
    for (int i = 0; i < m_fileList->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = m_fileList->topLevelItem(i);
        if (item->text(1) != "等待处理") {
            hasResults = true;
            break;
        }
    }
    
    if (hasResults) {
        int ret = QMessageBox::question(this, "确认", "确定要清除所有分类结果吗？",
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            for (int i = 0; i < m_fileList->topLevelItemCount(); i++) {
                QTreeWidgetItem* item = m_fileList->topLevelItem(i);
                item->setText(1, "等待处理");
                item->setText(2, "-");
                item->setForeground(1, QBrush());
                item->setToolTip(1, "");
                item->setToolTip(2, "");
            }
            updateStatus();
            QMessageBox::information(this, "完成", "已清除所有分类结果");
        }
    } else {
        QMessageBox::information(this, "提示", "没有需要清除的结果");
    }
}

void MainWindow::onShowSettings() {
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        updateApiStatusLabel();
        updateStatus();
        statusBar()->showMessage(QString("设置已更新 | API: %1").arg(Settings::instance().getApiUrl()), 3000);
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "关于 PPTX课件分类器", 
        "<h3>PPTX课件分类器 v1.0</h3>"
        "<p>基于深度学习模型的PPTX文件学科分类工具</p>"
        "<br>"
        "<b>功能特点：</b><br>"
        "• 支持批量PPTX/PPT文件分类<br>"
        "• 自动提取并显示关键词<br>"
        "• 支持导出CSV/JSON格式结果<br>"
        "• 支持自定义API服务器地址<br>"
        "• 显示置信度和处理进度<br>"
        "• 支持多线程并发处理<br>"
        "• 可配置超时、延迟等高级选项<br>"
        "<br>"
        "<b>使用说明：</b><br>"
        "1. 点击'工具 -> 设置'配置API服务器地址<br>"
        "2. 点击'测试连接'验证服务是否可用<br>"
        "3. 点击'添加文件'选择PPTX文件<br>"
        "4. 点击'开始分类'进行批量预测<br>"
        "5. 鼠标悬停在分类结果上查看关键词<br>"
        "6. 点击'导出结果'保存分类结果<br>"
        "<br>"
        "<b>技术支持：</b><br>"
        "需要先启动Python API服务（server-api.py）<br>"
        "<br>"
        "© 2024 YourCompany");
}

void MainWindow::updateStatus() {
    int count = m_fileList->topLevelItemCount();
    int processed = 0;
    int success = 0;
    
    for (int i = 0; i < count; i++) {
        QTreeWidgetItem* item = m_fileList->topLevelItem(i);
        QString status = item->text(1);
        if (status != "等待处理" && status != "处理中...") {
            processed++;
            if (status != "错误") {
                success++;
            }
        }
    }
    
    QString statusMsg = QString("共 %1 个文件 | 已处理: %2 | 成功: %3 | 失败: %4")
        .arg(count)
        .arg(processed)
        .arg(success)
        .arg(processed - success);
    
    if (count > 0 && processed == 0) {
        statusMsg += " | 点击\"开始分类\"进行处理";
    }
    
    statusBar()->showMessage(statusMsg);
    m_clearResultsBtn->setEnabled(processed > 0);
}

void MainWindow::exportToCSV(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("\xEF\xBB\xBF");
        file.write("文件名,分类结果,置信度/错误信息,状态,时间戳\n");
        
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        
        for (int i = 0; i < m_fileList->topLevelItemCount(); i++) {
            QTreeWidgetItem* item = m_fileList->topLevelItem(i);
            QString status = (item->text(1) == "错误") ? "失败" : "成功";
            QString line = QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\"\n")
                .arg(item->text(0).replace("\"", "\"\""))
                .arg(item->text(1).replace("\"", "\"\""))
                .arg(item->text(2).replace("\"", "\"\""))
                .arg(status)
                .arg(timestamp);
            file.write(line.toUtf8());
        }
        file.close();
        QMessageBox::information(this, "成功", QString("已导出到: %1").arg(filePath));
    } else {
        QMessageBox::warning(this, "错误", "无法写入文件");
    }
}

void MainWindow::exportToJSON(const QString& filePath) {
    QJsonArray results;
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    for (int i = 0; i < m_fileList->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = m_fileList->topLevelItem(i);
        QJsonObject obj;
        obj["filename"] = item->text(0);
        obj["predicted_class"] = item->text(1);
        obj["confidence"] = item->text(2);
        obj["status"] = (item->text(1) == "错误") ? "failed" : "success";
        obj["timestamp"] = timestamp;
        results.append(obj);
    }
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(results).toJson(QJsonDocument::Indented));
        file.close();
        QMessageBox::information(this, "成功", QString("已导出到: %1").arg(filePath));
    } else {
        QMessageBox::warning(this, "错误", "无法写入文件");
    }
}