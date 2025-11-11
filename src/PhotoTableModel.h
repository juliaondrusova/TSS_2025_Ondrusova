#pragma once
#include <QAbstractTableModel>
#include "Photo.h"

/**
 * @brief Table model for displaying photos with pagination, filtering, and sorting
 *
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
        ColumnCount ///< Total column count
    };

    explicit PhotoTableModel(QObject* parent = nullptr);

    // QAbstractTableModel interface
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
     * @return Photo object
     */
    Photo photoAt(int row) const;
    Photo* getPhotoPointer(int row);

    // Filtering
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

    // Pagination
    /**
     * @brief Move to next page
     */
    void nextPage();

    /**
     * @brief Move to previous page
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



    void firstPage();
    void lastPage();
    void setPageSize(int newSize);


    /**
     * @brief Get total number of pages
     * @return Total pages for current filter set
     */
    int totalPages() const;

    /**
    * @brief Initialize the model with a list of photo paths.
    * @param allPaths List of photo file paths to add to the model.
    */
    void initializeWithPaths(const QStringList& allPaths);

    QList<Photo*> getAllEditedPhotos();

private:
    /**
     * @brief Apply current filters to photo collection
     */
    void applyFilters();

    /**
     * @brief Get reference to active photo list (filtered or all)
     * @return Reference to QList<Photo>
     */
    const QList<Photo>& getActivePhotos() const;
    QList<Photo>& getActivePhotos();
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

    /**
     * @brief Get display text for a cell
     * @param photo Photo object
     * @param column Column index
     * @return Display text
     */
    QVariant getDisplayText(const Photo& photo, int column) const;

    /**
     * @brief Get decoration (icon/image) for a cell
     * @param photo Photo object
     * @param column Column index
     * @return Decoration data
     */
    QVariant getDecoration(const Photo& photo, int column) const;

    /**
      * @brief Get tooltip text for a cell
      * @param photo Photo object
      * @param column Column index
      * @return Tooltip text
      */
    QVariant getTooltip(const Photo& photo, int column) const;

    /**
    * @brief Format rating as star characters
    * @param rating Rating value (0-5)
    * @return String with filled and empty stars
    */
    QString formatRatingStars(int rating) const;

    /**
     * @brief Update photo data for a specific column
     * @param photo Photo to update
     * @param column Column being edited
     * @param value New value
     * @return True if data was updated
     */
    bool updatePhotoField(Photo& photo, int column, const QVariant& value);

    // Storage
    QList<Photo> m_allPhotos;      ///< All photos
    QList<Photo> m_filteredPhotos; ///< Filtered photos
    bool m_hasFilters;             ///< True if filters are active

    // Pagination
    int m_pageSize = 10;     ///< Items per page
    int m_currentPage = 0;   ///< Current page (0-based)

    // Filter conditions
    QDate m_filterDateFrom;    ///< Filter: start date
    QDate m_filterDateTo;      ///< Filter: end date
    QString m_filterTag;       ///< Filter: tag substring
    int m_filterMinRating;     ///< Filter: minimum rating
};