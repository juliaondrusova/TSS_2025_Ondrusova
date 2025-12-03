#pragma once
#include <QAbstractTableModel>
#include "Photo.h"

/**
 * @brief Table model for displaying photos with pagination, filtering, and sorting
 *
 * @details
 * This model manages photo display in a table view with support for:
 * - Pagination (default 10 items per page)
 * - Filtering by date range, tag, and minimum rating
 * - Column sorting
 * - Inline editing of tag, rating, and comment fields
 * - Automatic persistence to JSON storage
 */
class PhotoTableModel : public QAbstractTableModel {
    Q_OBJECT

signals:
    /**
     * Emitted after filters are applied.
     * @param empty true if no photos matched the filters
     */
    void noPhotosAfterFilter(bool empty);

public:
  
    /**
     * @brief Column indices for the table
     */
    enum Columns {
        Preview,    ///< Photo thumbnail
        Name,       ///< File path
        Tag,        ///< User-defined tag (editable)
        Rating,     ///< Star rating 0-5 (editable)
        Comment,    ///< User comment (editable)
        Size,       ///< File size
        DateTime,   ///< Last modified date/time
        Actions,    ///< Action buttons
		Export,	    ///< Export checkbox
        ColumnCount ///< Total column count
    };

    /**
     * @brief Constructs an empty photo table model.
     * @param parent Optional parent object.
     */
    explicit PhotoTableModel(QObject* parent = nullptr);

    // --- QAbstractTableModel interface ---
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    /**
     * @brief Add a new photo to the model
     * @param photo Photo object to add
     */
    void addPhoto(const Photo& photo);

    /**
     * @brief Get photo at specific row (relative to current page)
     * @param row Row index within current page
     * @return Copy of the requested Photo
     */
    Photo photoAt(int row) const;

    /**
     * @brief Returns pointer to photo at selected row (current page).
     * @param row Row index relative to current page.
     * @return Pointer to Photo in internal storage.
     */
    Photo* getPhotoPointer(int row);

    // --- Filtering ---
    /**
     * @brief Filter photos by date range
     * @param from Start date (inclusive)
     * @param to End date (inclusive)
     */
    void setDateFilter(const QDate& from, const QDate& to);

    /**
     * @brief Filter photos by tag (case-insensitive substring match)
     * @param tag Tag to filter by
     */
    void setTagFilter(const QString& tag);

    /**
     * @brief Filter photos by minimum rating
     * @param minRating Minimum rating (0-5)
     */
    void setRatingFilter(int minRating);

    /**
     * @brief Clear all active filters
     */
    void clearFilters();

    // --- Pagination --- 
    /**
     * @brief Move to next page, if available
     */
    void nextPage();

    /**
     * @brief Move to previous page, if available
     */
    void prevPage();

    /**
     * @brief Get current page number
     * @return Current page (0-based)
     */
    int currentPage() const { return m_currentPage; }

    /**
     * @brief Get page size
     * @return Number of items per page
     */
    int pageSize() const { return m_pageSize; }


    /**
    * @brief Moves to the first page.
    */
    void firstPage();

    /**
     * @brief Moves to the last available page.
     */
    void lastPage();

    /**
    * @brief Sets number of items displayed per page.
    * @param newSize New page size.
    */
    void setPageSize(int newSize);


    /**
     * @brief Get total number of pages
     * @return Total pages for current filter set
     */
    int totalPages() const;

    /**
    * @brief Initialize the model with a list of photo paths.
    * @param allPaths List of absolute file paths.
    */
    void initializeWithPaths(const QStringList& allPaths);

    /**
     * @brief Returns all photos that were edited.
     * @return List of pointers to edited photos.
     */
    QList<Photo*> getAllEditedPhotos();

    /**
     * @brief Access active (filtered or unfiltered) photos.
     * @return Const reference to active photo list.
     */
    const QList<Photo>& getActivePhotos() const;

    QList<Photo>& getActivePhotos();

    /**
    * @brief Retrieves photos marked for export.
    * @return List of Photo pointers selected for export.
    */
    QList<Photo*> getPhotosMarkedForExport();


    /**
     * @brief Load saved settings (page size, sorting, filters)
     */
    void loadSettings();

    /**
     * @brief Save current settings (page size, sorting, filters)
     */
    void saveSettings();

    /**
     * @brief Get current sort column
     * @return Column index being sorted by
     */
    int currentSortColumn() const { return m_sortColumn; }

    /**
     * @brief Get current sort order
     * @return Qt::AscendingOrder or Qt::DescendingOrder
     */
    Qt::SortOrder currentSortOrder() const { return m_sortOrder; }

private:
    // --- Internal filtering helpers ---

    /**
    * @brief Apply current filters to photo collection
    */
    void applyFilters();

    /**
     * @brief Convert table row to real index in photo list
     * @param row Table row number
     * @return Real index in m_allPhotos or m_filteredPhotos
     */
    int getRealIndex(int row) const;

    /**
     * @brief Check if any filter is currently active
     * @return True if filters are active
     */
    bool hasActiveFilters() const;

    /**
     * @brief Check if photo passes all active filters
     * @param photo Photo to check
     * @return True if photo passes filters
     */
    bool photoPassesFilters(const Photo& photo) const;

    // --- Data formatting for table view ---
    /**
     * @brief eturns text data for a column
     * @param photo Photo being displayed
     * @param column Column index
     * @return Display string
     */
    QVariant getDisplayText(const Photo& photo, int column) const;

    /**
     * @brief Get decoration (icon/image) for a cell
     * @param photo Photo being displayed.
     * @param column Column index
     * @return QVariant containing decoration data
     */
    QVariant getDecoration(const Photo& photo, int column) const;

    /**
     * @brief Get tooltip text for a cell
     * @param photo Photo being displayed
     * @param column Column index
     * @return Tooltip string
     */
    QVariant getTooltip(const Photo& photo, int column) const;

    /**
    * @brief Format rating as star characters
    * @param rating Rating value (0-5)
    * @return Unicode star representation
    */
    QString formatRatingStars(int rating) const;

    /**
     * @brief Updates a specific field on a Photo object.
     * @param photo Target photo.
     * @param column Column being modified.
     * @param value Replacement value.
     * @return True if the field was updated.
     */
    bool updatePhotoField(Photo& photo, int column, const QVariant& value);

    // --- Storage ---
    QList<Photo> m_allPhotos;      ///< Full original photo list
    QList<Photo> m_filteredPhotos; ///< Filtered photos (if filters active)
    bool m_hasFilters;             ///< Indicates if filtered mode is active

    // --- Pagination ---
    int m_pageSize = 10;     ///< Items per page
    int m_currentPage = 0;   ///< Current page (0-based)

    // --- Filter conditions ---
    QDate m_filterDateFrom;    ///< Filter: start date
    QDate m_filterDateTo;      ///< Filter: end date
    QString m_filterTag;       ///< Filter: tag substring
    int m_filterMinRating;     ///< Filter: minimum rating

	// --- Sorting ---
    int m_sortColumn = DateTime;              ///< Current sort column
    Qt::SortOrder m_sortOrder = Qt::DescendingOrder; ///< Current sort order
};