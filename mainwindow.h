// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QProgressDialog>
#include <QList>
#include <QLabel>
#include "classificationworker.h"

class QPushButton;
class QTreeWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddFiles();
    void onClearFiles();
    void onRemoveSelected();
    void onClassify();
    void onFileResult(int index, const QString& filename, bool success, 
                      const QString& className, double confidence, 
                      const QString& errorMsg, const QStringList& keywords);
    void onClassificationFinished();
    void onExportResults();
    void onTestConnection();
    void onGetCategories();
    void onClearResults();
    void onShowSettings();
    void onAbout();

private:
    void setupUi();
    void setupConnections();
    void setupMenuBar();
    void updateStatus();
    void updateApiStatusLabel();
    void exportToCSV(const QString& filePath);
    void exportToJSON(const QString& filePath);
    
    // UI组件
    QLabel* m_apiStatusLabel;
    QPushButton* m_testConnBtn;
    QPushButton* m_addFilesBtn;
    QPushButton* m_removeSelectedBtn;
    QPushButton* m_clearFilesBtn;
    QPushButton* m_clearResultsBtn;
    QPushButton* m_classifyBtn;
    QPushButton* m_exportBtn;
    QTreeWidget* m_fileList;
    
    // 处理状态
    bool m_processingInProgress;
    int m_successCount;
    int m_failedCount;
    int m_completedCount;
    int m_totalCount;
    QList<QTreeWidgetItem*> m_fileItems;
    
    // 工作线程
    QThread* m_currentThread;
    ClassificationWorker* m_currentWorker;
    QProgressDialog* m_currentProgress;
};

#endif // MAINWINDOW_H