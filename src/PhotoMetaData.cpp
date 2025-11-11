#include "PhotoMetadata.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>


// -------------------------
//   PhotoData
// -------------------------

QJsonObject PhotoData::toJson() const 
{
	// Serialize PhotoData to JSON object
    return 
    {
        {"filePath", filePath},
        {"tag", tag},
        {"rating", rating},
        {"comment", comment}
    };
}

PhotoData PhotoData::fromJson(const QJsonObject& json) 
{
	// Deserialize PhotoData from JSON object
    return 
    {
        json["filePath"].toString(),
        json["tag"].toString(),
        json["rating"].toInt(),
        json["comment"].toString()
    };
}

// -------------------------
//   PhotoMetadataManager
// -------------------------

PhotoMetadataManager& PhotoMetadataManager::instance() 
{
	static PhotoMetadataManager instance; // Singleton instance
    return instance;
}

PhotoMetadataManager::PhotoMetadataManager() 
{
	loadFromFile(); // Load metadata at construction
}

PhotoMetadataManager::~PhotoMetadataManager() 
{
	saveToFile(); // Save metadata at destruction
}

/**
 * @brief Default location for metadata storage.
 * Uses platform-specific AppData folder.
 */
QString PhotoMetadataManager::defaultFilePath() const 
{
	const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); // e.g., %APPDATA%/PhotoManager on Windows
    QDir().mkpath(basePath);
    return basePath + "/photo_metadata.json";
}

/**
 * @brief Load all photo metadata from disk.
 */
bool PhotoMetadataManager::loadFromFile(const QString& filePath) 
{
    const QString path = filePath.isEmpty() ? defaultFilePath() : filePath;
    m_currentFilePath = path;

    QFile file(path);
    if (!file.exists()) // No metadata yet
        return true;

	if (!file.open(QIODevice::ReadOnly)) // Cannot open file
        return false;

	const QJsonDocument doc = QJsonDocument::fromJson(file.readAll()); // Parse JSON
    file.close();

	if (!doc.isObject()) // Invalid format
        return false;

	m_metadata.clear(); // Clear existing metadata
	for (const auto& val : doc.object()["photos"].toArray()) // Load each photo entry
    {
        const PhotoData data = PhotoData::fromJson(val.toObject());
		m_metadata.insert(QFileInfo(data.filePath).absoluteFilePath(), data); // Use absolute path as key
    }
    return true;
}

/**
 * @brief Save all photo metadata to JSON file.
 */
bool PhotoMetadataManager::saveToFile(const QString& filePath) 
{
    QString path = filePath.isEmpty() ? m_currentFilePath : filePath;
    if (path.isEmpty())
        path = defaultFilePath();

	cleanupNonExistentFiles(); // if files were deleted, remove their metadata

    QJsonArray photoArray;
	for (const auto& data : m_metadata) // Serialize each PhotoData
        photoArray.append(data.toJson());

    const QJsonObject root{
        {"photos", photoArray}
    };

    QFile file(path);
	if (!file.open(QIODevice::WriteOnly)) // Cannot open file for writing
        return false;

	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)); // Write JSON to file, indented for readability
    return true;
}

PhotoData PhotoMetadataManager::getPhotoData(const QString& filePath) const 
{
	const QString key = QFileInfo(filePath).absoluteFilePath(); // Use absolute path as key
    return m_metadata.value(key, PhotoData{ key }); 
}

void PhotoMetadataManager::setRating(const QString& filePath, int rating) 
{
	const QString key = QFileInfo(filePath).absoluteFilePath(); // Use absolute path as key
    PhotoData data = getPhotoData(key);
    data.rating = qBound(0, rating, 5); // Clamp rating between 0 and 5
    m_metadata[key] = data;
}

void PhotoMetadataManager::setTag(const QString& filePath, const QString& tag) 
{
	const QString key = QFileInfo(filePath).absoluteFilePath(); // Use absolute path as key
    PhotoData data = getPhotoData(key);
    data.tag = tag;
    m_metadata[key] = data;
}

void PhotoMetadataManager::setComment(const QString& filePath, const QString& comment) 
{
	const QString key = QFileInfo(filePath).absoluteFilePath(); // Use absolute path as key
    PhotoData data = getPhotoData(key);
    data.comment = comment;
    m_metadata[key] = data;
}

/**
 * @brief Removes entries for photos that no longer exist on disk.
 */
void PhotoMetadataManager::cleanupNonExistentFiles() 
{
	for (auto it = m_metadata.begin(); it != m_metadata.end();) // Iterate through metadata entries
		it = QFileInfo::exists(it.key()) ? ++it : m_metadata.erase(it); // Remove if file does not exist
}
