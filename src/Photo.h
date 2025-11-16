#pragma once
#include <QString>
#include <QPixmap>
#include <QDateTime>
#include "PhotoMetadata.h"

/**
 * @class Photo
 * @brief Represents a single photo and its associated metadata.
 *
 * @details
 * The Photo class encapsulates information about a photo file, including
 * its path, tag, rating, comment, file size, modification date, and preview image.
 * It also supports lazy-loaded preview generation and edited versions of the photo.
 *
 * This class is intended for use in photo management applications where
 * displaying, editing, tagging, and rating images is required.
 *
 * @see QPixmap, QString, QDateTime
 */
class Photo {
public:
    /**
     * @brief Constructs a Photo object.
     * @param path Absolute or relative path to the photo file.
     *
     * @details
     * Initializes a Photo instance. If a path is provided, the object can
     * load data such as file size, modification date, and generate a cached preview.
     *
     * @note Calling this constructor does not automatically load the image data;
     * use generatePreview() to create a thumbnail.
     */
    Photo(const QString& path = QString());

    // --- Inline getters ---

    /**
     * @brief Returns the full file path of the photo.
     * @return Full path to the photo file.
     *
     * @details
     * This is the path used to locate the photo on disk.
     */
    QString filePath() const { return m_filePath; }

    /**
     * @brief Returns the user-defined tag for the photo.
     * @return Tag string.
     *
     * @details
     * Tags are used to categorize photos (e.g., "Vacation", "Family").
     * This does not modify the underlying file.
     */
    QString tag() const { return m_tag; }

    /**
     * @brief Returns the photo rating.
     * @return Integer rating from 0 (unrated) to 5 (highest rating).
     */
    int rating() const { return m_rating; }

    /**
     * @brief Returns the user comment for the photo.
     * @return Comment string.
     *
     * @details
     * This can include any description or note about the photo.
     */
    QString comment() const { return m_comment; }

    /**
     * @brief Returns the human-readable file size.
     * @return File size as a string (e.g., "2.4 MB").
     */
    QString size() const { return m_size; }

    /**
     * @brief Returns the last modification date/time of the photo file.
     * @return QDateTime object representing file modification timestamp.
     */
    QDateTime dateTime() const { return m_dateTime; }

    /**
     * @brief Returns the file size in bytes.
     * @return Exact file size in bytes.
     */
    qint64 sizeBytes() const { return m_sizeBytes; }

    // --- Inline simple setters ---

    /**
     * @brief Sets the photo file path.
     * @param path Absolute or relative file path.
     */
    void setFilePath(const QString& path) { m_filePath = path; }

    /**
     * @brief Sets the formatted file size string.
     * @param size Human-readable size (e.g., "3.1 MB").
     */
    void setSize(const QString& size) { m_size = size; }

    /**
     * @brief Sets the last modification date/time.
     * @param dt Date and time of last modification.
     */
    void setDateTime(const QDateTime& dt) { m_dateTime = dt; }

    // --- Metadata-modifying setters ---

    /**
     * @brief Sets the photo tag and updates metadata storage.
     * @param tag User-defined category label.
     *
     * @see tag(), PhotoMetadataManager
     */
    void setTag(const QString& tag)
    {
        m_tag = tag;
        PhotoMetadataManager::instance().setTag(m_filePath, tag); // Save to metadata manager
    }

    /**
     * @brief Sets the photo rating (0–5) and updates metadata.
     * @param rating Rating value from 0 to 5.
     */
    void setRating(int rating)
    {
        m_rating = rating;
        PhotoMetadataManager::instance().setRating(m_filePath, rating); // Save to metadata manager
    }

    /**
     * @brief Sets the user comment and updates metadata.
     * @param comment Description or note for the photo.
     */
    void setComment(const QString& comment) 
    {
        m_comment = comment;
        PhotoMetadataManager::instance().setComment(m_filePath, comment); // Save to metadata manager
    }

    // --- Image preview handling ---

    /**
     * @brief Returns the preview image.
     * @return Cached QPixmap thumbnail of the photo.
     *
     * @details
     * If the preview has not been generated yet, it will be created
     * on-demand (lazy-loaded).
     */
    QPixmap preview() const;

    /**
     * @brief Generates a scaled thumbnail preview.
     * @param size Target width and height in pixels (default 90).
     *
     * @details
     * The thumbnail is cached internally and can be retrieved using preview().
     * This function does not modify the original image file.
     */
    void generatePreview(int size = 90);

    /**
     * @brief Sets a custom edited version of the photo.
     * @param pixmap Edited image.
     *
     * @note Use hasEditedVersion() to check if an edited version exists.
     */
    void setEditedPixmap(const QPixmap& pixmap);

    /**
     * @brief Returns the edited photo if available.
     * @return Edited QPixmap.
     */
    QPixmap editedPixmap() const { return m_editedPixmap; }

    /**
     * @brief Checks whether an edited version exists.
     * @return True if edited version is available, false otherwise.
     */
    bool hasEditedVersion() const { return m_hasEditedVersion; }

    /**
     * @brief Clears the edited version of the photo.
     */
    void clearEditedVersion();

    /**
     * @brief Marks or unmarks the photo for export.
     * @param marked True to mark for export, false to unmark.
     */
    void setMarkedForExport(bool marked) { m_markedForExport = marked; }

    /**
     * @brief Checks if the photo is marked for export.
     * @return True if marked for export, false otherwise.
     */
    bool isMarkedForExport() const { return m_markedForExport; }

    /**
     * @brief Returns the photo to display.
     * @return Edited photo if available, otherwise original.
     *
     * @details
     * Helper method to decide which image version should be shown
     * in the UI.
     */
    QPixmap getDisplayPixmap() const { return m_hasEditedVersion ? m_editedPixmap : QPixmap(m_filePath); }

private:
    QString m_filePath;         ///< Absolute path to the photo.
    QString m_tag;              ///< Optional tag (label).
    int m_rating;               ///< Rating from 0 to 5.
    QString m_comment;          ///< Optional user comment.
    QString m_size;             ///< File size as formatted string (e.g., "2.4 MB").
    qint64 m_sizeBytes;         ///< File size in bytes.
    QDateTime m_dateTime;       ///< Last modification date/time.
    mutable QPixmap m_preview;  ///< Cached thumbnail (mutable for lazy loading).
    QPixmap m_editedPixmap;     ///< Edited version of the photo.
    bool m_hasEditedVersion;    ///< True if edited version exists.
    bool m_markedForExport;     ///< True if marked for export.
};
