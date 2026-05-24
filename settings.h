// settings.h
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QSettings>

class Settings {
public:
    static Settings& instance();
    
    // API设置
    QString getApiUrl() const;
    void setApiUrl(const QString& url);
    
    // 超时设置
    int getTimeoutSeconds() const;
    void setTimeoutSeconds(int seconds);
    
    // 请求延迟设置
    int getRequestDelayMs() const;
    void setRequestDelayMs(int ms);
    
    // 线程设置
    int getMaxConcurrentRequests() const;
    void setMaxConcurrentRequests(int count);
    
    // 界面设置
    bool getAutoClearResults() const;
    void setAutoClearResults(bool enabled);
    
    bool getShowKeywordsTooltip() const;
    void setShowKeywordsTooltip(bool enabled);
    
    // 导出设置
    QString getDefaultExportFormat() const;
    void setDefaultExportFormat(const QString& format);
    
    // 保存所有设置
    void save();
    
    // 重置为默认值
    void resetToDefaults();

private:
    Settings();
    ~Settings();
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    
    QSettings* m_settings;
};

#endif // SETTINGS_H