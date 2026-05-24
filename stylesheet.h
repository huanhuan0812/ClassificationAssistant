// 在您的main.cpp或程序启动时加载此样式表
// 示例: qApp->setStyleSheet(Material3StyleSheet::getStyle());
#pragma once
#include <QString>

class Material3StyleSheet {
public:
    static QString getStyle() {
        return QString(R"(
            /* 全局默认字体和背景 */
            * {
                font-family: "Segoe UI", "Microsoft YaHei", "PingFang SC", "Helvetica Neue", Helvetica, Arial, sans-serif;
            }
            
            QMainWindow {
                background-color: #f8fafd;
            }
            
            /* 主窗口/对话框背景 */
            QWidget {
                background-color: #f8fafd;
                color: #1e293b;
            }
            
            /* GroupBox 风格 */
            QGroupBox {
                font-weight: 500;
                border: 1px solid #e2e8f0;
                border-radius: 12px;
                margin-top: 12px;
                padding-top: 8px;
                background-color: #ffffff;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 16px;
                padding: 0 8px;
                color: #0f172a;
                font-size: 14px;
            }
            
            /* 标准按钮 - Material 填充按钮 */
            QPushButton {
                background-color: #eef2ff;
                color: #4f46e5;
                border: none;
                border-radius: 20px;
                padding: 8px 16px;
                font-weight: 500;
                font-size: 13px;
            }
            QPushButton:hover {
                background-color: #e0e7ff;
            }
            QPushButton:pressed {
                background-color: #c7d2fe;
            }
            QPushButton:disabled {
                background-color: #f1f5f9;
                color: #94a3b8;
            }
            
            /* 主要操作按钮 (绿色/蓝色强调) */
            QPushButton#classifyBtn, QPushButton[majorButton="true"] {
                background-color: #4f46e5;
                color: white;
                font-weight: 600;
                font-size: 14px;
                padding: 10px 20px;
                border-radius: 28px;
            }
            QPushButton#classifyBtn:hover, QPushButton[majorButton="true"]:hover {
                background-color: #6366f1;
            }
            QPushButton#classifyBtn:pressed, QPushButton[majorButton="true"]:pressed {
                background-color: #4338ca;
            }
            
            /* 导出按钮 - 次要填充样式 */
            QPushButton#exportBtn {
                background-color: #ffffff;
                border: 1px solid #cbd5e1;
                color: #334155;
                border-radius: 24px;
                font-weight: 500;
            }
            QPushButton#exportBtn:hover {
                background-color: #f8fafc;
                border-color: #94a3b8;
            }
            
            /* 测试连接按钮 - 警告色调 */
            QPushButton#testConnBtn {
                background-color: #fff7ed;
                color: #ea580c;
            }
            QPushButton#testConnBtn:hover {
                background-color: #ffedd5;
            }
            
            /* 扁平按钮 (用于设置栏的小按钮) */
            QPushButton[flat="true"] {
                background-color: transparent;
                color: #64748b;
                padding: 6px 12px;
            }
            QPushButton[flat="true"]:hover {
                background-color: #f1f5f9;
                color: #0f172a;
            }
            
            /* 输入框 LineEdit */
            QLineEdit {
                background-color: #ffffff;
                border: 1px solid #e2e8f0;
                border-radius: 12px;
                padding: 8px 12px;
                font-size: 13px;
                selection-background-color: #c7d2fe;
            }
            QLineEdit:focus {
                border: 2px solid #818cf8;
                padding: 7px 11px;
            }
            
            /* 微调框 SpinBox */
            QSpinBox {
                background-color: #ffffff;
                border: 1px solid #e2e8f0;
                border-radius: 12px;
                padding: 6px 8px;
                min-height: 22px;
            }
            QSpinBox:focus {
                border: 2px solid #818cf8;
            }
            
            /* 复选框 */
            QCheckBox {
                spacing: 8px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
                border-radius: 5px;
                border: 2px solid #cbd5e1;
                background-color: white;
            }
            QCheckBox::indicator:checked {
                background-color: #4f46e5;
                border-color: #4f46e5;
                image: url(:/icons/check.png); /* 如果没有资源图片可忽略，但会保留背景 */
            }
            QCheckBox::indicator:hover {
                border-color: #818cf8;
            }
            
            /* 树形控件 QTreeWidget (表格样式) */
            QTreeWidget {
                background-color: #ffffff;
                border: 1px solid #eef2ff;
                border-radius: 16px;
                outline: 0;
                alternate-background-color: #fcfcfd;
                selection-background-color: #eef2ff;
                selection-color: #1e293b;
                font-size: 13px;
            }
            QTreeWidget::item {
                padding: 10px 8px;
                border-bottom: 1px solid #f1f5f9;
            }
            QTreeWidget::item:hover {
                background-color: #f8fafc;
            }
            QHeaderView::section {
                background-color: #f8fafc;
                color: #334155;
                padding: 10px;
                border: none;
                border-bottom: 1px solid #e2e8f0;
                font-weight: 600;
                font-size: 13px;
            }
            
            /* 标签页 QTabWidget */
            QTabWidget::pane {
                border: 1px solid #e2e8f0;
                border-radius: 16px;
                background-color: white;
                top: -1px;
            }
            QTabBar::tab {
                background-color: #f1f5f9;
                color: #475569;
                border-radius: 24px;
                padding: 8px 20px;
                margin-right: 8px;
                font-weight: 500;
            }
            QTabBar::tab:selected {
                background-color: #eef2ff;
                color: #4f46e5;
            }
            QTabBar::tab:hover:!selected {
                background-color: #ffffff;
            }
            
            /* 进度条 QProgressBar */
            QProgressBar {
                border: none;
                background-color: #e2e8f0;
                border-radius: 12px;
                height: 8px;
                text-align: center;
                color: transparent;
            }
            QProgressBar::chunk {
                background-color: #4f46e5;
                border-radius: 12px;
            }
            
            /* 滚动条美化 */
            QScrollBar:vertical {
                background: #f1f5f9;
                width: 10px;
                border-radius: 5px;
            }
            QScrollBar::handle:vertical {
                background: #cbd5e1;
                border-radius: 5px;
                min-height: 20px;
            }
            QScrollBar::handle:vertical:hover {
                background: #94a3b8;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            
            /* 状态栏 */
            QStatusBar {
                background-color: #f8fafd;
                color: #475569;
                font-size: 12px;
                padding: 4px 8px;
            }
            
            /* 菜单栏 */
            QMenuBar {
                background-color: #f8fafd;
                color: #1e293b;
                border-bottom: 1px solid #eef2ff;
            }
            QMenuBar::item {
                background-color: transparent;
                padding: 6px 12px;
                border-radius: 8px;
            }
            QMenuBar::item:selected {
                background-color: #eef2ff;
                color: #4f46e5;
            }
            QMenu {
                background-color: #ffffff;
                border: 1px solid #e2e8f0;
                border-radius: 16px;
                padding: 8px;
            }
            QMenu::item {
                padding: 8px 32px 8px 20px;
                border-radius: 12px;
            }
            QMenu::item:selected {
                background-color: #eef2ff;
                color: #4f46e5;
            }
            
            /* 提示对话框 QMessageBox 自定义 */
            QMessageBox {
                background-color: #ffffff;
            }
            QMessageBox QPushButton {
                min-width: 80px;
                padding: 6px 12px;
            }
            
            /* 进度对话框 */
            QProgressDialog {
                background-color: rgba(255,255,255,0.95);
                border: 1px solid #e2e8f0;
                border-radius: 24px;
            }
            
            /* 组合框 QComboBox */
            QComboBox {
                background-color: white;
                border: 1px solid #e2e8f0;
                border-radius: 12px;
                padding: 6px 12px;
            }
            QComboBox::drop-down {
                border: none;
                width: 20px;
            }
            QComboBox::down-arrow {
                image: none;
                border-left: 4px solid transparent;
                border-right: 4px solid transparent;
                border-top: 5px solid #64748b;
                margin-right: 8px;
            }
            QComboBox QAbstractItemView {
                border: 1px solid #e2e8f0;
                border-radius: 12px;
                selection-background-color: #eef2ff;
                selection-color: #4f46e5;
            }
            
            /* Label 标签 */
            QLabel {
                color: #334155;
            }
            QLabel#apiStatusLabel[status="warning"] {
                color: #f97316;
            }
            QLabel#apiStatusLabel[status="ok"] {
                color: #10b981;
            }
        )");
    }
};