#include "PhotoTableModel.h"
#include "PhotoMetadata.h"
#include <QApplication>
#include <QStyle>
#include <algorithm>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QElapsedTimer>

// Constants
static const QChar STAR_FILLED(0x2605); 
static const QChar STAR_EMPTY(0x2606);  

static const QStringList COLUMN_HEADERS = {
    "Preview", "Name", "Tag", "Rating", "Comment", "Size", "Date", "Actions"
};

// Constructor
PhotoTableModel::PhotoTableModel(QObject* parent)
    : QAbstractTableModel(parent),
    m_filterMinRating(0),
    m_hasFilters(false)
{
}

// --- QAbstractTableModel Interface ---

int PhotoTableModel::rowCount(const QModelIndex&) const {
    const QList<Photo>& photos = getActivePhotos();
    int start = m_currentPage * m_pageSize;
    int remaining = photos.size() - start;
    return qMax(0, qMin(m_pageSize, remaining));
}

int PhotoTableModel::columnCount(const QModelIndex&) const {
    return ColumnCount;
}

QVariant PhotoTableModel::data(const QModelIndex& index, int role) const 
{
    if (!index.isValid())
        return QVariant();

    const QList<Photo>& photos = getActivePhotos();
    int realIndex = getRealIndex(index.row());

    if (realIndex < 0 || realIndex >= photos.size()) 
        return QVariant();

    const Photo& photo = photos[realIndex];
    int column = index.column();

    switch (role) 
    {
    case Qt::DisplayRole:
        return getDisplayText(photo, column);

    case Qt::DecorationRole:
        return getDecoration(photo, column);

    case Qt::TextAlignmentRole:
        return (column == Preview || column == Actions) ? Qt::AlignCenter : QVariant();

    case Qt::ToolTipRole:
        return getTooltip(photo, column);

    default:
        return QVariant();
    }
}

QVariant PhotoTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }

    return (section >= 0 && section < COLUMN_HEADERS.size())
        ? COLUMN_HEADERS[section]
        : QVariant();
}

Qt::ItemFlags PhotoTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    // Allow editing for Tag, Rating, and Comment columns
    if (index.column() == Tag || index.column() == Rating || index.column() == Comment) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool PhotoTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }

    QList<Photo>& photos = getActivePhotos();
    int realIndex = getRealIndex(index.row());

    if (realIndex < 0 || realIndex >= photos.size()) {
        return false;
    }

    Photo& photo = photos[realIndex];

    if (updatePhotoField(photo, index.column(), value)) {
        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
        PhotoMetadataManager::instance().saveToFile();
        return true;
    }

    return false;
}

void PhotoTableModel::sort(int column, Qt::SortOrder order) {
    // Get the active list of photos that needs to be sorted
    QList<Photo>& photos = getActivePhotos();

    // Determine the sorting order: ascending or descending
    bool ascending = (order == Qt::AscendingOrder);

    // std::sort goes through all items and sorts them based on the comparison function
    // The lambda compares two photos and returns true if the first should come before the second
    std::sort(photos.begin(), photos.end(), [column, ascending](const Photo& a, const Photo& b) {
        switch (column) {
        case Name:
            return ascending ? a.filePath() < b.filePath() : a.filePath() > b.filePath();
        case Size:
            return ascending ? a.sizeBytes() < b.sizeBytes() : a.sizeBytes() > b.sizeBytes();
        case DateTime:
            return ascending ? a.dateTime() < b.dateTime() : a.dateTime() > b.dateTime();
        case Rating:
            return ascending ? a.rating() < b.rating() : a.rating() > b.rating();
        default:
            return false; // Unknown column ? keep the current order
        }
        });

    // Notify the table model that the order has changed so the view updates
    emit layoutChanged();
}


// --- Public Interface ---

void PhotoTableModel::addPhoto(const Photo& photo) {
    beginResetModel();
    m_allPhotos.append(photo);

    if (m_hasFilters) {
        applyFilters();
    }
    else {
        m_filteredPhotos.clear();
    }

    endResetModel();
}

Photo PhotoTableModel::photoAt(int row) const {
    const QList<Photo>& photos = getActivePhotos();
    return photos.value(row);
}

// --- Filtering ---

void PhotoTableModel::setRatingFilter(int minRating) {
    m_filterMinRating = minRating;
    applyFilters();
}

void PhotoTableModel::setTagFilter(const QString& tag) {
    m_filterTag = tag;
    applyFilters();
}

void PhotoTableModel::setDateFilter(const QDate& from, const QDate& to) {
    m_filterDateFrom = from;
    m_filterDateTo = to;
    applyFilters();
}

void PhotoTableModel::clearFilters() {
    m_filterDateFrom = QDate();
    m_filterDateTo = QDate();
    m_filterTag.clear();
    m_filterMinRating = 0;
    m_hasFilters = false;
    applyFilters();
}

void PhotoTableModel::applyFilters() {

}

// --- Pagination ---

void PhotoTableModel::nextPage() {
    const QList<Photo>& photos = getActivePhotos();
    int total = photos.size();
    int totalPages = (total + m_pageSize - 1) / m_pageSize;

    if (m_currentPage + 1 < totalPages) {
        beginResetModel();
        ++m_currentPage;
        endResetModel();
    }
}

void PhotoTableModel::prevPage() {
    if (m_currentPage > 0) {
        beginResetModel();
        --m_currentPage;
        endResetModel();
    }
}

int PhotoTableModel::totalPages() const {
    const QList<Photo>& photos = getActivePhotos();
    return (photos.size() + m_pageSize - 1) / m_pageSize;
}


void PhotoTableModel::setPageSize(int newSize)
{
    if (newSize <= 0 || newSize == m_pageSize)
        return;

    beginResetModel();
    m_pageSize = newSize;
    m_currentPage = 0; // reset to first page
    endResetModel();
}


void PhotoTableModel::firstPage()
{
    if (m_currentPage == 0) return;

    beginResetModel();
    m_currentPage = 0;
    endResetModel();
}

void PhotoTableModel::lastPage()
{
    const QList<Photo>& photos = getActivePhotos();
    int total = photos.size();
    int lastPage = (total + m_pageSize - 1) / m_pageSize - 1;

    if (lastPage < 0 || m_currentPage == lastPage)
        return;

    beginResetModel();
    m_currentPage = lastPage;
    endResetModel();
}

// --- Private Helper Methods ---

const QList<Photo>& PhotoTableModel::getActivePhotos() const {
    return m_hasFilters ? m_filteredPhotos : m_allPhotos;
}

QList<Photo>& PhotoTableModel::getActivePhotos() {
    return m_hasFilters ? m_filteredPhotos : m_allPhotos;
}

int PhotoTableModel::getRealIndex(int row) const {
    return m_currentPage * m_pageSize + row;
}

bool PhotoTableModel::hasActiveFilters() const {
    return (m_filterDateFrom.isValid() && m_filterDateTo.isValid()) ||
        !m_filterTag.isEmpty() ||
        (m_filterMinRating > 0);
}

bool PhotoTableModel::photoPassesFilters(const Photo& photo) const {
   
    return true;
}

QVariant PhotoTableModel::getDisplayText(const Photo& photo, int column) const {
    switch (column) {
    case Name:     return photo.filePath();
    case Tag:      return photo.tag();
    case Rating:   return formatRatingStars(photo.rating());
    case Comment:  return photo.comment();
    case Size:     return photo.size();
    case DateTime: return photo.dateTime().toString("dd.MM.yyyy hh:mm");
    default:       return QVariant();
    }
}

QVariant PhotoTableModel::getDecoration(const Photo& photo, int column) const {
    if (column == Preview) {
        return photo.preview();
    }

    if (column == Actions) {
        QIcon icon = QApplication::style()->standardIcon(QStyle::SP_ArrowForward);
        return icon.pixmap(25, 25);
    }

    return QVariant();
}


QVariant PhotoTableModel::getTooltip(const Photo& photo, int column) const {
    switch (column) {
    case Preview:  return QString("Double-click to open photo detail");
    case Name:     return photo.filePath();
    case Tag:      return photo.tag();
    case Rating:   return QString("Enter value from 0 to 5");
    case Comment:  return photo.comment();
    case Size:     return photo.size();
    case DateTime: return photo.dateTime().toString("dd.MM.yyyy hh:mm");
    case Actions:  return QString("Edit photo");
    default:       return QVariant();
    }
}

QString PhotoTableModel::formatRatingStars(int rating) const {
    QString stars;
    stars.reserve(5);

    for (int i = 0; i < 5; ++i) {
        stars += (i < rating) ? STAR_FILLED : STAR_EMPTY;
    }

    return stars;
}

bool PhotoTableModel::updatePhotoField(const Photo& photo, int column, const QVariant& value) {
    QString filePath = photo.filePath();
        return false;
}


// --- Lazy Loading Interface ---

void PhotoTableModel::initializeWithPaths(const QStringList& allPaths) 
{
    beginResetModel();

    // Keep old photos, do not clear them
    int oldSize = m_allPhotos.size();
    m_currentPage = 0;

    QProgressDialog progress("Loading photos...", "Cancel", 0, allPaths.size());
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setWindowTitle("Initializing Photos");
    progress.setMinimumDuration(0);
    progress.show();

    // Pre-allocate memory for efficiency
    m_allPhotos.reserve(oldSize + allPaths.size());

    for (int i = 0; i < allPaths.size(); ++i) 
    {
        if (progress.wasCanceled())
            break;

        // Append new Photo with only the file path (no heavy data yet)
        m_allPhotos.append(Photo(allPaths[i]));
      
       // Update progress
        progress.setValue(i + 1);
        QCoreApplication::processEvents(); // refresh GUI
    }

    progress.close();
    endResetModel();
}