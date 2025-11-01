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
    : m_filePath(path), m_rating(0)
{
    if (path.isEmpty())
        return; // Default-constructed Photo (no file path)

    QFileInfo info(path);

    // --- File size calculation (human-readable) ---
    const qint64 sizeBytes = info.size();
    if (sizeBytes < ONE_MB) {
        const double kb = sizeBytes / static_cast<double>(ONE_KB);
        m_size = QString::number(kb, 'f', 1) + " KB";
    }
    else {
        const double mb = sizeBytes / static_cast<double>(ONE_MB);
        m_size = QString::number(mb, 'f', 1) + " MB";
    }

    // Store file modification time
    m_dateTime = info.lastModified();

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

void Photo::setTag(const QString& tag) {
    m_tag = tag;
    PhotoMetadataManager::instance().setTag(m_filePath, tag);
}

void Photo::setRating(int rating) {
    m_rating = rating;
    PhotoMetadataManager::instance().setRating(m_filePath, rating);
}

void Photo::setComment(const QString& comment) {
    m_comment = comment;
    PhotoMetadataManager::instance().setComment(m_filePath, comment);
}

// --- Preview management ---

/**
 * @brief Returns a cached preview image or generates one on demand.
 */
QPixmap Photo::preview() const {
    if (m_preview.isNull()) {
        const_cast<Photo*>(this)->generatePreview();
    }
    return m_preview;
}

/**
 * @brief Generates a scaled thumbnail while preserving the aspect ratio.
 */
void Photo::generatePreview(int size) {
    QImage img(m_filePath);
    if (img.isNull())
        return; // Skip invalid image files

    const QImage scaled = img.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_preview = QPixmap::fromImage(scaled);
}
