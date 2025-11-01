#pragma once
#include <QString>
#include <QMap>
#include <QJsonObject>

/**
 * @brief Represents metadata for a single photo.
 *
 * Stores information such as file path, tag, rating, and user comment.
 * Provides JSON serialization and deserialization.
 */
struct PhotoData {
    QString filePath;   ///< Absolute path to the photo file.
    QString tag;        ///< User-defined tag (category or label).
    int rating = 0;     ///< Rating from 0 to 5.
    QString comment;    ///< Optional user comment.

    QJsonObject toJson() const;
    static PhotoData fromJson(const QJsonObject& json);
};

/**
 * @brief Singleton manager for loading, saving, and updating photo metadata.
 *
 * Manages a JSON file that stores metadata for all photos. Automatically
 * loads data at construction and saves it at destruction.
 */
class PhotoMetadataManager {
public:
    static PhotoMetadataManager& instance();

    bool loadFromFile(const QString& filePath = {});
    bool saveToFile(const QString& filePath = {});

    PhotoData getPhotoData(const QString& filePath) const;

    void setRating(const QString& filePath, int rating);
    void setTag(const QString& filePath, const QString& tag);
    void setComment(const QString& filePath, const QString& comment);

    /// Removes entries for files that no longer exist.
    void cleanupNonExistentFiles();

private:
    PhotoMetadataManager();
    ~PhotoMetadataManager();

    Q_DISABLE_COPY(PhotoMetadataManager)

        QString defaultFilePath() const;

    QMap<QString, PhotoData> m_metadata;
    QString m_currentFilePath;
};
