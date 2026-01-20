#include <QtTest>
#include <QTemporaryDir>
#include <QImage>
#include "PhotoTableModel.h"

/**
 * @brief TestTSSAppUnit
 * Unit test for PhotoTableModel - import photos.
 */
class TestTSSAppUnit : public QObject
{
    Q_OBJECT

private slots:
    void testImportPhotos();
};

void TestTSSAppUnit::testImportPhotos()
{
    // Create temporary directory
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Create 3 test photos
    QStringList files;
    for (int i = 0; i < 3; ++i)
    {
        QString filename = QString("%1/photo_%2.jpg")
            .arg(tmpDir.path())
            .arg(i);

        QImage img(100, 100, QImage::Format_RGB32);
        img.fill(Qt::blue);
        img.save(filename, "JPG");
        files << filename;
    }
    // =====================================================
    // FR-1.1 – Import photos from folder or external drive
    // FR-1.2 - The system displays photos in a gallery or list view
    // =====================================================

    // Create model and load photos
    PhotoTableModel model;
    model.initializeWithPaths(files);

    // Verify all photos loaded
    QCOMPARE(model.getActivePhotos().size(), 3);
}

QTEST_MAIN(TestTSSAppUnit)
#include "TestTSSAppUnit.moc"