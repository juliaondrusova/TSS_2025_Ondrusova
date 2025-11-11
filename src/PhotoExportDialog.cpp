#include "PhotoExportDialog.h"
#include "PhotoDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QTimer>

PhotoExportDialog::PhotoExportDialog(const QList<Photo*>& editedPhotos, QWidget* parent)
    : QDialog(parent),
    m_editedPhotos(editedPhotos)
{
    setWindowTitle("Export Edited Photos");
    resize(1000, 600);

	// Setup UI components
    setupUI();
	// Populate table with edited photos
    populateTable();
}

void PhotoExportDialog::setupUI() 
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Info label
    QLabel* infoLabel = new QLabel(
        QString("Found %1 edited photo(s). Select which to export and specify output paths.")
        .arg(m_editedPhotos.size()), this
    );
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Table widget
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(ColumnCount);
    m_tableWidget->setHorizontalHeaderLabels({
        "Export", "Preview", "Original Path", "New Path", "Browse", "Status"
        });

	// Set column widths and properties
    m_tableWidget->horizontalHeader()->setStretchLastSection(false);
    m_tableWidget->setColumnWidth(ColCheckbox, 80);
    m_tableWidget->setColumnWidth(ColPreview, 120);
    m_tableWidget->setColumnWidth(ColOriginalPath, 280);
    m_tableWidget->setColumnWidth(ColNewPath, 280);
    m_tableWidget->setColumnWidth(ColBrowse, 110);
    m_tableWidget->setColumnWidth(ColStatus, 80);

    m_tableWidget->verticalHeader()->setDefaultSectionSize(80);
    m_tableWidget->verticalHeader()->hide();
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    mainLayout->addWidget(m_tableWidget);


    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_btnSelectAll = new QPushButton("Select All", this);
    m_btnDeselectAll = new QPushButton("Deselect All", this);
    buttonLayout->addWidget(m_btnSelectAll);
    buttonLayout->addWidget(m_btnDeselectAll);
	buttonLayout->addStretch(); 

    m_btnCancel = new QPushButton("Cancel", this);
    m_btnExport = new QPushButton("Export", this);
	m_btnExport->setDefault(true); //can be triggered by Enter key

    buttonLayout->addWidget(m_btnCancel);
    buttonLayout->addWidget(m_btnExport);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_btnSelectAll, &QPushButton::clicked, this, &PhotoExportDialog::onSelectAllClicked);
    connect(m_btnDeselectAll, &QPushButton::clicked, this, &PhotoExportDialog::onDeselectAllClicked);
    connect(m_btnExport, &QPushButton::clicked, this, &PhotoExportDialog::onExportClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &PhotoExportDialog::onCancelClicked);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked, this, &PhotoExportDialog::onPreviewDoubleClicked);
    connect(m_tableWidget, &QTableWidget::cellChanged, this, &PhotoExportDialog::onNewPathChanged);
}

void PhotoExportDialog::populateTable() 
{ 
    m_tableWidget->setRowCount(m_editedPhotos.size()); 
	m_tableWidget->blockSignals(true); // Prevent signals during setup

    for (int i = 0; i < m_editedPhotos.size(); ++i)
        createTableRow(i, m_editedPhotos[i]);

	m_tableWidget->blockSignals(false); // Re-enable signals

    // Now update status icons for all rows (widgets/items are fully created).
    for (int i = 0; i < m_editedPhotos.size(); ++i)
        updateStatusIcon(i);
}

void PhotoExportDialog::createTableRow(int row, Photo* photo) 
{
    // Column 0: Checkbox
    QWidget* checkboxWidget = new QWidget();
    QCheckBox* checkbox = new QCheckBox();
    checkbox->setChecked(true); // Default checked
    QHBoxLayout* checkboxLayout = new QHBoxLayout(checkboxWidget);
    checkboxLayout->addWidget(checkbox);
    checkboxLayout->setAlignment(Qt::AlignCenter);
    m_tableWidget->setCellWidget(row, ColCheckbox, checkboxWidget);

    // Column 1: Preview
    QLabel* previewLabel = new QLabel();
    QPixmap preview = photo->editedPixmap().scaled(
        100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation
    );
    previewLabel->setPixmap(preview);
    previewLabel->setAlignment(Qt::AlignCenter);
    m_tableWidget->setCellWidget(row, ColPreview, previewLabel);

    // Column 2: Original Path (read-only)
    QTableWidgetItem* originalPathItem = new QTableWidgetItem(photo->filePath());
	originalPathItem->setFlags(originalPathItem->flags() & ~Qt::ItemIsEditable); // Make read-only
    originalPathItem->setToolTip(photo->filePath());
    m_tableWidget->setItem(row, ColOriginalPath, originalPathItem);

    // Column 3: New Path (editable)
    QTableWidgetItem* newPathItem = new QTableWidgetItem(photo->filePath());
	newPathItem->setFlags(newPathItem->flags() | Qt::ItemIsEditable); // Make editable
    newPathItem->setToolTip("Double-click to edit or use Browse button");
    m_tableWidget->setItem(row, ColNewPath, newPathItem);

    // Column 4: Browse button
    QPushButton* browseBtn = new QPushButton("Browse...", this);
    connect(browseBtn, &QPushButton::clicked, this, [this, row]() { 
		onBrowseClicked(row); //open folder dialog
        });
    m_tableWidget->setCellWidget(row, ColBrowse, browseBtn);

    // Column 5: Status placeholder - initialize empty
    QLabel* statusLabel = new QLabel();
    statusLabel->setAlignment(Qt::AlignCenter);
    m_tableWidget->setCellWidget(row, ColStatus, statusLabel);
}

void PhotoExportDialog::onBrowseClicked(int row) 
{
    QString currentPath = m_tableWidget->item(row, ColNewPath)->text();
    QFileInfo fileInfo(currentPath);

    QString selectedFolder = QFileDialog::getExistingDirectory(
        this,
        "Select Export Folder",
        fileInfo.absolutePath(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks // only directories and no symlink resolution
    );

    if (!selectedFolder.isEmpty()) 
    {
		// Original file name
        QString fileName = fileInfo.fileName();
        QString newPath = QDir(selectedFolder).filePath(fileName);

        m_tableWidget->item(row, ColNewPath)->setText(newPath);

        // Update status icon
        updateStatusIcon(row);
    }
}

void PhotoExportDialog::onSelectAllClicked() 
{
    for (int i = 0; i < m_tableWidget->rowCount(); i++) 
    {
        QWidget* widget = m_tableWidget->cellWidget(i, ColCheckbox);
		QCheckBox* checkbox = widget->findChild<QCheckBox*>(); // Find the checkbox
		if (checkbox) // Set it checked
            checkbox->setChecked(true);
    }
}

void PhotoExportDialog::onDeselectAllClicked() 
{
    for (int i = 0; i < m_tableWidget->rowCount(); i++) 
    {
        QWidget* widget = m_tableWidget->cellWidget(i, ColCheckbox);
		QCheckBox* checkbox = widget->findChild<QCheckBox*>(); // Find the checkbox
		if (checkbox) // Set it unchecked
            checkbox->setChecked(false);
    }
}

void PhotoExportDialog::onPreviewDoubleClicked(int row, int column) 
{
    if (column != ColPreview) return;

    Photo* photo = m_editedPhotos[row];
    PhotoDetailDialog* dlg = new PhotoDetailDialog(this); 
	dlg->setPhoto(*photo); // Set photo to display
	dlg->exec();
	delete dlg; // Clean up after closing
}

void PhotoExportDialog::onNewPathChanged(int row, int column) 
{
    if (column != ColNewPath) return;

    // Update status icon for this row
    updateStatusIcon(row);
}

bool PhotoExportDialog::validateAllPaths() 
{
    QStringList invalidRows;

	// Check each selected row for path validity
    for (int i = 0; i < m_tableWidget->rowCount(); i++) 
    {
        QWidget* widget = m_tableWidget->cellWidget(i, ColCheckbox);
        QCheckBox* checkbox = widget->findChild<QCheckBox*>();

        if (checkbox && checkbox->isChecked()) 
        {
            QString newPath = m_tableWidget->item(i, ColNewPath)->text();
            QString error = getPathError(newPath);

			if (!error.isEmpty()) // Found an error
                invalidRows.append(QString("Row %1: %2").arg(i + 1).arg(error)); 
        }
    }
	// If any invalid paths found, show warning
    if (!invalidRows.isEmpty()) 
    {
        QMessageBox::warning(
            this,
            "Invalid Paths",
            QString("Found %1 invalid export path(s):\n\n%2\n\n"
                "Please fix the errors before exporting.") 
            .arg(invalidRows.size())
            .arg(invalidRows.join("\n"))
        );
        return false;
    }

    return true;
}


QList<Photo*> PhotoExportDialog::getSelectedPhotos() 
{
    QList<Photo*> selected;

	// Collect all selected photos
    for (int i = 0; i < m_tableWidget->rowCount(); ++i) 
    {
        QWidget* widget = m_tableWidget->cellWidget(i, ColCheckbox);
        QCheckBox* checkbox = widget->findChild<QCheckBox*>();

		if (checkbox && checkbox->isChecked())  // If checked, add to selected list
            selected.append(m_editedPhotos[i]);
    }
    return selected;
}

QString PhotoExportDialog::getPathError(const QString& path) 
{
    if (path.isEmpty())
        return "Path is empty";

    QFileInfo fileInfo(path);

    // Check if directory exists
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists())
        return "Directory does not exist";

    // Check if file has valid extension
    QString suffix = fileInfo.suffix().toLower();
    QStringList validExtensions = { "png", "jpg", "jpeg", "bmp", "tiff", "gif" };
    if (!validExtensions.contains(suffix))
        return "Invalid file extension (use: " + validExtensions.join(", ") + ")";

    return ""; // No error
}

void PhotoExportDialog::updateStatusIcon(int row) 
{
    if (!m_tableWidget->item(row, ColNewPath)) return;

    QString newPath = m_tableWidget->item(row, ColNewPath)->text();
    QString error = getPathError(newPath);

    QLabel* statusLabel = qobject_cast<QLabel*>(m_tableWidget->cellWidget(row, ColStatus)); 
	// Create label if not existing
    if (!statusLabel) 
    {
        statusLabel = new QLabel();
        statusLabel->setAlignment(Qt::AlignCenter);
        m_tableWidget->setCellWidget(row, ColStatus, statusLabel);
    }

	//Green check for valid
    if (error.isEmpty()) 
    {
        QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
        statusLabel->setPixmap(icon.pixmap(24, 24));
        statusLabel->setToolTip("Path is valid");
    }
    //Red cross for invalid
    else 
    {
        QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
        statusLabel->setPixmap(icon.pixmap(24, 24));
        statusLabel->setToolTip(error);
    }
}


void PhotoExportDialog::onExportClicked() 
{
    // Validate all paths first
    if (!validateAllPaths())
        return;

	QList<Photo*> selectedPhotos = getSelectedPhotos(); // get checked photos

	// Check if any photos selected
    if (selectedPhotos.isEmpty()) 
    {
        QMessageBox::information(this, "No Selection", "Please select at least one photo to export.");
        return;
    }

    // Check for overwrites
    QStringList overwriteList;
    for (int i = 0; i < m_tableWidget->rowCount(); i++) 
    {
        QWidget* widget = m_tableWidget->cellWidget(i, ColCheckbox);
        QCheckBox* checkbox = widget->findChild<QCheckBox*>();

        if (checkbox && checkbox->isChecked()) 
        {
            QString originalPath = m_tableWidget->item(i, ColOriginalPath)->text();
            QString newPath = m_tableWidget->item(i, ColNewPath)->text();

			if (originalPath == newPath) // Same path, will overwrite
                overwriteList.append(QFileInfo(originalPath).fileName());
        }
    }

	// Handle overwrites
    if (!overwriteList.isEmpty()) 
    {
		// Overwrite warning message
        QString message = QString("The following %1 file(s) will be OVERWRITTEN:\n\n%2\n\nContinue?")
            .arg(overwriteList.size())
            .arg(overwriteList.join("\n"));

		// Confirm overwrite
        if (QMessageBox::question(this, "Confirm Overwrite", message,
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
    }

    // Confirm export
    QString confirmMsg = QString("Export %1 edited photo(s)?").arg(selectedPhotos.size());
    if (QMessageBox::question(this, "Confirm Export", confirmMsg,
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

	// Start export process
    exportPhotos();
}

void PhotoExportDialog::exportPhotos() 
{
    // Disable buttons during export
    m_btnExport->setEnabled(false);
    m_btnCancel->setEnabled(false);
    m_btnSelectAll->setEnabled(false);
    m_btnDeselectAll->setEnabled(false);

    // Show progress bar
    m_progressBar->setVisible(true);
    m_progressBar->setMaximum(m_tableWidget->rowCount());
    m_progressBar->setValue(0);

    int exportedCount = 0;
    int failedCount = 0;
    QStringList failedFiles;

	// iterate through all rows
    for (int i = 0; i < m_tableWidget->rowCount(); i++) 
    {
        QWidget* widget = m_tableWidget->cellWidget(i, ColCheckbox);
        QCheckBox* checkbox = widget->findChild<QCheckBox*>();

		if (checkbox && checkbox->isChecked()) // check if selected
        {
            QString newPath = m_tableWidget->item(i, ColNewPath)->text();
            Photo* photo = m_editedPhotos[i];

			QApplication::processEvents(); // Keep UI responsive

            QPixmap editedPixmap = photo->editedPixmap();  // Save the edited pixmap

			// Attempt to save the edited pixmap
            if (editedPixmap.save(newPath))
            {
                exportedCount++;
            }
			// failed to save
            else 
            {
                failedCount++;
                failedFiles.append(QFileInfo(newPath).fileName());
            }
        }

        m_progressBar->setValue(i + 1);
        QApplication::processEvents();
    }

    // Hide progress bar
    m_progressBar->setVisible(false);

    // Show results
    QString resultMsg;
	// All successful
    if (failedCount == 0) 
    {
        resultMsg = QString("Successfully exported %1 photo(s)!").arg(exportedCount);
        QMessageBox::information(this, "Export Complete", resultMsg);
        accept(); // Close dialog
    }
	// At least one failed
    else 
    {
        resultMsg = QString("Exported %1 photo(s).\n\nFailed to export %2 photo(s):\n%3")
            .arg(exportedCount)
            .arg(failedCount)
            .arg(failedFiles.join("\n"));
        QMessageBox::warning(this, "Export Completed with Errors", resultMsg);
    }

    // Re-enable buttons
    m_btnExport->setEnabled(true);
    m_btnCancel->setEnabled(true);
    m_btnSelectAll->setEnabled(true);
    m_btnDeselectAll->setEnabled(true);
}

void PhotoExportDialog::onCancelClicked() 
{
	reject(); // Close dialog without exporting
}