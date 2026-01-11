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

        if (darkMode)
        {
            // Modern dark theme inspired by mobile design
            widget->setStyleSheet(R"(
                QMainWindow, QDialog {
                    background-color: #111827;
                    color: #f9fafb;
                }
                
                QScrollArea {
                    background-color: transparent;
                    border: none;
                }
                
                QScrollArea QWidget {
                    background-color: transparent;
                }
                
                QTableView {
                    background-color: #111827;
                    color: #f9fafb;
                    gridline-color: transparent;
                    border: none;
                    selection-background-color: rgba(79, 70, 229, 0.15);
                    selection-color: #f9fafb;
                    font-size: 10pt;
                }
                
                QTableView::item {
                    padding: 12px 8px;
                    border: none;
                    border-radius: 16px;
                    margin: 4px;
                    background-color: #1f2937;
                }
                
                QTableView::item:hover {
                    background-color: #374151;
                }

                QTableView::item:selected {
                    background-color: rgba(79, 70, 229, 0.2);
                    border: 2px solid #4f46e5;
                }
                
                QHeaderView::section {
                    background-color: #111827;
                    color: #9ca3af;
                    padding: 16px 8px;
                    border: none;
                    border-bottom: 1px solid #374151;
                    font-weight: 600;
                    font-size: 9pt;
                    text-transform: uppercase;
                    letter-spacing: 0.5px;
                }
                
                QPushButton,QScrollArea QPushButton {
                    background-color: #4f46e5;
                    color: #ffffff;
                    border: none;
                    border-radius: 16px;
                    padding: 12px 24px;
                    font-weight: 600;
                    font-size: 10pt;
                }
              
                QPushButton:hover,QScrollArea QPushButton:hover {
                    background-color: #6366f1;
                }
                
                QPushButton:pressed, QScrollArea QPushButton:pressed {
                    background-color: #4338ca;
                }
                
                QPushButton:disabled, QScrollArea QPushButton:disaabled {
                    background-color: #374151;
                    color: #6b7280;
                }
                
                QPushButton:checked, QScrollArea QPushButton:checked {
                    background-color: #3730a3; /* Svieža zelená pre aktívny stav */
                    color: #ffffff;
                    border: 1px solid #ffffff;
                }

                QLineEdit, QSpinBox, QDateEdit, QComboBox {
                    background-color: #1f2937;
                    color: #f9fafb;
                    border: 1px solid #374151;
                    border-radius: 12px;
                    padding: 12px 16px;
                    font-size: 10pt;
                }
                
                QLineEdit:focus, QSpinBox:focus, QDateEdit:focus, QComboBox:focus {
                    border: 2px solid #4f46e5;
                    background-color: #1f2937;
                    padding: 11px 15px;
                }
                
                QLabel {
                    color: #f9fafb;
                    font-size: 10pt;
                    background-color: transparent;
                }
                
                QGroupBox {
                    color: #f9fafb;
                    border: 1px solid #374151;
                    border-radius: 24px;
                    margin-top: 20px;
                    padding: 20px 16px 16px 16px;
                    font-weight: 600;
                    font-size: 11pt;
                    background-color: #1f2937;
                }
                
                QGroupBox::title {
                    color: #9ca3af;
                    subcontrol-origin: margin;
                    subcontrol-position: top left;
                    padding: 0 16px;
                    background-color: #111827;
                    text-transform: uppercase;
                    font-size: 9pt;
                    letter-spacing: 0.5px;
                }
                
                QScrollBar:vertical {
                    background-color: #111827;
                    width: 8px;
                    border-radius: 4px;
                }
                
                QScrollBar::handle:vertical {
                    background-color: #4f46e5;
                    border-radius: 4px;
                    min-height: 30px;
                }
                
                QScrollBar::handle:vertical:hover {
                    background-color: #6366f1;
                }
                
                QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                    height: 0px;
                }
                
                QSlider::groove:horizontal {
                    background-color: #374151;
                    height: 4px;
                    border-radius: 2px;
                }
                
                QSlider::handle:horizontal {
                    background-color: #4f46e5;
                    width: 18px;
                    height: 18px;
                    margin: -7px 0;
                    border-radius: 9px;
                }
                
                QSlider::handle:horizontal:hover {
                    background-color: #6366f1;
                }

                QComboBox::drop-down {
                    border: none;
                    padding-right: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #1f2937;
                    color: #f9fafb;
                    border: 1px solid #374151;
                    border-radius: 12px;
                    padding: 4px;
                    selection-background-color: #4f46e5;
                    selection-color: #ffffff;
                }

                QComboBox QAbstractItemView::item {
                    padding: 8px 12px;
                    border-radius: 8px;
                    margin: 2px;
                }

                QHeaderView {
                    background-color: #111827;
                    border: none;
                }              
             
                QTableView::indicator:checked {
                    background-color: #4f46e5;
                    border: 2px solid #6366f1;
                    border-radius: 6px;
                }

                QTableView::indicator:unchecked {
                    background-color: #1f2937;
                    border: 2px solid #4b5563;
                    border-radius: 6px;
                }

            )");
        }
        else {
            // Modern light theme inspired by mobile design
            widget->setStyleSheet(R"(
                QMainWindow, QDialog {
                    background-color: #f9fafb;
                    color: #111827;
                }
                
                QScrollArea {
                    background-color: transparent;
                    border: none;
                }
                
                QScrollArea QWidget {
                    background-color: transparent;
                }

                QTableView {
                    background-color: #f9fafb;
                    color: #111827;
                    gridline-color: transparent;
                    border: none;
                    selection-background-color: rgba(79, 70, 229, 0.1);
                    selection-color: #111827;
                    font-size: 10pt;
                }
                
                QTableView::item {
                    padding: 12px 8px;
                    border: none;
                    border-radius: 16px;
                    margin: 4px;
                    background-color: #ffffff;
                    border: 1px solid #e5e7eb;
                }
                
                QTableView::item:hover {
                    background-color: #f3f4f6;
                    border-color: #d1d5db;
                }
                
                QTableView::item:selected {
                    background-color: rgba(79, 70, 229, 0.12);
                    border: 2px solid #4f46e5;
                }
                
                QHeaderView::section {
                    background-color: #f9fafb;
                    color: #6b7280;
                    padding: 16px 8px;
                    border: none;
                    border-bottom: 1px solid #e5e7eb;
                    font-weight: 600;
                    font-size: 9pt;
                    text-transform: uppercase;
                    letter-spacing: 0.5px;
                }

                QPushButton,QScrollArea QPushButton {
                    background-color: #4f46e5;
                    color: #ffffff;
                    border: none;
                    border-radius: 16px;
                    padding: 12px 24px;
                    font-weight: 600;
                    font-size: 10pt;
                }
                
                QPushButton:hover,QScrollArea QPushButton:hover {
                    background-color: #6366f1;
                }
                
                QPushButton:pressed, QScrollArea QPushButton:pressed {
                    background-color: #4338ca;
                }
                
                QPushButton:disabled QScrollArea QPushButton:disabled {
                    background-color: #e5e7eb;
                    color: #9ca3af;
                }

                QPushButton:checked, QScrollArea QPushButton:checked {
                    background-color: #3730a3;
                    color: #ffffff;
                    border: 1px solid #ffffff;
                }
                
                QLineEdit, QSpinBox, QDateEdit, QComboBox {
                    background-color: #f3f4f6;
                    color: #111827;
                    border: 1px solid #e5e7eb;
                    border-radius: 12px;
                    padding: 12px 16px;
                    font-size: 10pt;
                }
                
                QLineEdit:focus, QSpinBox:focus, QDateEdit:focus, QComboBox:focus {
                    border: 2px solid #4f46e5;
                    background-color: #ffffff;
                    padding: 11px 15px;
                }
                
                QLabel {
                    color: #111827;
                    font-size: 10pt;
                    background-color: transparent;
                }
                
                QGroupBox {
                    color: #111827;
                    border: 1px solid #e5e7eb;
                    border-radius: 24px;
                    margin-top: 20px;
                    padding: 20px 16px 16px 16px;
                    font-weight: 600;
                    font-size: 11pt;
                    background-color: #ffffff;
                }
                
                QGroupBox::title {
                    color: #6b7280;
                    subcontrol-origin: margin;
                    subcontrol-position: top left;
                    padding: 0 16px;
                    background-color: #f9fafb;
                    text-transform: uppercase;
                    font-size: 9pt;
                    letter-spacing: 0.5px;
                }
                
                QScrollBar:vertical {
                    background-color: #f3f4f6;
                    width: 8px;
                    border-radius: 4px;
                }
                
                QScrollBar::handle:vertical {
                    background-color: #4f46e5;
                    border-radius: 4px;
                    min-height: 30px;
                }
                
                QScrollBar::handle:vertical:hover {
                    background-color: #6366f1;
                }
                
                QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                    height: 0px;
                }
                
                QSlider::groove:horizontal {
                    background-color: #e5e7eb;
                    height: 4px;
                    border-radius: 2px;
                }
                
                QSlider::handle:horizontal {
                    background-color: #4f46e5;
                    width: 18px;
                    height: 18px;
                    margin: -7px 0;
                    border-radius: 9px;
                }
                
                QSlider::handle:horizontal:hover {
                    background-color: #6366f1;
                }

                QComboBox::drop-down {
                    border: none;
                    padding-right: 12px;
                }

                QComboBox QAbstractItemView {
                    background-color: #ffffff;
                    color: #111827;
                    border: 1px solid #e5e7eb;
                    border-radius: 12px;
                    padding: 4px;
                    selection-background-color: #4f46e5;
                    selection-color: #ffffff;
                }

                QComboBox QAbstractItemView::item {
                    padding: 8px 12px;
                    border-radius: 8px;
                    margin: 2px;
                }

                QHeaderView {
                    background-color: #f9fafb;
                    border: none;
                }

                QTableView::indicator:checked {
                    background-color: #4f46e5;
                    border: 2px solid #6366f1;
                    border-radius: 6px;
                }

                QTableView::indicator:unchecked {
                    background-color: #ffffff;
                    border: 2px solid #9ca3af;
                    border-radius: 6px;
                }

            )");
        }
    }
}