#pragma once
#include <QString>
#include <QPixmap>
#include <QDateTime>

/**
 * @brief Represents a single photo and its associated metadata.
 *
 * This class encapsulates information such as file path, tag, rating, comment,
 * file size, and modification date. It also supports lazy-loaded preview generation.
 */
class Photo {
public:
    /**
     * @brief Constructs a Photo object and loads its metadata if available.
     * @param path Absolute or relative path to the photo file.
     */
    Photo(const QString& path = QString());

    // --- Inline getters ---
    QString filePath() const { return m_filePath; }   ///< Full path to the photo file.
    QString tag() const { return m_tag; }             ///< User-defined tag (category or label).
    int rating() const { return m_rating; }           ///< Photo rating (0–5).
    QString comment() const { return m_comment; }     ///< User comment or description.
    QString size() const { return m_size; }           ///< Human-readable file size.
    QDateTime dateTime() const { return m_dateTime; } ///< File modification timestamp.

    // --- Inline simple setters ---
    void setFilePath(const QString& path) { m_filePath = path; }
    void setSize(const QString& size) { m_size = size; }
    void setDateTime(const QDateTime& dt) { m_dateTime = dt; }

    // --- Metadata-modifying setters ---
    // These update the JSON metadata via PhotoMetadataManager
    void setTag(const QString& tag);
    void setRating(int rating);
    void setComment(const QString& comment);

    // --- Image preview handling ---
    /**
     * @brief Returns the preview image (lazy-loaded if not yet generated).
     */
    QPixmap preview() const;

    /**
     * @brief Generates a scaled thumbnail preview for the photo.
     * @param size Target width/height (default 68px).
     */
    void generatePreview(int size = 68);

private:
    QString m_filePath;   ///< Absolute path to the photo.
    QString m_tag;        ///< Optional tag (label).
    int m_rating;         ///< Rating from 0 to 5.
    QString m_comment;    ///< Optional user comment.
    QString m_size;       ///< File size as a formatted string (e.g., "2.4 MB").
    QDateTime m_dateTime; ///< Last modification date/time.
    mutable QPixmap m_preview; ///< Cached thumbnail (mutable for lazy loading).
};
