#include <QtTest>
#include <QApplication>
#include <QTemporaryDir>
#include <QImage>
#include <QPainter>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QModelIndex>
#include "TSS_App.h"
#include "PhotoTableModel.h"

/**
 * @brief TestTSSAppGui
 * Integration test for TSS_App GUI.
 *
 * This test simulates user interactions with the GUI:
 * - Loading photos
 * - Applying and clearing filters via GUI controls
 * - Editing photo tags directly in the table view
 */
class TestTSSAppGui : public QObject
{
    Q_OBJECT

private slots:
    void guiFilterAndEditSimulation();
};

void TestTSSAppGui::guiFilterAndEditSimulation()
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

    for (int i = 0; i < 5; i++)
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
    QCOMPARE(model->getActivePhotos().size(), 5);

    QTest::qWait(2000); // Visual pause to confirm photos are visible

    // --- Simulate entering a filter in the tag line edit ---
    QLineEdit* tagEdit = app.findChild<QLineEdit*>("tagFilterEdit");
    QVERIFY(tagEdit != nullptr);

    QTest::mouseClick(tagEdit, Qt::LeftButton);
    QTest::qWait(1000); // Pause after click
    QTest::keyClicks(tagEdit, "nonexistent"); // Type a tag that matches nothing
    QTest::qWait(2000); // Pause to simulate user reading time

    // --- Click the "Apply Filter" button ---
    QPushButton* btnApply = app.findChild<QPushButton*>("btnApplyFilter");
    QVERIFY(btnApply != nullptr);

    QTest::mouseClick(btnApply, Qt::LeftButton);
    QTest::qWait(2000); // Pause after applying filter

    // Verify that no photos match the filter
    QCOMPARE(model->getActivePhotos().size(), 0);

    // --- Click the "Clear Filter" button ---
    QPushButton* btnClear = app.findChild<QPushButton*>("btnClearFilter");
    QVERIFY(btnClear != nullptr);

    QTest::mouseClick(btnClear, Qt::LeftButton);
    QTest::qWait(2000); // Pause after clearing filter

    // Verify that all photos are visible again
    QCOMPARE(model->getActivePhotos().size(), 5);

    // --- Edit the tag of the first photo directly in the table view ---
    QModelIndex firstIndex = model->index(0, PhotoTableModel::Tag);
    QVERIFY(firstIndex.isValid());

    tableView->edit(firstIndex); // Activate edit mode
    QTest::qWait(1000);          // Pause for edit mode to initialize
    QTest::keyClicks(tableView->focusWidget(), "holiday"); // Enter new tag
    QTest::keyClick(tableView->focusWidget(), Qt::Key_Return); // Confirm edit
    QTest::qWait(2000);          // Pause after editing

    // --- Apply filter again using the GUI ---
    QTest::mouseClick(tagEdit, Qt::LeftButton);
    QTest::qWait(1000);
    tagEdit->clear();
    QTest::keyClicks(tagEdit, "holiday");
    QTest::mouseClick(btnApply, Qt::LeftButton);
    QTest::qWait(2000); // Pause after applying filter

    // Verify that exactly one photo matches the filter
    QCOMPARE(model->getActivePhotos().size(), 1);

    QTest::qWait(3000); // Final visual pause before closing
    app.close();
}

// Macro to run the test
QTEST_MAIN(TestTSSAppGui)

// MOC include must be at the end of the file
#include "TestTSSAppGUI.moc"
