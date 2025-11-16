#pragma once
#include <QString>
#include <QMap>
#include <QJsonObject>

/**
 * @struct PhotoData
 * @brief Represents metadata for a single photo.
 *
 * @details Stores information about a photo including its file path,
 * user-assigned tag, rating, and optional comment. Provides methods
 * for JSON serialization and deserialization.
 *
 * @see PhotoMetadataManager
 */
struct PhotoData {
    QString filePath;   ///< Absolute path to the photo file.
    QString tag;        ///< User-defined tag (category or label).
    int rating = 0;     ///< Rating from 0 to 5.
    QString comment;    ///< Optional user comment.

    /**
     * @brief Serializes the photo data to a QJsonObject.
     * @return QJsonObject containing the photo metadata.
     */
    QJsonObject toJson() const;

    /**
     * @brief Constructs a PhotoData object from a QJsonObject.
     * @param json The JSON object containing photo metadata.
     * @return PhotoData instance initialized from JSON.
     */
    static PhotoData fromJson(const QJsonObject& json);
};

/**
 * @class PhotoMetadataManager
 * @brief Singleton manager for photo metadata.
 *
 * @details Manages a collection of PhotoData for multiple photos.
 * Handles loading and saving metadata from/to a JSON file. 
 *
 * @see PhotoData
 */
class PhotoMetadataManager {
public:
    /**
     * @brief Returns the singleton instance.
     * @return Reference to the PhotoMetadataManager singleton.
     */
    static PhotoMetadataManager& instance();

    /**
     * @brief Loads metadata from a JSON file.
     * @param filePath Optional path to the JSON file. Uses default path if empty.
     * @return True if loading succeeds, false otherwise.
     */
    bool loadFromFile(const QString& filePath = {});

    /**
     * @brief Saves current metadata to a JSON file.
     * @param filePath Optional path to the JSON file. Uses default path if empty.
     * @return True if saving succeeds, false otherwise.
     */
    bool saveToFile(const QString& filePath = {});

    /**
     * @brief Retrieves metadata for a given photo.
     * @param filePath Absolute path to the photo file.
     * @return PhotoData for the specified file. Returns default PhotoData if not found.
     */
    PhotoData getPhotoData(const QString& filePath) const;

    /**
     * @brief Sets the rating for a specific photo.
     * @param filePath Absolute path to the photo file.
     * @param rating Rating value from 0 to 5.
     */
    void setRating(const QString& filePath, int rating);

    /**
     * @brief Sets the tag for a specific photo.
     * @param filePath Absolute path to the photo file.
     * @param tag User-defined tag string.
     */
    void setTag(const QString& filePath, const QString& tag);

    /**
     * @brief Sets the comment for a specific photo.
     * @param filePath Absolute path to the photo file.
     * @param comment User-defined comment string.
     */
    void setComment(const QString& filePath, const QString& comment);

    /**
     * @brief Removes metadata entries for files that no longer exist on disk.
     *
     * @details Iterates through all metadata entries and removes
     * those whose filePath does not point to an existing file.
     */
    void cleanupNonExistentFiles();

private:
    PhotoMetadataManager();
    ~PhotoMetadataManager();

    Q_DISABLE_COPY(PhotoMetadataManager)

    /**
    * @brief Returns the default file path used for storing metadata.
    * @return QString containing the default JSON file path.
    */
    QString defaultFilePath() const;

    QMap<QString, PhotoData> m_metadata; ///< Map of file paths to photo metadata.
    QString m_currentFilePath;           ///< Current path to the JSON metadata file.
};
