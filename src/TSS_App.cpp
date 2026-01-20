#include "TSS_App.h"
#include "PhotoTableModel.h"
#include "PhotoMetadata.h"
#include "PhotoDetailDialog.h"
#include "PhotoEditDialog.h"
#include "PhotoExportDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QProgressDialog>
#include <QApplication>
#include <QSettings>

TSS_App::TSS_App(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowTitle("Photo Manager");
    ThemeUtils::setWidgetDarkMode(this, true);

    // Initialize the photo table model
    auto model = new PhotoTableModel(this);
    ui.tableView->setModel(model);
    ui.tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableView->verticalHeader()->hide();
    ui.tableView->verticalHeader()->setDefaultSectionSize(75);

    // Enable sorting
    ui.tableView->setSortingEnabled(true);
    ui.tableView->horizontalHeader()->setSortIndicatorShown(true);
    ui.tableView->horizontalHeader()->setSectionsClickable(true);
    ui.tableView->sortByColumn(PhotoTableModel::DateTime, Qt::DescendingOrder);

    // Set column widths
    ui.tableView->setColumnWidth(PhotoTableModel::Preview, 60);
    ui.tableView->setColumnWidth(PhotoTableModel::Name, 245);
    ui.tableView->setColumnWidth(PhotoTableModel::Tag, 75);
    ui.tableView->setColumnWidth(PhotoTableModel::Rating, 95);
    ui.tableView->setColumnWidth(PhotoTableModel::Comment, 160);
    ui.tableView->setColumnWidth(PhotoTableModel::Size, 80);
    ui.tableView->setColumnWidth(PhotoTableModel::DateTime, 120);
    ui.tableView->setColumnWidth(PhotoTableModel::Actions, 90);
    ui.tableView->setColumnWidth(PhotoTableModel::Export, 75);

    // Default date filters
    ui.dateFromEdit->setDate(QDate::currentDate().addMonths(-1));
    ui.dateToEdit->setDate(QDate::currentDate());

    // Install event filters for filter inputs
    ui.tagFilterEdit->installEventFilter(this);
    ui.ratingFilterSpin->installEventFilter(this);
    ui.dateFromEdit->installEventFilter(this);
    ui.dateToEdit->installEventFilter(this);

    // Placeholder label for no matching photos
    m_placeholderLabel = new QLabel(ui.tableView->viewport());
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->hide();

    QPixmap placeholderPixmap(QCoreApplication::applicationDirPath() + "/resources/no_photos_placeholder.png");

    // Natahneme ho na viewport tabu¾ky
    QSize viewportSize = ui.tableView->viewport()->size();
    m_placeholderLabel->setPixmap(placeholderPixmap.scaled(
        viewportSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    ));
    m_placeholderLabel->resize(viewportSize);

    // Connect buttons and actions
    connect(ui.btnImport, &QPushButton::clicked, this, &TSS_App::importPhotos);
    connect(ui.btnExport, &QPushButton::clicked, this, &TSS_App::exportPhotos);
    connect(ui.btnToggleDarkMode, &QPushButton::clicked, this, &TSS_App::toggleDarkMode);

    // Apply filter button
    connect(ui.btnApplyFilter, &QPushButton::clicked, this, [=]() {
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        model->setDateFilter(ui.dateFromEdit->date(), ui.dateToEdit->date());
        model->setTagFilter(ui.tagFilterEdit->text());
        model->setRatingFilter(ui.ratingFilterSpin->value());
        updatePageLabel();
        });

    // Clear filter button
    connect(ui.btnClearFilter, &QPushButton::clicked, this, [=]() {
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        model->clearFilters();
        ui.tagFilterEdit->clear();
        ui.ratingFilterSpin->setValue(0);
        ui.dateFromEdit->setDate(QDate::currentDate().addMonths(-1));
        ui.dateToEdit->setDate(QDate::currentDate());
        ui.tableView->sortByColumn(PhotoTableModel::DateTime, Qt::DescendingOrder);
        m_placeholderLabel->hide();
        updatePageLabel();
        });

    // Double-click on preview column open detail dialog
    connect(ui.tableView, &QTableView::doubleClicked, this, [=](const QModelIndex& index) {
        if (index.column() != PhotoTableModel::Preview) return;

        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());

        Photo* photo = model->getPhotoPointer(index.row());
        if (!photo) return;

        auto dlg = new PhotoDetailDialog(this);
        dlg->setPhoto(*photo);
        dlg->exec();
        delete dlg;
        });

    // Click on Actions column --> open editor dialog
    connect(ui.tableView, &QTableView::clicked, this, [=](const QModelIndex& index) {
        if (index.column() != PhotoTableModel::Actions) return;
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        Photo* photo = model->getPhotoPointer(index.row());
        if (!photo) return;

        auto editor = new PhotoEditorDialog(photo, this);
        ThemeUtils::setWidgetDarkMode(editor, m_darkMode);
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

    connect(ui.btnFirstPage, &QPushButton::clicked, this, [=]() {
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        model->firstPage();
        updatePageLabel();
        });

    connect(ui.btnLastPage, &QPushButton::clicked, this, [=]() {
        auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
        model->lastPage();
        updatePageLabel();
        });

    connect(ui.comboPageSize, &QComboBox::currentTextChanged, this, [=](const QString& text) {
        bool ok;
        int size = text.toInt(&ok);
        if (ok) {
            auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
            model->setPageSize(size);
            updatePageLabel();
        }
        });

    connect(model, &PhotoTableModel::noPhotosAfterFilter, this, [=](bool empty) {
        if (empty) {
            m_placeholderLabel->resize(ui.tableView->viewport()->size());
            m_placeholderLabel->show();
        }
        else {
            m_placeholderLabel->hide();
        }
        });

    loadSettings();

    model->setDateFilter(ui.dateFromEdit->date(), ui.dateToEdit->date());
    model->setTagFilter(ui.tagFilterEdit->text());
    model->setRatingFilter(ui.ratingFilterSpin->value());

    updatePageLabel();

	// Show placeholder if no photos have been loaded yet
    if (model->getActivePhotos().isEmpty()) 
    {
        m_placeholderLabel->resize(ui.tableView->viewport()->size());
        m_placeholderLabel->show();
    }
}


TSS_App::~TSS_App() {
    saveSettings();
}

// --- Settings Persistence ---
void TSS_App::loadSettings()
{
    QSettings settings("TssApp", "PhotoViewer");

	// Load geometry and state
    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
    restoreState(settings.value("mainWindow/state").toByteArray());

	// Load dark mode
    m_darkMode = settings.value("ui/darkMode", true).toBool();
    ThemeUtils::setWidgetDarkMode(this, m_darkMode);

	// Load column widths
    for (int col = 0; col < PhotoTableModel::ColumnCount; ++col) 
    {
        QString key = QString("table/columnWidth_%1").arg(col);
        if (settings.contains(key)) 
        {
            int width = settings.value(key).toInt();
            ui.tableView->setColumnWidth(col, width);
        }
    }

	// Load page size
    int savedPageSize = settings.value("table/pageSize", 10).toInt();
    QString pageSizeStr = QString::number(savedPageSize);
    int index = ui.comboPageSize->findText(pageSizeStr);
    if (index >= 0) {
        ui.comboPageSize->setCurrentIndex(index);
    }

    // Load filter values into UI
    if (settings.contains("filters/tag")) {
        ui.tagFilterEdit->setText(settings.value("filters/tag").toString());
    }
    if (settings.contains("filters/minRating")) {
        ui.ratingFilterSpin->setValue(settings.value("filters/minRating").toInt());
    }
    if (settings.value("filters/hasDateFilter", false).toBool()) {
        ui.dateFromEdit->setDate(settings.value("filters/dateFrom").toDate());
        ui.dateToEdit->setDate(settings.value("filters/dateTo").toDate());
    }

	// Load model settings (page size, sorting, filters)
    auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
    model->loadSettings();

	// Load last opened folder
    m_currentFolderPath = settings.value("lastFolder").toString();
   
	// Apply sorting
    ui.tableView->sortByColumn(model->currentSortColumn(), model->currentSortOrder());
}

void TSS_App::saveSettings()
{
    QSettings settings("TssApp", "PhotoViewer");

	// Save geometry and state
    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/state", saveState());

	// Save dark mode
    settings.setValue("ui/darkMode", m_darkMode);

	// Save column widths
    for (int col = 0; col < PhotoTableModel::ColumnCount; ++col) {
        QString key = QString("table/columnWidth_%1").arg(col);
        settings.setValue(key, ui.tableView->columnWidth(col));
    }

	// Save filter values from UI
    settings.setValue("filters/tag", ui.tagFilterEdit->text());
    settings.setValue("filters/minRating", ui.ratingFilterSpin->value());
    settings.setValue("filters/dateFrom", ui.dateFromEdit->date());
    settings.setValue("filters/dateTo", ui.dateToEdit->date());

	// Save last opened folder
    if (!m_currentFolderPath.isEmpty()) {
        settings.setValue("lastFolder", m_currentFolderPath);
    }

	// Save model settings (page size, sorting, filters)
    auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
    model->saveSettings();
}

void TSS_App::importPhotos()
{
    PhotoMetadataManager::instance().loadFromFile();

    QString startPath = m_currentFolderPath.isEmpty() ? QDir::homePath() : m_currentFolderPath;
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        "Select folder with photos", 
		startPath // Starting directory
    ); // Open directory selection dialog

    if (dirPath.isEmpty()) return;

    m_currentFolderPath = dirPath;

    // Find image files in the selected directory
    QStringList files;
    QDirIterator it(dirPath, { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif", "*.tiff" },
        QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) files.append(it.next());

    if (files.isEmpty()) // No images found
    {
        QMessageBox::information(this, "No images found", "This folder doesn't contain supported image files.");
        return;
    }

    // Confirm import
    if (QMessageBox::question(this, "Import Photos",
        QString("Found %1 images. Import them?\n\n").arg(files.size()),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;


    auto model = static_cast<PhotoTableModel*>(ui.tableView->model());

    QApplication::setOverrideCursor(Qt::WaitCursor); // Show wait cursor during loading
    model->initializeWithPaths(files); // Lazy load photos

    // Apply current filters from UI AFTER loading
    model->applyFilters();

    QApplication::restoreOverrideCursor(); // Restore normal cursor
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100); // Ensure UI updates
    
    int totalPhotos = files.size();
    const QList<Photo>& visiblePhotos = model->getActivePhotos();
    
    int visibleCount = visiblePhotos.size();
    visibleCount > 0 ? m_placeholderLabel->hide() : m_placeholderLabel->show();

    QString resultMessage;

	// Check if filters are active
    if (visibleCount < totalPhotos)
    {
		// Filters are applied - not all photos are visible
        resultMessage = QString(
            "Successfully imported %1 photos.\n\n"
            "Currently displaying only %2 photos\n"
            "because active filters are applied.\n\n"
            "Click 'Clear Filter' to see all imported photos."
        ).arg(totalPhotos).arg(visibleCount);
    }
    else
    {
		// No filters - all photos are visible
        resultMessage = QString(
            "Successfully imported %1 photos.\n\n"
            "All photos are now visible in the table."
        ).arg(totalPhotos);
    }

    QMessageBox::information(this, "Import Complete", resultMessage);


    updatePageLabel(); // Update pagination label
}


void TSS_App::exportPhotos() {
    auto model = static_cast<PhotoTableModel*>(ui.tableView->model());

    QList<Photo*> photosToExport = model->getPhotosMarkedForExport();

    if (photosToExport.isEmpty()) {
        QMessageBox::information(
            this,
            "No Photos Selected",
            "No photos are marked for export.\n\n"
            "Check the 'Export' checkbox in the table for photos you want to export."
        );
        return;
    }

    // Open the export dialog
    PhotoExportDialog* exportDialog = new PhotoExportDialog(photosToExport, this);
    exportDialog->exec();
    delete exportDialog;
}


void TSS_App::updatePageLabel()
{
    auto model = static_cast<PhotoTableModel*>(ui.tableView->model());
    int current = model->currentPage() + 1;
    int total = model->totalPages();

    ui.lblPage->setText(total == 0
        ? "No results"
        : QString("Page %1 / %2").arg(current).arg(total));

    ui.btnPrevPage->setEnabled(model->currentPage() > 0);
    ui.btnNextPage->setEnabled(current < total);
    ui.btnFirstPage->setEnabled(model->currentPage() > 0);
    ui.btnLastPage->setEnabled(current < total);
}

void TSS_App::toggleDarkMode()
{
    m_darkMode = !m_darkMode;
    ThemeUtils::setWidgetDarkMode(this, m_darkMode); // Apply theme to main window
}


bool TSS_App::eventFilter(QObject* obj, QEvent* event)
{
  
    // Check if Enter/Return was pressed in filter inputs
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            // Check if the object is one of our filter inputs
            if (obj == ui.tagFilterEdit ||
                obj == ui.ratingFilterSpin ||
                obj == ui.dateFromEdit ||
                obj == ui.dateToEdit)
            {
                // Trigger filter application
                ui.btnApplyFilter->click();
                return true; // Event handled
            }
        }
    }
    // Pass event to base class
    return QMainWindow::eventFilter(obj, event);
}

void TSS_App::closeEvent(QCloseEvent* event)
{
    saveSettings();
    event->accept();
}