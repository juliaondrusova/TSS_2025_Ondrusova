#include "TSS_App.h"
#include "PhotoTableModel.h"
#include "PhotoMetadata.h"
#include "PhotoDetailDialog.h"
#include "PhotoEditDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QProgressDialog>
#include <QApplication>

/**
 * @brief Initializes the PhotoManager main window and connects UI events.
 * @param parent Optional parent widget.
 */

TSS_App::TSS_App(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Initialize the photo table model
    auto model = new PhotoTableModel(this);
    ui.tableView->setModel(model);
    ui.tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableView->verticalHeader()->hide();
    ui.tableView->verticalHeader()->setDefaultSectionSize(70);

    // Enable sorting
    ui.tableView->setSortingEnabled(true);
    ui.tableView->horizontalHeader()->setSortIndicatorShown(true);
    ui.tableView->horizontalHeader()->setSectionsClickable(true);

    // Set column widths
    ui.tableView->setColumnWidth(PhotoTableModel::Preview, 110);
    ui.tableView->setColumnWidth(PhotoTableModel::Name, 230);
    ui.tableView->setColumnWidth(PhotoTableModel::Tag, 80);
    ui.tableView->setColumnWidth(PhotoTableModel::Rating, 100);
    ui.tableView->setColumnWidth(PhotoTableModel::Comment, 150);
    ui.tableView->setColumnWidth(PhotoTableModel::Size, 80);
    ui.tableView->setColumnWidth(PhotoTableModel::DateTime, 120);
    ui.tableView->setColumnWidth(PhotoTableModel::Actions, 90);

    // Default date filters
    ui.dateFromEdit->setDate(QDate::currentDate().addMonths(-1));
    ui.dateToEdit->setDate(QDate::currentDate());

    // Connect buttons and actions
    connect(ui.btnImport, &QPushButton::clicked, this, &TSS_App::importPhotos);
    connect(ui.btnExport, &QPushButton::clicked, this, &TSS_App::exportPhotos);
    connect(ui.btnToggleDarkMode, &QPushButton::clicked, this, &TSS_App::toggleDarkMode);

    // Double-click on preview column open detail dialog
    connect(ui.tableView, &QTableView::doubleClicked, this, [=](const QModelIndex& index) {
        if (index.column() != PhotoTableModel::Preview) return;
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        int realRow = model->currentPage() * model->pageSize() + index.row();
        Photo photo = model->photoAt(realRow);

        auto dlg = new PhotoDetailDialog(this);
        dlg->setPhoto(photo);
        dlg->exec();
        });

    // Click on Actions column --> open editor dialog
    connect(ui.tableView, &QTableView::clicked, this, [=](const QModelIndex& index) {
        if (index.column() != PhotoTableModel::Actions) return;
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        int realRow = model->currentPage() * model->pageSize() + index.row();
        Photo photo = model->photoAt(realRow);

        auto editor = new PhotoEditorDialog(photo, this);
        editor->exec();
        });

    // Pagination controls
    connect(ui.btnNextPage, &QPushButton::clicked, this, [=]() {
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        model->nextPage();
        updatePageLabel();
        });

    connect(ui.btnPrevPage, &QPushButton::clicked, this, [=]() {
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        model->prevPage();
        updatePageLabel();
        });

    updatePageLabel();
}

TSS_App::~TSS_App()
{}


 
void TSS_App::importPhotos() {
    auto& metaManager = PhotoMetadataManager::instance();
    metaManager.loadFromFile();

    QString dirPath = QFileDialog::getExistingDirectory(this, "Select folder with photos");
    if (dirPath.isEmpty()) return;

    QStringList files;
    QDirIterator it(dirPath, { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif", "*.tiff" },
        QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) files.append(it.next());

    if (files.isEmpty()) {
        QMessageBox::information(this, "No images found", "This folder doesn't contain supported image files.");
        return;
    }

    if (QMessageBox::question(this, "Import Photos",
        QString("Found %1 images. Import them?\n\n")
        .arg(files.size()),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    auto progress = new QProgressDialog("Importing photos...", "Cancel", 0, files.size(), this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowTitle("Import Progress");

    auto model = static_cast<PhotoTableModel*>(ui.tableView->model());

    int count = 0;
    for (const QString& file : files) {
        if (progress->wasCanceled())
            break;

        model->addPhoto(file);
        progress->setValue(++count);
        qApp->processEvents();
    }

    progress->close();
    QMessageBox::information(this, "Done", QString("Imported %1 photos.").arg(count));

    updatePageLabel();
}

/**
 * @brief Exports all loaded photos to a user-selected directory with a progress dialog.
 */
void TSS_App::exportPhotos() {
  }

/**
 * @brief Updates the page label and enables/disables navigation buttons.
 */
void TSS_App::updatePageLabel() {
    auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
    int current = model->currentPage() + 1;
    int total = model->totalPages();

    ui.lblPage->setText(total == 0
        ? "No results"
        : QString("Page %1 / %2").arg(current).arg(total));

    ui.btnPrevPage->setEnabled(model->currentPage() > 0);
    ui.btnNextPage->setEnabled(current < total);
}

/**
 * @brief Toggles between dark and light mode and applies the theme to the main window.
 */
void TSS_App::toggleDarkMode() {
    m_darkMode = !m_darkMode;
}
