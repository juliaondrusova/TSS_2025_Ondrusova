#include "Photo.h"
#include "PhotoMetadata.h"
#include <QFileInfo>
#include <QImage>

// Constants for file size calculation
static const qint64 ONE_KB = 1024;
static const qint64 ONE_MB = 1024 * 1024;

/**
 * @brief Constructs a Photo object, loads file info, and reads stored metadata.
 */
Photo::Photo(const QString& path)
    : m_filePath(path),
      m_rating(0), 
      m_hasEditedVersion(false), 
      m_markedForExport(false)
{
    if (path.isEmpty())
        return; // Default-constructed Photo (no file path)

    QFileInfo info(path);

    // --- File size calculation (human-readable) ---
    const qint64 sizeBytes = info.size();
    m_sizeBytes = sizeBytes;
	if (sizeBytes < ONE_KB) { // Bytes
        m_size = QString::number(sizeBytes) + " B";
    }
	else if (sizeBytes < ONE_MB) { // Kilobytes
        double kb = sizeBytes / static_cast<double>(ONE_KB);
        m_size = QString::number(kb, 'f', 1) + " KB";
    }
	else { // Megabytes
        double mb = sizeBytes / static_cast<double>(ONE_MB);
        m_size = QString::number(mb, 'f', 1) + " MB";
    }

    m_dateTime = info.lastModified(); // Store file modification time

    // Normalize path for consistent metadata lookup
    QString normalizedPath = info.canonicalFilePath();
    if (normalizedPath.isEmpty())
        normalizedPath = info.absoluteFilePath();

    m_filePath = normalizedPath;

    // --- Load metadata from JSON ---
    const PhotoData data = PhotoMetadataManager::instance().getPhotoData(m_filePath);
    m_tag = data.tag;
    m_rating = data.rating;
    m_comment = data.comment;
}

// --- Metadata-modifying setters ---

void Photo::setTag(const QString& tag) 
{
    m_tag = tag;
	PhotoMetadataManager::instance().setTag(m_filePath, tag); // Save to metadata manager
}

void Photo::setRating(int rating) 
{
    m_rating = rating;
	PhotoMetadataManager::instance().setRating(m_filePath, rating); // Save to metadata manager
}

void Photo::setComment(const QString& comment) {
    m_comment = comment;
	PhotoMetadataManager::instance().setComment(m_filePath, comment); // Save to metadata manager
}

// --- Preview management ---

/**
 * @brief Returns a cached preview image or generates one on demand.
 */
QPixmap Photo::preview() const 
{
    if (m_preview.isNull())
		const_cast<Photo*>(this)->generatePreview(); // Generate preview if not already done
    return m_preview;
}

/**
 * @brief Generates a scaled thumbnail while preserving the aspect ratio.
 */
void Photo::generatePreview(int size) 
{
    QImage img(m_filePath);
	if (img.isNull()) // Failed to load image
        return;

	const QImage scaled = img.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation); // Scale image
	m_preview = QPixmap::fromImage(scaled); // Store as QPixmap
}

void Photo::setEditedPixmap(const QPixmap& pixmap) 
{
    m_editedPixmap = pixmap;
	m_hasEditedVersion = !pixmap.isNull(); // Mark that an edited version exists

    if (m_hasEditedVersion)
        m_markedForExport = true;
}

void Photo::clearEditedVersion() 
{
    m_editedPixmap = QPixmap();
	m_hasEditedVersion = false; // No edited version
	m_markedForExport = false; // Clear export mark
}

void Photo::setMarkedForExport(bool marked) {
    m_markedForExport = marked;
}

bool Photo::isMarkedForExport() const {
    return m_markedForExport;
}


QPixmap Photo::getDisplayPixmap() const 
{
	return m_hasEditedVersion ? m_editedPixmap : QPixmap(m_filePath); // Return edited or original
}
