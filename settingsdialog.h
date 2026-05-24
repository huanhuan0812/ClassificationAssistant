// settingsdialog.h
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    
private slots:
    void onSave();
    void onCancel();
    void onResetToDefaults();
    void onTestConnection();

private:
    void loadSettings();
    void saveSettings();
    void setupUi();
    
    // 通用设置页
    QLineEdit* m_apiUrlEdit;
    QSpinBox* m_timeoutSpin;
    QSpinBox* m_delaySpin;
    QSpinBox* m_concurrentSpin;
    
    // 界面设置页
    QCheckBox* m_autoClearCheck;
    QCheckBox* m_keywordTooltipCheck;
    QComboBox* m_exportFormatCombo;
    
    // 关于页
    QLabel* m_versionLabel;
    QLabel* m_statusLabel;
    
    QTabWidget* m_tabWidget;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
    QPushButton* m_resetBtn;
};

#endif // SETTINGSDIALOG_H