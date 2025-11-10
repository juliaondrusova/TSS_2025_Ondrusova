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
 * Features:
 * - Basic adjustments: brightness, contrast, saturation
 * - Rotation: 90° increments left/right
 * - Crop tool with visual selection
 * - Preset filters: grayscale, sepia, negative, pastel, vintage
 * - Watermark overlay with position and opacity control
 */
class PhotoEditorDialog : public QDialog {
    Q_OBJECT

public:
    PhotoEditorDialog(Photo* photo, QWidget* parent = nullptr);
signals:
    /**
     * @brief Emitted when photo editing is confirmed
     * @param photo Original photo object
     */
    void photoEdited(const Photo& photo);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    // Basic operations
    void rotateLeft();
    void rotateRight();
    void cropClicked(bool checked);
    void updatePreview();

    // Actions
    void applyChanges();
    void resetChanges();

    // Effects
    void applyFilter(int filterIndex);
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

    // Signal connections
    void connectSignals();
    void connectSliderWithSpinbox(QSlider* slider, QSpinBox* spinbox, int& value);

    // Image processing
    void applyCrop();
    void applyRotation(QImage& image);
    void applyContrast(QImage& image);
    void applySaturation(QImage& image);
    void applyBrightness(QImage& image);
    void applyActiveFilter(QImage& image);
    void applyWatermark(QImage& image);
    void displayScaledPreview();

    // Filters
    void applyGrayscaleFilter(QImage& image, QProgressDialog* progress = nullptr);
    void applySepiaFilter(QImage& image, QProgressDialog* progress = nullptr);
    void applyNegativeFilter(QImage& image, QProgressDialog* progress = nullptr);
    void applyPastelFilter(QImage& image, QProgressDialog* progress = nullptr);
    void applyVintageFilter(QImage& image, QProgressDialog* progress = nullptr);

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

    // Crop tool
    bool m_cropMode;
    QRubberBand* m_rubberBand;
    QPoint m_cropOrigin;
    QRect m_cropRect;

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
};