// ThemeUtils.cpp
#include "ThemeUtils.h"
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateEdit>
#include <QTableView>
#include <QPushButton>
#include <QScrollArea>

namespace ThemeUtils {
    void setWidgetDarkMode(QWidget* widget, bool darkMode) {
        if (!widget) return;

        QColor textColor = darkMode ? Qt::white : Qt::black;

        if (darkMode) {
            // Dark modern theme with red accent
            widget->setStyleSheet(R"(
                QMainWindow, QDialog {
                    background-color: #0f0f0f;
                    color: #f5f5f5;
                }
                
                QScrollArea {
                    background-color: #1a1a1a;
                    border: none;
                }
                
                QScrollArea > QWidget {
                    background-color: #1a1a1a;
                }
                
                QTableView {
                    background-color: #1a1a1a;
                    color: #f5f5f5;
                    gridline-color: #2a2a2a;
                    border: none;
                    selection-background-color: #dc2626;
                    selection-color: #ffffff;
                    font-size: 10pt;
                }
                
                QTableView::item {
                    padding: 8px;
                    border-bottom: 1px solid #2a2a2a;
                }
                
                QTableView::item:hover {
                    background-color: #252525;
                }

               QTableView::item:selected {
                    border: 2px solid #dc2626;  /* red border around selected row */
                    border-radius: 4px;
                }
                
                QHeaderView::section {
                    background-color: #0f0f0f;
                    color: #f5f5f5;
                    padding: 12px;
                    border: none;
                    border-bottom: 2px solid #dc2626;
                    font-weight: 600;
                    font-size: 10pt;
                    text-transform: uppercase;
                    letter-spacing: 1px;
                }
                
                QPushButton {
                    background-color: #dc2626;
                    color: #ffffff;
                    border: none;
                    border-radius: 8px;
                    padding: 10px 20px;
                    font-weight: 600;
                    font-size: 10pt;
                }
                
                QPushButton:hover {
                    background-color: #ef4444;
                }
                
                QPushButton:pressed {
                    background-color: #b91c1c;
                }
                
                QPushButton:disabled {
                    background-color: #2a2a2a;
                    color: #6b7280;
                }
                
                QLineEdit, QSpinBox, QDateEdit {
                    background-color: #1a1a1a;
                    color: #f5f5f5;
                    border: 2px solid #2a2a2a;
                    border-radius: 8px;
                    padding: 10px;
                    font-size: 10pt;
                }
                
                QLineEdit:focus, QSpinBox:focus, QDateEdit:focus {
                    border: 2px solid #dc2626;
                    background-color: #252525;
                }
                
                QLabel {
                    color: #f5f5f5;
                    font-size: 10pt;
                    background-color: transparent;
                }
                
                QGroupBox {
                    color: #f5f5f5;
                    border: 2px solid #2a2a2a;
                    border-radius: 12px;
                    margin-top: 16px;
                    padding-top: 16px;
                    font-weight: 600;
                    font-size: 11pt;
                }
                
                QGroupBox::title {
                    color: #dc2626;
                    subcontrol-origin: margin;
                    subcontrol-position: top left;
                    padding: 0 12px;
                    background-color: #0f0f0f;
                }
                
                QScrollBar:vertical {
                    background-color: #1a1a1a;
                    width: 10px;
                    border-radius: 5px;
                }
                
                QScrollBar::handle:vertical {
                    background-color: #dc2626;
                    border-radius: 5px;
                    min-height: 30px;
                }
                
                QScrollBar::handle:vertical:hover {
                    background-color: #ef4444;
                }
                
                QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                    height: 0px;
                }
                
                QSlider::groove:horizontal {
                    background-color: #2a2a2a;
                    height: 4px;
                    border-radius: 2px;
                }
                
                QSlider::handle:horizontal {
                    background-color: #dc2626;
                    width: 18px;
                    height: 18px;
                    margin: -7px 0;
                    border-radius: 9px;
                }
                
                QSlider::handle:horizontal:hover {
                    background-color: #ef4444;
                }
            )");
        }
        else {
            // Light modern theme with red accent
            widget->setStyleSheet(R"(
                QMainWindow, QDialog {
                    background-color: #ffffff;
                    color: #0f172a;
                }
                
                QScrollArea {
                    background-color: #f8fafc;
                    border: none;
                }
                
                QScrollArea > QWidget {
                    background-color: #f8fafc;
                }
                
                QTableView {
                    background-color: #ffffff;
                    color: #0f172a;
                    gridline-color: #e2e8f0;
                    border: none;
                    selection-background-color: #dc2626;
                    selection-color: #0f172a;
                    font-size: 10pt;
                }
                
                QTableView::item {
                    padding: 8px;
                    border-bottom: 1px solid #f1f5f9;
                }
                
                QTableView::item:hover {
                    background-color: #f8fafc;
                }
                QTableView::item:selected {
                    border: 2px solid #dc2626;  /* red border around selected row */
                    border-radius: 4px;          /* optional for rounded corners */
                }
                QHeaderView::section {
                    background-color: #ffffff;
                    color: #0f172a;
                    padding: 12px;
                    border: none;
                    border-bottom: 2px solid #dc2626;
                    font-weight: 600;
                    font-size: 10pt;
                    text-transform: uppercase;
                    letter-spacing: 1px;
                }
                
                QPushButton {
                    background-color: #dc2626;
                    color: #ffffff;
                    border: none;
                    border-radius: 8px;
                    padding: 10px 20px;
                    font-weight: 600;
                    font-size: 10pt;
                }
                
                QPushButton:hover {
                    background-color: #ef4444;
                }
                
                QPushButton:pressed {
                    background-color: #b91c1c;
                }
                
                QPushButton:disabled {
                    background-color: #e2e8f0;
                    color: #94a3b8;
                }
                
                QLineEdit, QSpinBox, QDateEdit {
                    background-color: #ffffff;
                    color: #0f172a;
                    border: 2px solid #e2e8f0;
                    border-radius: 8px;
                    padding: 10px;
                    font-size: 10pt;
                }
                
                QLineEdit:focus, QSpinBox:focus, QDateEdit:focus {
                    border: 2px solid #dc2626;
                    background-color: #fef2f2;
                }
                
                QLabel {
                    color: #0f172a;
                    font-size: 10pt;
                    background-color: transparent;
                }
                
                QGroupBox {
                    color: #0f172a;
                    border: 2px solid #e2e8f0;
                    border-radius: 12px;
                    margin-top: 16px;
                    padding-top: 16px;
                    font-weight: 600;
                    font-size: 11pt;
                }
                
                QGroupBox::title {
                    color: #dc2626;
                    subcontrol-origin: margin;
                    subcontrol-position: top left;
                    padding: 0 12px;
                    background-color: #ffffff;
                }
                
                QScrollBar:vertical {
                    background-color: #f1f5f9;
                    width: 10px;
                    border-radius: 5px;
                }
                
                QScrollBar::handle:vertical {
                    background-color: #dc2626;
                    border-radius: 5px;
                    min-height: 30px;
                }
                
                QScrollBar::handle:vertical:hover {
                    background-color: #ef4444;
                }
                
                QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                    height: 0px;
                }
                
                QSlider::groove:horizontal {
                    background-color: #e2e8f0;
                    height: 4px;
                    border-radius: 2px;
                }
                
                QSlider::handle:horizontal {
                    background-color: #dc2626;
                    width: 18px;
                    height: 18px;
                    margin: -7px 0;
                    border-radius: 9px;
                }
                
                QSlider::handle:horizontal:hover {
                    background-color: #ef4444;
                }
            )");
        }

        // Apply palette recursively
        QList<QLabel*> labels = widget->findChildren<QLabel*>();
        for (QLabel* label : labels) {
            QPalette p = label->palette();
            p.setColor(QPalette::WindowText, textColor);
            p.setColor(QPalette::Text, textColor);
            label->setPalette(p);
        }

        QList<QGroupBox*> groups = widget->findChildren<QGroupBox*>();
        for (QGroupBox* group : groups) {
            QPalette p = group->palette();
            p.setColor(QPalette::WindowText, textColor);
            p.setColor(QPalette::Text, textColor);
            group->setPalette(p);
        }

        QList<QLineEdit*> lineEdits = widget->findChildren<QLineEdit*>();
        for (QLineEdit* edit : lineEdits) {
            QPalette p = edit->palette();
            p.setColor(QPalette::Text, textColor);
            p.setColor(QPalette::Base, darkMode ? QColor("#1a1a1a") : Qt::white);
            edit->setPalette(p);
        }

        QList<QSpinBox*> spinBoxes = widget->findChildren<QSpinBox*>();
        for (QSpinBox* spin : spinBoxes) {
            QPalette p = spin->palette();
            p.setColor(QPalette::Text, textColor);
            p.setColor(QPalette::Base, darkMode ? QColor("#1a1a1a") : Qt::white);
            spin->setPalette(p);
        }

        QList<QDateEdit*> dateEdits = widget->findChildren<QDateEdit*>();
        for (QDateEdit* date : dateEdits) {
            QPalette p = date->palette();
            p.setColor(QPalette::Text, textColor);
            p.setColor(QPalette::Base, darkMode ? QColor("#1a1a1a") : Qt::white);
            date->setPalette(p);
        }

        QList<QScrollArea*> scrollAreas = widget->findChildren<QScrollArea*>();
        for (QScrollArea* scroll : scrollAreas) {
            QPalette p = scroll->palette();
            p.setColor(QPalette::Base, darkMode ? QColor("#1a1a1a") : QColor("#f8fafc"));
            p.setColor(QPalette::Window, darkMode ? QColor("#1a1a1a") : QColor("#f8fafc"));
            scroll->setPalette(p);
        }
    }
} // namespace ThemeUtils