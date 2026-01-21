#include "PhotoTableModel.h"
#include "PhotoMetadata.h"
#include <QApplication>
#include <QStyle>
#include <algorithm>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QSettings>

// Constants
static const QChar STAR_FILLED(0x2605); 
static const QChar STAR_EMPTY(0x2606);  

// Column indices
static const QStringList COLUMN_HEADERS = {
	"Preview", "Name", "Tag", "Rating", "Comment", "Size", "Date", "Actions", "Export"
};

// Constructor
PhotoTableModel::PhotoTableModel(QObject* parent)
    : QAbstractTableModel(parent),
    m_filterMinRating(0),
    m_hasFilters(false)
{
}

// --- Row Count with Pagination ---
int PhotoTableModel::rowCount(const QModelIndex&) const 
{
	const QList<Photo>& photos = getActivePhotos(); // Get the current list of photos (filtered or all)
    int start = m_currentPage * m_pageSize;
    int remaining = photos.size() - start;
    return qMax(0, qMin(m_pageSize, remaining)); 
}

// --- Fixed Column Count ---
int PhotoTableModel::columnCount(const QModelIndex&) const {
	return ColumnCount; // Fixed number of columns
}

// --- Data Retrieval for Each Cell ---
QVariant PhotoTableModel::data(const QModelIndex& index, int role) const 
{
    if (!index.isValid()) // Check if the index is valid
        return QVariant();

	const QList<Photo>& photos = getActivePhotos(); // Get the current list of photos (filtered or all)
    int realIndex = getRealIndex(index.row());

	if (realIndex < 0 || realIndex >= photos.size()) // Check if the real index is within bounds
        return QVariant();

    const Photo& photo = photos[realIndex];
    int column = index.column();

	// Return data based on the requested role
    switch (role) 
    {
	case Qt::DisplayRole: // Text to display
        return getDisplayText(photo, column);

	case Qt::DecorationRole: // Icons or images
        return getDecoration(photo, column);

	case Qt::TextAlignmentRole: // Text alignment
        return (column == Preview || column == Actions) ? Qt::AlignCenter : QVariant();
   
	case Qt::CheckStateRole:  // Checkbox state
        if (column == Export)
            return photo.isMarkedForExport() ? Qt::Checked : Qt::Unchecked;
        return QVariant();

	case Qt::ToolTipRole: // Tooltip text
        return getTooltip(photo, column);

	default: // Unknown role
        return QVariant();
    }
}

// --- Header Data in Table ---
QVariant PhotoTableModel::headerData(int section, Qt::Orientation orientation, int role) const 
{
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal) // Only handle horizontal headers for display role
        return QVariant();

    return (section >= 0 && section < COLUMN_HEADERS.size())
		? COLUMN_HEADERS[section] // Return the header text for the column
        : QVariant();
}

//  --- State Flags for Each Item ---
Qt::ItemFlags PhotoTableModel::flags(const QModelIndex& index) const 
{
	if (!index.isValid()) // Invalid index
        return Qt::NoItemFlags;

	Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled; // selectable and enabled

    // Allow editing for Tag, Rating, and Comment columns
    if (index.column() == Tag || index.column() == Rating || index.column() == Comment)
        flags |= Qt::ItemIsEditable;

    // Allow checking for Export column
    if (index.column() == Export) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

// --- Set Data to Model ---
bool PhotoTableModel::setData(const QModelIndex& index, const QVariant& value, int role) 
{
	if (!index.isValid()) // Invalid index
        return false;

	QList<Photo>& photos = getActivePhotos(); // Get the current list of photos (filtered or all)
    int realIndex = getRealIndex(index.row());

    if (realIndex < 0 || realIndex >= photos.size()) //
        return false;

    Photo& photo = photos[realIndex];

    if (index.column() == Export && role == Qt::CheckStateRole) {
        photo.setMarkedForExport(value.toInt() == Qt::Checked);
        emit dataChanged(index, index, { Qt::CheckStateRole });
        return true;
    }

    if (role != Qt::EditRole)
        return false;

	// Update the appropriate field based on the column
    if (updatePhotoField(photo, index.column(), value)) 
    {
		emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole }); // Notify view of data change
		PhotoMetadataManager::instance().saveToFile(); // Save metadata changes
        return true;
    }

    return false;
}

// --- Sorting ---
void PhotoTableModel::sort(int column, Qt::SortOrder order) 
{
    m_sortColumn = column;
    m_sortOrder = order;

    QList<Photo>& photos = getActivePhotos(); // Get the current list of photos (filtered or all)

	bool ascending = (order == Qt::AscendingOrder); // true for ascending, false for descending

    // std::sort goes through all items and sorts them based on the comparison function
    // The lambda compares two photos and returns true if the first should come before the second
    std::sort(photos.begin(), photos.end(), [column, ascending](const Photo& a, const Photo& b) {
        switch (column)
        {
        case Name:
            return ascending ? a.filePath() > b.filePath() : a.filePath() < b.filePath();
        case Size:
            return ascending ? a.sizeBytes() > b.sizeBytes() : a.sizeBytes() < b.sizeBytes();
        case DateTime:
            return ascending ? a.dateTime() > b.dateTime() : a.dateTime() < b.dateTime();
        case Rating:
            return ascending ? a.rating() > b.rating() : a.rating() < b.rating();
        default:
            return false;
        }
	});
	emit layoutChanged(); // Notify view that the layout has changed
}

// --- Add Photo ---
void PhotoTableModel::addPhoto(const Photo& photo) 
{
	beginResetModel(); // Notify view of upcoming changes
    m_allPhotos.append(photo);

	if (m_hasFilters) // If filters are active, re-apply them
        applyFilters();
	else // No filters, just clear filtered list
        m_filteredPhotos.clear();

	endResetModel(); // Notify view that changes are done
}


// --- Filtering ---

// --- Set rating filter ---
void PhotoTableModel::setRatingFilter(int minRating) 
{
    m_filterMinRating = minRating;
    applyFilters();
}

// --- Set tag filter ---
void PhotoTableModel::setTagFilter(const QString& tag) {
    m_filterTag = tag;
    applyFilters();
}

// --- Set date range filter ---
void PhotoTableModel::setDateFilter(const QDate& from, const QDate& to) {
    m_filterDateFrom = from;
    m_filterDateTo = to;
    applyFilters();
}

// --- Clear all filters ---
void PhotoTableModel::clearFilters() 
{
	// Reset all filter criteria
    m_filterDateFrom = QDate();
    m_filterDateTo = QDate();
    m_filterTag.clear();
    m_filterMinRating = 0;
    m_hasFilters = false;
    applyFilters();
}

// --- Apply current filters to photo collection ---
void PhotoTableModel::applyFilters() 
{
	beginResetModel(); // Notify view of upcoming changes
    m_filteredPhotos.clear();

	m_hasFilters = hasActiveFilters(); // Check if any filters are active

    if (!m_hasFilters)
    {
        endResetModel();
        return;
    }

    // Copy photos that pass all filters
    std::copy_if(m_allPhotos.begin(), m_allPhotos.end(),
        std::back_inserter(m_filteredPhotos),
		[this](const Photo& photo) { return photoPassesFilters(photo); }); // Filter photos

	endResetModel(); // Notify view that changes are done

	emit noPhotosAfterFilter(m_filteredPhotos.isEmpty()); // Notify if no photos match filters
}


// --- Pagination ---

// --- Move to next page ---
void PhotoTableModel::nextPage() 
{
	const QList<Photo>& photos = getActivePhotos(); // Get the current list of photos (filtered or all)
    int total = photos.size();
    int totalPages = (total + m_pageSize - 1) / m_pageSize;

	// Move to next page if not on the last page
    if (m_currentPage + 1 < totalPages) 
    {
        beginResetModel();
        ++m_currentPage;
        endResetModel();
    }
}

// --- Move to previous page ---
void PhotoTableModel::prevPage() 
{
	// Move to previous page if not on the first page
    if (m_currentPage > 0) {
        beginResetModel();
        --m_currentPage;
        endResetModel();
    }
}

// --- Get total number of pages ---
int PhotoTableModel::totalPages() const 
{
    const QList<Photo>& photos = getActivePhotos();
	return (photos.size() + m_pageSize - 1) / m_pageSize; // calculate total pages
}

// --- Set page size ---
void PhotoTableModel::setPageSize(int newSize)
{
	if (newSize <= 0 || newSize == m_pageSize) // invalid size or no change
        return;

    beginResetModel();
    m_pageSize = newSize;
    m_currentPage = 0; // reset to first page
    endResetModel();
}

// --- Move to first page ---
void PhotoTableModel::firstPage()
{
    if (m_currentPage == 0) return;

    beginResetModel();
    m_currentPage = 0;
    endResetModel();
}

// --- Move to last page ---
void PhotoTableModel::lastPage()
{
	const QList<Photo>& photos = getActivePhotos(); // Get the current list of photos (filtered or all)
    int total = photos.size();
    int lastPage = (total + m_pageSize - 1) / m_pageSize - 1;

	if (lastPage < 0 || m_currentPage == lastPage) // already on last page
        return;

    beginResetModel();
    m_currentPage = lastPage;
    endResetModel();
}


// Const version for read-only access
const QList<Photo>& PhotoTableModel::getActivePhotos() const 
{
	return m_hasFilters ? m_filteredPhotos : m_allPhotos; // Return filtered or all photos based on filter state
}

// Non-const version for modification
QList<Photo>& PhotoTableModel::getActivePhotos() 
{
	return m_hasFilters ? m_filteredPhotos : m_allPhotos; // Return filtered or all photos based on filter state
}

// --- Convert visible row index to real index in active photo list ---
int PhotoTableModel::getRealIndex(int row) const 
{
    return m_currentPage * m_pageSize + row; 
}

// --- Check if any filters are active ---
bool PhotoTableModel::hasActiveFilters() const 
{
	// Check if any filter criteria are set
    return (m_filterDateFrom.isValid() && m_filterDateTo.isValid()) ||
        !m_filterTag.isEmpty() ||
        (m_filterMinRating > 0);
}

// --- Check if a photo passes all active filters ---
bool PhotoTableModel::photoPassesFilters(const Photo& photo) const 
{
    // Date filter
    if (m_filterDateFrom.isValid() && m_filterDateTo.isValid()) 
    {
        QDate photoDate = photo.dateTime().date();
		if (photoDate < m_filterDateFrom || photoDate > m_filterDateTo) // outside date range
            return false;
    }

    // Tag filter (case-insensitive substring match)
    if (!m_filterTag.isEmpty()) 
    {
		if (!photo.tag().contains(m_filterTag, Qt::CaseInsensitive))  // tag does not match
            return false;
    }

    // Rating filter
	if (m_filterMinRating > 0 && photo.rating() < m_filterMinRating) // rating too low
        return false;
   

    return true;
}

// --- Get text to display for a cell ---
QVariant PhotoTableModel::getDisplayText(const Photo& photo, int column) const 
{
    switch (column) 
    {
    case Name:     return photo.filePath();
    case Tag:      return photo.tag();
    case Rating:   return formatRatingStars(photo.rating());
    case Comment:  return photo.comment();
    case Size:     return photo.size();
    case DateTime: return photo.dateTime().toString("dd.MM.yyyy hh:mm");
    default:       return QVariant();
    }
}

// --- Get decoration (icon/image) for a cell ---
QVariant PhotoTableModel::getDecoration(const Photo& photo, int column) const 
{
	// Preview column shows the photo thumbnail
    if (column == Preview) 
    {
		// Use edited version if available, otherwise use preview
        QPixmap displayPixmap = photo.hasEditedVersion() 
            ? photo.editedPixmap()
            : photo.preview();

		// If no preview is available, load from file
        if (displayPixmap.isNull()) 
        {
            displayPixmap = QPixmap(photo.filePath()).scaled(
                62, 62, Qt::KeepAspectRatio, Qt::SmoothTransformation
            );
        }
		else  // Scale existing pixmap
        {
            displayPixmap = displayPixmap.scaled(
                62, 62, Qt::KeepAspectRatio, Qt::SmoothTransformation
            );
        }

        return displayPixmap;
    }

	// Actions column shows a standard forward arrow icon
    if (column == Actions) 
    {
        QIcon icon = QApplication::style()->standardIcon(QStyle::SP_ArrowForward);
        return icon.pixmap(25, 25);
    }

    return QVariant();
}

// --- Get tooltip text for a cell ---
QVariant PhotoTableModel::getTooltip(const Photo& photo, int column) const 
{
    switch (column) 
    {
    case Preview:  return QString("Double-click to open photo detail");
    case Name:     return photo.filePath();
    case Tag:      return photo.tag();
    case Rating:   return QString("Enter value from 0 to 5");
    case Comment:  return photo.comment();
    case Size:     return photo.size();
    case DateTime: return photo.dateTime().toString("dd.MM.yyyy hh:mm");
    case Actions:  return QString("Edit photo");
    case Export:   return QString("Check for export");
    default:       return QVariant();
    }
}

// --- Format rating as stars ---
QString PhotoTableModel::formatRatingStars(int rating) const 
{
    QString stars;
    stars.reserve(5);

    for (int i = 0; i < 5; ++i)
		stars += (i < rating) ? STAR_FILLED : STAR_EMPTY; // filled or empty star

    return stars;
}

// --- Update photo field based on column ---
bool PhotoTableModel::updatePhotoField(Photo& photo, int column, const QVariant& value) 
{
    QString filePath = photo.filePath();

    switch (column) 
    {
    case Tag:
        photo.setTag(value.toString()); // update tag in metadata manager inside setter
        return true;

    case Rating: {
        int rating = qBound(0, value.toInt(), 5);
        photo.setRating(rating); // update rating in metadata manager inside setter
        return true;
    }

    case Comment:
		photo.setComment(value.toString()); // update comment in metadata manager inside setter
        return true;

    default:
        return false;
    }
}

// --- Lazy Loading Interface ---
void PhotoTableModel::initializeWithPaths(const QStringList& allPaths) 
{
    beginResetModel();

    // Keep old photos, do not clear them
    int oldSize = m_allPhotos.size(); 
    m_currentPage = 0;

	// Progress dialog setup
    QProgressDialog progress("Loading photos...", "Cancel", 0, allPaths.size());
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setWindowTitle("Initializing Photos");
    progress.setMinimumDuration(0);
    progress.show();

	m_allPhotos.reserve(oldSize + allPaths.size());  // Pre-allocate memory for efficiency

    for (int i = 0; i < allPaths.size(); ++i) 
    {
	    if (progress.wasCanceled()) // User canceled loading
            break;

        m_allPhotos.append(Photo(allPaths[i]));  // Append new Photo with only the file path (no heavy data yet)
      
        // Update progress
        progress.setValue(i + 1);
        QCoreApplication::processEvents(); // refresh GUI
    }

    endResetModel();

    if (hasActiveFilters()) {
        applyFilters();
    }

    progress.close();
   
}

// --- Get pointer to Photo at given row ---
Photo * PhotoTableModel::getPhotoPointer(int row) 
{
	QList<Photo>& photos = getActivePhotos(); // Get the current list of photos (filtered or all)
    int realIndex = getRealIndex(row);

    if (realIndex < 0 || realIndex >= photos.size())
        return nullptr;

    return &photos[realIndex];
}

// --- Get list of photos marked for export ---
QList<Photo*> PhotoTableModel::getPhotosMarkedForExport() 
{
    QList<Photo*> marked;
    QList<Photo>& photos = getActivePhotos();

    for (Photo& photo : photos) 
    {
        if (photo.isMarkedForExport())
            marked.append(&photo);
    }

    return marked;
}

// --- Load saved settings ---
void PhotoTableModel::loadSettings()
{
    QSettings settings("TssApp", "PhotoViewer");

	// load page size
    int savedPageSize = settings.value("table/pageSize", 10).toInt();
    if (savedPageSize > 0) {
        setPageSize(savedPageSize);
    }

	// load sorting
    m_sortColumn = settings.value("table/sortColumn", DateTime).toInt();
    m_sortOrder = static_cast<Qt::SortOrder>(
        settings.value("table/sortOrder", Qt::DescendingOrder).toInt()
        );

	// Apply sorting
    if (!m_allPhotos.isEmpty()) {
        sort(m_sortColumn, m_sortOrder);
    }

	// load filters
    bool hasDateFilter = settings.value("filters/hasDateFilter", false).toBool();
    if (hasDateFilter) 
    {
        QDate from = settings.value("filters/dateFrom").toDate();
        QDate to = settings.value("filters/dateTo").toDate();
        if (from.isValid() && to.isValid())
        {
            setDateFilter(from, to);
        }
    }
	// Load tag filter
    QString savedTag = settings.value("filters/tag", "").toString();
    if (!savedTag.isEmpty()) {
        setTagFilter(savedTag);
    }

	// Load minimum rating filter
    int savedMinRating = settings.value("filters/minRating", 0).toInt();
    if (savedMinRating > 0) {
        setRatingFilter(savedMinRating);
    }
}

// --- Save current settings ---
void PhotoTableModel::saveSettings()
{
    QSettings settings("TssApp", "PhotoViewer");

	// save page size
    settings.setValue("table/pageSize", m_pageSize);

	// save sorting
    settings.setValue("table/sortColumn", m_sortColumn);
    settings.setValue("table/sortOrder", static_cast<int>(m_sortOrder));

	// save filters
    settings.setValue("filters/hasDateFilter",
        m_filterDateFrom.isValid() && m_filterDateTo.isValid());
    settings.setValue("filters/dateFrom", m_filterDateFrom);
    settings.setValue("filters/dateTo", m_filterDateTo);
    settings.setValue("filters/tag", m_filterTag);
    settings.setValue("filters/minRating", m_filterMinRating);
}