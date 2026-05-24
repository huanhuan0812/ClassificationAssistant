// settings.cpp
#include "settings.h"

Settings& Settings::instance() {
    static Settings instance;
    return instance;
}

Settings::Settings() {
    m_settings = new QSettings("YourCompany", "PPTXClassifier");
}

Settings::~Settings() {
    delete m_settings;
}

QString Settings::getApiUrl() const {
    return m_settings->value("apiUrl", "http://localhost:5000").toString();
}

void Settings::setApiUrl(const QString& url) {
    m_settings->setValue("apiUrl", url);
}

int Settings::getTimeoutSeconds() const {
    return m_settings->value("timeout", 30).toInt();
}

void Settings::setTimeoutSeconds(int seconds) {
    m_settings->setValue("timeout", seconds);
}

int Settings::getRequestDelayMs() const {
    return m_settings->value("requestDelay", 100).toInt();
}

void Settings::setRequestDelayMs(int ms) {
    m_settings->setValue("requestDelay", ms);
}

int Settings::getMaxConcurrentRequests() const {
    return m_settings->value("maxConcurrent", 5).toInt();
}

void Settings::setMaxConcurrentRequests(int count) {
    m_settings->setValue("maxConcurrent", count);
}

bool Settings::getAutoClearResults() const {
    return m_settings->value("autoClearResults", false).toBool();
}

void Settings::setAutoClearResults(bool enabled) {
    m_settings->setValue("autoClearResults", enabled);
}

bool Settings::getShowKeywordsTooltip() const {
    return m_settings->value("showKeywordsTooltip", true).toBool();
}

void Settings::setShowKeywordsTooltip(bool enabled) {
    m_settings->setValue("showKeywordsTooltip", enabled);
}

QString Settings::getDefaultExportFormat() const {
    return m_settings->value("defaultExportFormat", "csv").toString();
}

void Settings::setDefaultExportFormat(const QString& format) {
    m_settings->setValue("defaultExportFormat", format);
}

void Settings::save() {
    m_settings->sync();
}

void Settings::resetToDefaults() {
    m_settings->clear();
    setApiUrl("http://localhost:5000");
    setTimeoutSeconds(30);
    setRequestDelayMs(100);
    setMaxConcurrentRequests(5);
    setAutoClearResults(false);
    setShowKeywordsTooltip(true);
    setDefaultExportFormat("csv");
}