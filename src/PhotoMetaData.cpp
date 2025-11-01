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

QJsonObject PhotoData::toJson() const {
    return {
        {"filePath", filePath},
        {"tag", tag},
        {"rating", rating},
        {"comment", comment}
    };
}

PhotoData PhotoData::fromJson(const QJsonObject& json) {
    return {
        json["filePath"].toString(),
        json["tag"].toString(),
        json["rating"].toInt(),
        json["comment"].toString()
    };
}

// -------------------------
//   PhotoMetadataManager
// -------------------------

PhotoMetadataManager& PhotoMetadataManager::instance() {
    static PhotoMetadataManager instance;
    return instance;
}

PhotoMetadataManager::PhotoMetadataManager() {
    loadFromFile();
}

PhotoMetadataManager::~PhotoMetadataManager() {
    saveToFile();
}

/**
 * @brief Default location for metadata storage.
 * Uses platform-specific AppData folder.
 */
QString PhotoMetadataManager::defaultFilePath() const {
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(basePath);
    return basePath + "/photo_metadata.json";
}

/**
 * @brief Load all photo metadata from disk.
 */
bool PhotoMetadataManager::loadFromFile(const QString& filePath) {
    const QString path = filePath.isEmpty() ? defaultFilePath() : filePath;
    m_currentFilePath = path;

    QFile file(path);
    if (!file.exists())
        return true; // no data yet

    if (!file.open(QIODevice::ReadOnly))
        return false;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return false;

    m_metadata.clear();
    for (const auto& val : doc.object()["photos"].toArray()) {
        const PhotoData data = PhotoData::fromJson(val.toObject());
        m_metadata.insert(QFileInfo(data.filePath).absoluteFilePath(), data);
    }
    return true;
}

/**
 * @brief Save all photo metadata to JSON file.
 */
bool PhotoMetadataManager::saveToFile(const QString& filePath) {
    QString path = filePath.isEmpty() ? m_currentFilePath : filePath;
    if (path.isEmpty())
        path = defaultFilePath();

    cleanupNonExistentFiles();

    QJsonArray photoArray;
    for (const auto& data : m_metadata)
        photoArray.append(data.toJson());

    const QJsonObject root{
        {"version", "1.0"},
        {"photos", photoArray}
    };

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

PhotoData PhotoMetadataManager::getPhotoData(const QString& filePath) const {
    const QString key = QFileInfo(filePath).absoluteFilePath();
    return m_metadata.value(key, PhotoData{ key });
}

void PhotoMetadataManager::setRating(const QString& filePath, int rating) {
    const QString key = QFileInfo(filePath).absoluteFilePath();
    PhotoData data = getPhotoData(key);
    data.rating = qBound(0, rating, 5);
    m_metadata[key] = data;
}

void PhotoMetadataManager::setTag(const QString& filePath, const QString& tag) {
    const QString key = QFileInfo(filePath).absoluteFilePath();
    PhotoData data = getPhotoData(key);
    data.tag = tag;
    m_metadata[key] = data;
}

void PhotoMetadataManager::setComment(const QString& filePath, const QString& comment) {
    const QString key = QFileInfo(filePath).absoluteFilePath();
    PhotoData data = getPhotoData(key);
    data.comment = comment;
    m_metadata[key] = data;
}

/**
 * @brief Removes entries for photos that no longer exist on disk.
 */
void PhotoMetadataManager::cleanupNonExistentFiles() {
    for (auto it = m_metadata.begin(); it != m_metadata.end();)
        it = QFileInfo::exists(it.key()) ? ++it : m_metadata.erase(it);
}
