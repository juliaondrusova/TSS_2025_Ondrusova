#include <QtTest>
#include <QApplication>
#include <QTemporaryDir>
#include <QImage>
#include <QPainter>
#include <QLineEdit>
#include <QTableView>
#include "TSS_App.h"
#include "PhotoTableModel.h"

/**
 * @brief TestTSSAppIntegration
 * Integration test for TSS_App.
 *
 * This test visually launches the TSS_App window, loads test photos into the table view,
 * applies filters through the GUI, and verifies that the model correctly updates.
 */
class TestTSSAppIntegration : public QObject
{
    Q_OBJECT

private slots:
    void filterPhotosWithWindowVisible();
};

void TestTSSAppIntegration::filterPhotosWithWindowVisible()
{
    // --- Launch the application ---
    TSS_App app;
    app.show(); // Show main window
    QVERIFY(QTest::qWaitForWindowExposed(&app)); // Wait until window is fully visible
    QTest::qWait(2000); // Visual pause to see the GUI

    // --- Create temporary test photos ---
    QTemporaryDir tmpDir;        // Temporary directory for storing photos
    QVERIFY(tmpDir.isValid());   // Ensure temp directory was created

    QStringList files;           // List to store paths to all test photos

    for (int i = 0; i < 10; ++i)
    {
        // Generate file name like "C:/Temp/photo_xxx.jpg"
        QString filename = QString("%1/photo_%2.jpg")
            .arg(tmpDir.path())
            .arg(i, 3, 10, QChar('0'));

        // Create a simple colored image with text
        QImage img(200, 200, QImage::Format_RGB32);   // Image 200x200
        img.fill(Qt::red);                            // Plain red background
        QPainter(&img).drawText(img.rect(), Qt::AlignCenter, QString("Photo %1").arg(i + 1)); // Add text
        img.save(filename, "JPG");                   // Save to disk

        files << filename;                            // Add file path to list
    }

    // --- Initialize the model through the table view ---
    QTableView* tableView = app.findChild<QTableView*>("tableView");
    QVERIFY(tableView != nullptr);                   // Ensure tableView exists

    PhotoTableModel* model = qobject_cast<PhotoTableModel*>(tableView->model());
    QVERIFY(model != nullptr);                       // Ensure model exists

    // Load photos into model
    model->initializeWithPaths(files);
    QCoreApplication::processEvents();

    // Ensure no filters active initially
    model->clearFilters();
    QCoreApplication::processEvents();

    // Verify all photos are loaded
    QCOMPARE(model->getActivePhotos().size(), 10);

    QTest::qWait(2000); // Visual pause to confirm photos are visible

    // --- Simulate GUI filter input ---
    QLineEdit* tagEdit = app.findChild<QLineEdit*>("tagFilterEdit");
    QVERIFY(tagEdit != nullptr); // Ensure line edit exists

    tagEdit->setText("nonexistent"); // Enter a tag that matches nothing
    QTest::qWait(1000);              // Small pause

    // Apply filter via model
    model->setTagFilter(tagEdit->text());
    QTest::qWait(1000);

    // Verify that no photos pass the filter
    QCOMPARE(model->getActivePhotos().size(), 0);
    QTest::qWait(2000); // Visual confirmation

    // --- Clear the filter ---
    model->setTagFilter("");
    QTest::qWait(1000);

    // Verify that all photos are visible again
    QCOMPARE(model->getActivePhotos().size(), 10);

    // Final visual pause before closing application
    QTest::qWait(3000);
    app.close();
}

// Macro to run the test
QTEST_MAIN(TestTSSAppIntegration)

// MOC include must be at the end of the file
#include "TestTSSAppIntegration.moc"
