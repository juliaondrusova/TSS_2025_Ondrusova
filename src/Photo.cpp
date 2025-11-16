#include "Photo.h"
#include "PhotoMetadata.h"
#include <QFileInfo>
#include <QImage>

// Constants for file size calculation
static const qint64 ONE_KB = 1024;
static const qint64 ONE_MB = 1024 * 1024;

/**
 * Constructor implementation.
 *
 * Loads file info, calculates human-readable size, normalizes path,
 * and retrieves stored metadata. If the path is empty, creates an
 * empty Photo object.
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

    // File size calculation (human-readable)
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

    // Load metadata from JSON
    const PhotoData data = PhotoMetadataManager::instance().getPhotoData(m_filePath);
    m_tag = data.tag;
    m_rating = data.rating;
    m_comment = data.comment;
}



// --- Preview management ---


/** Returns the cached preview, generates it on demand if missing. */
QPixmap Photo::preview() const 
{
    if (m_preview.isNull())
		const_cast<Photo*>(this)->generatePreview(); // Generate preview if not already done
    return m_preview;
}

/** Generates a scaled thumbnail while keeping the aspect ratio. */
void Photo::generatePreview(int size) 
{
    QImage img(m_filePath);
	if (img.isNull()) // Failed to load image
        return;

	const QImage scaled = img.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation); // Scale image
	m_preview = QPixmap::fromImage(scaled); // Store as QPixmap
}

/** Sets a custom edited version of the photo and marks it for export. */
void Photo::setEditedPixmap(const QPixmap& pixmap) 
{
    m_editedPixmap = pixmap;
	m_hasEditedVersion = !pixmap.isNull(); // Mark that an edited version exists

    if (m_hasEditedVersion)
        m_markedForExport = true;
}

/** Clears any edited version and resets export flag. */
void Photo::clearEditedVersion() 
{
    m_editedPixmap = QPixmap();
	m_hasEditedVersion = false; // No edited version
	m_markedForExport = false; // Clear export mark
}