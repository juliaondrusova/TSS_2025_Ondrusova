#pragma once
#include <QDialog>
#include "Photo.h"

class QLabel;
class QSlider;
class QPushButton;
class QSpinBox;
class QTimer;
class QRubberBand;
class QComboBox;
class QVBoxLayout;
class QProgressDialog;

/**
 * @brief Photo editing dialog with adjustments, filters, and watermarks
 *
 * @details
 * Basic adjustments: brightness, contrast, saturation
 * Rotation: 90° increments left/right
 * Crop tool with visual selection
 * Preset filters: grayscale, sepia, negative, pastel, vintage
 * Watermark overlay with position and opacity control
 */
class PhotoEditorDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor for photo editor dialog.
     * @param photo Pointer to the Photo object to edit.
     * @param parent Parent widget, default is nullptr.
     */
    PhotoEditorDialog(Photo* photo, QWidget* parent = nullptr);
signals:
    /**
    * @brief Signal emitted when photo editing is confirmed.
    * @param photo Edited Photo object.
    * @details Connect this signal to update the main photo library or UI.
    */
    void photoEdited(const Photo& photo);

protected:
    /**
     * @brief Handles mouse press events.
     * @param event Mouse event data.
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse release events.
     * @param event Mouse event data.
     */
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    // Basic operations
    /**
     * @brief Rotates the image 90° to the left.
     * @details Preview and edited image are updated accordingly.
     */
    void rotateLeft();

    /**
     * @brief Rotates the image 90° to the right.
     */
    void rotateRight();

    /**
     * @brief Updates the preview image with current adjustments and filters.
     */
    void updatePreview();

    // Actions
    /**
     * @brief Applies all changes to the photo.
     * @details Updates the Photo object and emits photoEdited signal.
     */
    void applyChanges();

    /**
     * @brief Resets all changes to the original state.
     */
    void resetChanges();

    // Effects
    /**
     * @brief Applies the selected filter to the photo.
     * @param filterIndex Index of the selected filter.
     */
    void applyFilter(int filterIndex);

    /**
     * @brief Adds a watermark to the photo.
     * @details Position and opacity are controlled by the UI.
     */
    void addWatermark();

private:
    // UI setup
    void buildUI();
    void createPreviewArea(QVBoxLayout* layout);
    void createToolbar(QVBoxLayout* layout);
    void createAdjustmentPanel(QVBoxLayout* layout);
    void createFilterPanel(QVBoxLayout* layout);
    void createWatermarkPanel(QVBoxLayout* layout);
    void createActionButtons(QVBoxLayout* layout);
    void createSliderWithSpinbox(const QString& label, QSlider*& slider, QSpinBox*& spinbox, QVBoxLayout* layout);

    void createAdjustmentSlider(const QString& label, QSlider*& slider, QSpinBox*& spinbox, QVBoxLayout* layout);

    bool eventFilter(QObject* obj, QEvent* event);
    void resizeEvent(QResizeEvent* event);

    void connectSignals();
    void connectSliderWithSpinbox(QSlider* slider, QSpinBox* spinbox, int& value);

    // Image processing
    void applyRotation(QImage& image);
    void applyContrast(QImage& image);
    void applySaturation(QImage& image);
    void applyBrightness(QImage& image);
    void applyActiveFilter(QImage& image);
    void applyWatermark(QImage& image);
    void displayScaledPreview();

    // Filters
    void processImagePixels(QImage& image, QProgressDialog* progress, int filterNumber);

    // Watermark positioning
    QPoint calculateWatermarkPosition(const QSize& imageSize, const QSize& watermarkSize);

    // Original photo data
    Photo m_originalPhoto;
    Photo* m_photoPtr;
    QPixmap m_originalPixmap;
    QPixmap m_editedPixmap;

    // UI components
    QLabel* previewLabel;
    QPushButton* cropBtn;
    QPushButton* rotateLeftBtn;
    QPushButton* rotateRightBtn;
    QSlider* brightnessSlider;
    QSlider* contrastSlider;
    QSlider* saturationSlider;
    QSpinBox* brightnessValue;
    QSpinBox* contrastValue;
    QSpinBox* saturationValue;
    QComboBox* filterCombo;
    QPushButton* watermarkBtn;
    QComboBox* watermarkPositionCombo;
    QSlider* watermarkOpacitySlider;
    QPushButton* applyBtn;
    QPushButton* resetBtn;
    QPushButton* cancelBtn;
    QTimer* updateTimer;


    // Adjustment values
    int m_rotation;
    int m_brightness;
    int m_contrast;
    int m_saturation;

    // Filter & watermark
    int m_activeFilter;
    QPixmap m_watermarkPixmap;
    int m_watermarkOpacity;
    int m_watermarkPosition;

    bool m_showingOriginal;


    int m_temperature;
    QSlider* temperatureSlider;
    QSpinBox* temperatureValue;
    void applyTemperature(QImage& image);


    // V private members:
    int m_red;
    int m_green;
    int m_blue;

    QSlider* redSlider;
    QSlider* greenSlider;
    QSlider* blueSlider;

    QSpinBox* redValue;
    QSpinBox* greenValue;
    QSpinBox* blueValue;

    // V private functions:
    void applyRGB(QImage& image);
};