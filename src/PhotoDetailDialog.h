#pragma once
#include <QDialog>
#include <QShowEvent>
#include <QWheelEvent>
#include "Photo.h"

class QLabel;
class QScrollArea;
class QSlider;
class QPushButton;

/**
 * @class PhotoDetailDialog
 * @brief Dialog window for displaying detailed photo view.
 *
 * @details
 * The PhotoDetailDialog class provides a popup window to show a
 * single photo in detail. It supports zooming via slider or mouse
 * wheel, toggling fullscreen mode, and scrolling if the image
 * exceeds the window size. The dialog is designed for use in
 * photo management applications.
 *
 * @see Photo
 */
class PhotoDetailDialog : public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructs the PhotoDetailDialog.
     * @param parent Parent QWidget (optional).
     *
     * Initializes UI components such as image label, scroll area,
     * zoom slider, and fullscreen button.
     */
    PhotoDetailDialog(QWidget* parent = nullptr);

    /**
     * @brief Sets the photo to be displayed in the dialog.
     * @param photo Photo object containing image and metadata.
     *
     * Updates the imageLabel and internal data for display.
     */
    void setPhoto(const Photo& photo);

protected:
    /**
     * @brief Handles the widget show event.
     * @param event Show event pointer.
     *
     * Used to perform initialization or adjust UI when dialog
     * becomes visible.
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief Handles mouse wheel events for zooming.
     * @param event Wheel event pointer.
     *
     * Allows users to zoom in/out using the mouse wheel.
     */
    void wheelEvent(QWheelEvent* event) override;

private slots:
    /**
     * @brief Slot called when zoom slider value changes.
     * @param value Zoom level (typically percentage).
     *
     * Updates image scaling in the scroll area.
     */
    void zoomChanged(int value);

    /**
     * @brief Toggles fullscreen mode on/off.
     *
     * Adjusts window flags and resizes the dialog accordingly.
     */
    void toggleFullscreen();

private:
    QLabel* imageLabel;       ///< Label widget displaying the image.
    QScrollArea* scrollArea;  ///< Scroll area for large images.
    QSlider* zoomSlider;      ///< Slider controlling zoom level.
    QPushButton* fullscreenBtn; ///< Button to toggle fullscreen mode.
    QPixmap originalPixmap;   ///< Original photo pixmap for scaling.
    Photo currentPhoto;       ///< Photo currently displayed.
    bool isFullscreen;        ///< Tracks fullscreen state of the dialog.
};
