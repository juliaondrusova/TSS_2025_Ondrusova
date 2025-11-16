#ifndef PHOTOEXPORTDIALOG_H
#define PHOTOEXPORTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QLabel>
#include "Photo.h"

/**
 * @class PhotoExportDialog
 * @brief Dialog for exporting edited photos.
 *
 * @details
 * Displays a list of edited photos, lets the user choose export destinations,
 * validates paths, and performs export with progress tracking.
 */
class PhotoExportDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructs the export dialog  with a list of edited photos.
     * @param editedPhotos List of pointers to edited Photo objects.
     * @param parent Optional parent widget.
     * 
     * @details The dialog will display all photos in a table with options
     * to select/deselect for export, modify export paths, and monitor progress.
     * 
     * @see populateTable(), setupUI()
     */
    explicit PhotoExportDialog(const QList<Photo*>& photosToExport, QWidget* parent = nullptr);

    /**
     * @brief Default destructor.
     */
    ~PhotoExportDialog() = default;

private slots:
    /**
     * @brief Opens a file dialog to choose export path for a photo.
     * @param row The row index in the table.
     */
    void onBrowseClicked(int row);

    /**
     * @brief Selects all photos in the table for export.
     *
     */
    void onSelectAllClicked();

    /**
     * @brief Deselects all photos in the table.
     *
     */
    void onDeselectAllClicked();

    /**
      * @brief Starts exporting all selected photos.
      *
      * @details Validates all paths before export. Updates the progress bar
      * during operation.
      */
    void onExportClicked();

    /**
    * @brief Cancels export and closes the dialog.
    */
    void onCancelClicked();

    /**
     * @brief Opens photo detail preview when double-clicked.
     * @param row Table row index.
     * @param column Table column index.
     */
    void onPreviewDoubleClicked(int row, int column);

    /**
     * @brief Called when an export path is modified in the table.
     * @param row Row index.
     * @param column Column index.
     * @details Validates the new path and updates the status icon.
     */
    void onNewPathChanged(int row, int column);

    /**
     * @brief Toggles inclusion of non-edited photos for export.
     * @param checked True if non-edited photos should be included.
     */
    void onIncludeNonEditedToggled(bool checked);

private:
    /**
     * @brief Initializes and arranges all UI components.
     *
     * @details Creates table, buttons, progress bar, and layouts.
     * Connects signals to corresponding slots.
     */
    void setupUI();

    /**
     * @brief Populates the table with photo information.
     *
     */
    void populateTable();

    /**
     * @brief Creates a single row in the table.
     * @param row Target row index.
     * @param photo Pointer to the photo data.
     */
    void createTableRow(int row, Photo* photo);

    /**
     * @brief Checks if a given export path is valid.
     * @param path File path to validate.
     * @return Empty string if valid, or an error message.
     */
    QString getPathError(const QString& path);

    /**
     * @brief Validates all export paths in the table.
     * @return True if all paths are valid.
     */
    bool validateAllPaths();

    /**
     * @brief Returns a list of selected photos (checked for export).
     * @return List of selected photo pointers.
     */
    QList<Photo*> getSelectedPhotos();

    /// Executes the actual photo export operation.
    void exportPhotos();

    /**
     * @brief Updates the status icon for a specific row.
     * @param row Row index to update.
     */
    void updateStatusIcon(int row);

    // --- UI components ---
    QTableWidget* m_tableWidget;
    QPushButton* m_btnSelectAll;
    QPushButton* m_btnDeselectAll;
    QPushButton* m_btnExport;
    QPushButton* m_btnCancel;
    QProgressBar* m_progressBar;

    // --- Data ---
    QList<Photo*> m_photosToExport;

    /// Table column indexes.
    enum Columns {
        ColCheckbox = 0,
        ColPreview,
        ColOriginalPath,
        ColNewPath,
        ColBrowse,
        ColStatus,
        ColumnCount
    };
};

#endif
