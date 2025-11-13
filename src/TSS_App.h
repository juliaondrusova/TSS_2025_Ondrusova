#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TSS_App.h"
#include "ThemeUtils.h"

/**
 * @class PhotoManager
 * @brief Main window for managing photo import, export, filtering, and viewing.
 */


class TSS_App : public QMainWindow
{
    Q_OBJECT

public:
    TSS_App(QWidget *parent = nullptr);
    ~TSS_App();
private slots:

    void importPhotos();
    void exportPhotos();
    void toggleDarkMode();
    
protected:
	bool eventFilter(QObject* obj, QEvent* event) override; // For filter input fields

private:
    void updatePageLabel();
    Ui::TSS_AppClass ui;
    bool m_darkMode = true;
    QLabel* m_placeholderLabel = nullptr;
};