// settingsdialog.cpp
#include "settingsdialog.h"
#include "settings.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QProgressDialog>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    loadSettings();
    setWindowTitle("设置");
    setMinimumWidth(500);
    setModal(true);
}

void SettingsDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    
    // ========== 通用设置页 ==========
    QWidget* generalTab = new QWidget();
    QVBoxLayout* generalLayout = new QVBoxLayout(generalTab);
    
    // API设置组
    QGroupBox* apiGroup = new QGroupBox("API设置");
    QFormLayout* apiForm = new QFormLayout(apiGroup);
    
    m_apiUrlEdit = new QLineEdit();
    m_apiUrlEdit->setPlaceholderText("http://localhost:5000");
    apiForm->addRow("API地址:", m_apiUrlEdit);
    
    QHBoxLayout* testLayout = new QHBoxLayout();
    QPushButton* testBtn = new QPushButton("测试连接");
    connect(testBtn, &QPushButton::clicked, this, &SettingsDialog::onTestConnection);
    testLayout->addWidget(testBtn);
    testLayout->addStretch();
    apiForm->addRow("", testLayout);
    
    generalLayout->addWidget(apiGroup);
    
    // 处理设置组
    QGroupBox* processGroup = new QGroupBox("处理设置");
    QFormLayout* processForm = new QFormLayout(processGroup);
    
    m_timeoutSpin = new QSpinBox();
    m_timeoutSpin->setRange(5, 300);
    m_timeoutSpin->setSuffix(" 秒");
    processForm->addRow("请求超时:", m_timeoutSpin);
    
    m_delaySpin = new QSpinBox();
    m_delaySpin->setRange(0, 5000);
    m_delaySpin->setSuffix(" 毫秒");
    m_delaySpin->setToolTip("每个请求之间的延迟，避免请求过快");
    processForm->addRow("请求延迟:", m_delaySpin);
    
    m_concurrentSpin = new QSpinBox();
    m_concurrentSpin->setRange(1, 10);
    m_concurrentSpin->setSuffix(" 个");
    m_concurrentSpin->setToolTip("同时处理的最大文件数");
    processForm->addRow("最大并发:", m_concurrentSpin);
    
    generalLayout->addWidget(processGroup);
    generalLayout->addStretch();
    
    // ========== 界面设置页 ==========
    QWidget* interfaceTab = new QWidget();
    QVBoxLayout* interfaceLayout = new QVBoxLayout(interfaceTab);
    
    QGroupBox* displayGroup = new QGroupBox("显示设置");
    QFormLayout* displayForm = new QFormLayout(displayGroup);
    
    m_autoClearCheck = new QCheckBox("开始新分类前自动清除上次结果");
    displayForm->addRow("", m_autoClearCheck);
    
    m_keywordTooltipCheck = new QCheckBox("在鼠标悬停时显示关键词");
    displayForm->addRow("", m_keywordTooltipCheck);
    
    interfaceLayout->addWidget(displayGroup);
    
    // 导出设置组
    QGroupBox* exportGroup = new QGroupBox("导出设置");
    QFormLayout* exportForm = new QFormLayout(exportGroup);
    
    m_exportFormatCombo = new QComboBox();
    m_exportFormatCombo->addItem("CSV格式", "csv");
    m_exportFormatCombo->addItem("JSON格式", "json");
    exportForm->addRow("默认导出格式:", m_exportFormatCombo);
    
    interfaceLayout->addWidget(exportGroup);
    interfaceLayout->addStretch();
    
    // ========== 关于页 ==========
    QWidget* aboutTab = new QWidget();
    QVBoxLayout* aboutLayout = new QVBoxLayout(aboutTab);
    
    QGroupBox* aboutGroup = new QGroupBox("关于");
    QVBoxLayout* aboutGroupLayout = new QVBoxLayout(aboutGroup);
    
    m_versionLabel = new QLabel();
    m_versionLabel->setText("<h3>PPTX课件分类器 v1.0</h3>");
    m_versionLabel->setAlignment(Qt::AlignCenter);
    aboutGroupLayout->addWidget(m_versionLabel);
    
    QLabel* infoLabel = new QLabel(
        "基于深度学习模型的PPTX文件学科分类工具\n\n"
        "功能特点：\n"
        "• 支持批量PPTX/PPT文件分类\n"
        "• 自动提取并显示关键词\n"
        "• 支持导出CSV/JSON格式结果\n"
        "• 支持自定义API服务器地址\n"
        "• 显示置信度和处理进度\n"
        "• 支持多线程并发处理"
    );
    infoLabel->setWordWrap(true);
    aboutGroupLayout->addWidget(infoLabel);
    
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("color: gray;");
    m_statusLabel->setText("配置文件位置: " + 
        QSettings("YourCompany", "PPTXClassifier").fileName());
    m_statusLabel->setWordWrap(true);
    aboutGroupLayout->addWidget(m_statusLabel);
    
    aboutLayout->addWidget(aboutGroup);
    
    // 添加标签页
    m_tabWidget->addTab(generalTab, "通用");
    m_tabWidget->addTab(interfaceTab, "界面");
    m_tabWidget->addTab(aboutTab, "关于");
    
    mainLayout->addWidget(m_tabWidget);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_resetBtn = new QPushButton("恢复默认");
    m_saveBtn = new QPushButton("保存");
    m_cancelBtn = new QPushButton("取消");
    
    connect(m_resetBtn, &QPushButton::clicked, this, &SettingsDialog::onResetToDefaults);
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancel);
    
    buttonLayout->addWidget(m_resetBtn);
    buttonLayout->addWidget(m_saveBtn);
    buttonLayout->addWidget(m_cancelBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsDialog::loadSettings() {
    Settings& s = Settings::instance();
    m_apiUrlEdit->setText(s.getApiUrl());
    m_timeoutSpin->setValue(s.getTimeoutSeconds());
    m_delaySpin->setValue(s.getRequestDelayMs());
    m_concurrentSpin->setValue(s.getMaxConcurrentRequests());
    m_autoClearCheck->setChecked(s.getAutoClearResults());
    m_keywordTooltipCheck->setChecked(s.getShowKeywordsTooltip());
    
    QString format = s.getDefaultExportFormat();
    int index = m_exportFormatCombo->findData(format);
    if (index >= 0) m_exportFormatCombo->setCurrentIndex(index);
}

void SettingsDialog::saveSettings() {
    Settings& s = Settings::instance();
    s.setApiUrl(m_apiUrlEdit->text());
    s.setTimeoutSeconds(m_timeoutSpin->value());
    s.setRequestDelayMs(m_delaySpin->value());
    s.setMaxConcurrentRequests(m_concurrentSpin->value());
    s.setAutoClearResults(m_autoClearCheck->isChecked());
    s.setShowKeywordsTooltip(m_keywordTooltipCheck->isChecked());
    s.setDefaultExportFormat(m_exportFormatCombo->currentData().toString());
    s.save();
}

void SettingsDialog::onSave() {
    saveSettings();
    accept();
}

void SettingsDialog::onCancel() {
    reject();
}

void SettingsDialog::onResetToDefaults() {
    Settings::instance().resetToDefaults();
    loadSettings();
    QMessageBox::information(this, "提示", "已恢复所有默认设置");
}

void SettingsDialog::onTestConnection() {
    QString apiUrl = m_apiUrlEdit->text();
    if (apiUrl.isEmpty()) {
        QMessageBox::warning(this, "警告", "请设置API地址");
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
                QMessageBox::information(this, "连接成功", 
                    QString("服务状态: %1\n\nAPI地址: %2")
                    .arg(status)
                    .arg(m_apiUrlEdit->text()));
            } else {
                QMessageBox::warning(this, "响应无效", "服务器返回了无效的响应格式");
            }
        } else {
            QMessageBox::critical(this, "连接失败", 
                QString("错误: %1\n\n请确保API服务正在运行")
                .arg(reply->errorString()));
        }
        reply->deleteLater();
        manager->deleteLater();
        progress->deleteLater();
    });
}