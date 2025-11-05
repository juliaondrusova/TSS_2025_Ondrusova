#include "PhotoEditDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QTimer>
#include <QRubberBand>
#include <QMouseEvent>
#include <QFileDialog>
#include <QComboBox>
#include <QPainter>
#include <QProgressDialog>
#include <QApplication>

// Constants
namespace {
    constexpr int MIN_ADJUSTMENT = -100;
    constexpr int MAX_ADJUSTMENT = 100;
    constexpr int DEFAULT_ADJUSTMENT = 0;
    constexpr int TIMER_DELAY_MS = 50;
    constexpr int DEFAULT_WATERMARK_OPACITY = 70;
    constexpr int DEFAULT_WATERMARK_POSITION = 3; // Bottom Right
    constexpr int WATERMARK_MARGIN = 20;
    constexpr int PROGRESS_THRESHOLD_PIXELS = 1000000; // 1 megapixel
}

// Constructor
PhotoEditorDialog::PhotoEditorDialog(const Photo& photo, QWidget* parent)
    : QDialog(parent),
    m_originalPhoto(photo),
    m_rotation(0),
    m_brightness(0),
    m_contrast(0),
    m_saturation(0),
    m_cropMode(false),
    m_activeFilter(0),
    m_watermarkOpacity(DEFAULT_WATERMARK_OPACITY),
    m_watermarkPosition(DEFAULT_WATERMARK_POSITION)
{
    setWindowTitle("Photo Editor");
    resize(900, 700);

    m_originalPixmap = QPixmap(photo.filePath());
    m_editedPixmap = m_originalPixmap;

    buildUI();
    connectSignals();
    updatePreview();
}

// --- UI Construction ---

void PhotoEditorDialog::buildUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    createPreviewArea(mainLayout);
    createToolbar(mainLayout);

    // Two-column layout for adjustments and filters
    QHBoxLayout* middleLayout = new QHBoxLayout();

    QVBoxLayout* leftColumn = new QVBoxLayout();
    createAdjustmentPanel(leftColumn);
    middleLayout->addLayout(leftColumn);

    QVBoxLayout* rightColumn = new QVBoxLayout();
    createFilterPanel(rightColumn);
    createWatermarkPanel(rightColumn);
    middleLayout->addLayout(rightColumn);

    mainLayout->addLayout(middleLayout);
    createActionButtons(mainLayout);
}

void PhotoEditorDialog::createPreviewArea(QVBoxLayout* layout) {
    previewLabel = new QLabel(this);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(600, 400);
    previewLabel->setScaledContents(false);
    previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    previewLabel->setMouseTracking(true);
    layout->addWidget(previewLabel);
}

void PhotoEditorDialog::createToolbar(QVBoxLayout* layout) {
    QHBoxLayout* toolbar = new QHBoxLayout();

    cropBtn = new QPushButton("Crop", this);
    cropBtn->setCheckable(true);
    rotateLeftBtn = new QPushButton("Rotate Left", this);
    rotateRightBtn = new QPushButton("Rotate Right", this);

    toolbar->addWidget(cropBtn);
    toolbar->addWidget(rotateLeftBtn);
    toolbar->addWidget(rotateRightBtn);
    toolbar->addStretch();

    layout->addLayout(toolbar);
}

void PhotoEditorDialog::createAdjustmentPanel(QVBoxLayout* layout) {
    QGroupBox* group = new QGroupBox("Adjustments", this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    createSliderWithSpinbox("Brightness:", brightnessSlider, brightnessValue, groupLayout);
    createSliderWithSpinbox("Contrast:", contrastSlider, contrastValue, groupLayout);
    createSliderWithSpinbox("Saturation:", saturationSlider, saturationValue, groupLayout);

    layout->addWidget(group);
}

void PhotoEditorDialog::createSliderWithSpinbox(const QString& label, QSlider*& slider,
    QSpinBox*& spinbox, QVBoxLayout* layout) {
    QHBoxLayout* row = new QHBoxLayout();
    row->addWidget(new QLabel(label, this));

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(MIN_ADJUSTMENT, MAX_ADJUSTMENT);
    slider->setValue(DEFAULT_ADJUSTMENT);

    spinbox = new QSpinBox(this);
    spinbox->setRange(MIN_ADJUSTMENT, MAX_ADJUSTMENT);
    spinbox->setValue(DEFAULT_ADJUSTMENT);

    row->addWidget(slider, 3);
    row->addWidget(spinbox, 1);
    layout->addLayout(row);
}

void PhotoEditorDialog::createFilterPanel(QVBoxLayout* layout) {
    QGroupBox* group = new QGroupBox("Filters", this);
    QHBoxLayout* groupLayout = new QHBoxLayout(group);

    groupLayout->addWidget(new QLabel("Preset Filter:", this));

    filterCombo = new QComboBox(this);
    filterCombo->addItems({ "None", "Grayscale", "Sepia", "Negative", "Pastel", "Vintage" });

    groupLayout->addWidget(filterCombo);
    groupLayout->addStretch();

    layout->addWidget(group);
}

void PhotoEditorDialog::createWatermarkPanel(QVBoxLayout* layout) {
    QGroupBox* group = new QGroupBox("Watermark", this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    // Select watermark button
    watermarkBtn = new QPushButton("Select Watermark Image", this);
    groupLayout->addWidget(watermarkBtn);

    // Position selector
    QHBoxLayout* posLayout = new QHBoxLayout();
    posLayout->addWidget(new QLabel("Position:", this));
    watermarkPositionCombo = new QComboBox(this);
    watermarkPositionCombo->addItems({ "Top Left", "Top Right", "Bottom Left", "Bottom Right", "Center" });
    watermarkPositionCombo->setCurrentIndex(DEFAULT_WATERMARK_POSITION);
    posLayout->addWidget(watermarkPositionCombo);
    groupLayout->addLayout(posLayout);

    // Opacity slider
    QHBoxLayout* opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(new QLabel("Opacity:", this));

    watermarkOpacitySlider = new QSlider(Qt::Horizontal, this);
    watermarkOpacitySlider->setRange(0, 100);
    watermarkOpacitySlider->setValue(DEFAULT_WATERMARK_OPACITY);

    QSpinBox* opacitySpin = new QSpinBox(this);
    opacitySpin->setRange(0, 100);
    opacitySpin->setValue(DEFAULT_WATERMARK_OPACITY);
    opacitySpin->setSuffix("%");

    connect(watermarkOpacitySlider, &QSlider::valueChanged, opacitySpin, &QSpinBox::setValue);
    connect(opacitySpin, QOverload<int>::of(&QSpinBox::valueChanged),
        watermarkOpacitySlider, &QSlider::setValue);

    opacityLayout->addWidget(watermarkOpacitySlider, 3);
    opacityLayout->addWidget(opacitySpin, 1);
    groupLayout->addLayout(opacityLayout);

    layout->addWidget(group);
}

void PhotoEditorDialog::createActionButtons(QVBoxLayout* layout) {
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    resetBtn = new QPushButton("Reset", this);
    cancelBtn = new QPushButton("Cancel", this);
    applyBtn = new QPushButton("Apply", this);

    buttonLayout->addWidget(resetBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(applyBtn);

    layout->addLayout(buttonLayout);
}

// --- Signal Connections ---

void PhotoEditorDialog::connectSignals() {
    // Update timer for debouncing
    updateTimer = new QTimer(this);
    updateTimer->setSingleShot(true);
    updateTimer->setInterval(TIMER_DELAY_MS);
    connect(updateTimer, &QTimer::timeout, this, &PhotoEditorDialog::updatePreview);

    // Basic tools
    connect(rotateLeftBtn, &QPushButton::clicked, this, &PhotoEditorDialog::rotateLeft);
    connect(rotateRightBtn, &QPushButton::clicked, this, &PhotoEditorDialog::rotateRight);
    connect(cropBtn, &QPushButton::toggled, this, &PhotoEditorDialog::cropClicked);

    // Adjustments
    connectSliderWithSpinbox(brightnessSlider, brightnessValue, m_brightness);
    connectSliderWithSpinbox(contrastSlider, contrastValue, m_contrast);
    connectSliderWithSpinbox(saturationSlider, saturationValue, m_saturation);

    // Actions
    connect(applyBtn, &QPushButton::clicked, this, &PhotoEditorDialog::applyChanges);
    connect(resetBtn, &QPushButton::clicked, this, &PhotoEditorDialog::resetChanges);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    // Filter
    connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &PhotoEditorDialog::applyFilter);

    // Watermark
    connect(watermarkBtn, &QPushButton::clicked, this, &PhotoEditorDialog::addWatermark);
    connect(watermarkPositionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index) {
            m_watermarkPosition = index;
            updateTimer->start();
        });
    connect(watermarkOpacitySlider, &QSlider::valueChanged, this, [this](int value) {
        m_watermarkOpacity = value;
        updateTimer->start();
        });
}

void PhotoEditorDialog::connectSliderWithSpinbox(QSlider* slider, QSpinBox* spinbox, int& value) {
    connect(slider, &QSlider::valueChanged, spinbox, &QSpinBox::setValue);
    connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);
    connect(slider, &QSlider::valueChanged, this, [this, &value](int newValue) {
        value = newValue;
        updateTimer->start();
        });
}

// --- Basic Operations ---

void PhotoEditorDialog::rotateLeft() {
    m_rotation = (m_rotation - 90 + 360) % 360;
    updatePreview();
}

void PhotoEditorDialog::rotateRight() {
    m_rotation = (m_rotation + 90) % 360;
    updatePreview();
}

void PhotoEditorDialog::cropClicked(bool checked) {
 
}

// --- Mouse Events for Crop ---

void PhotoEditorDialog::mousePressEvent(QMouseEvent* event) {
   
}

void PhotoEditorDialog::mouseMoveEvent(QMouseEvent* event) {
  
}

void PhotoEditorDialog::mouseReleaseEvent(QMouseEvent* event) {
  
}

void PhotoEditorDialog::applyCrop() {
}

// --- Image Processing ---

void PhotoEditorDialog::updatePreview() {
    QImage image = m_originalPixmap.toImage();

    applyRotation(image);
    applyBrightness(image);
    applyContrast(image);
    applySaturation(image);
    applyActiveFilter(image);
    applyWatermark(image);

    m_editedPixmap = QPixmap::fromImage(image);
    displayScaledPreview();
}

void PhotoEditorDialog::applyRotation(QImage& image) {
    if (m_rotation == 0) return;

    QTransform transform;
    transform.rotate(m_rotation);
    image = image.transformed(transform, Qt::SmoothTransformation);
}

void PhotoEditorDialog::applyContrast(QImage& image) 
{
    if (m_contrast == 0) return; // ziadna zmena

    // vypocet kontrastneho faktora (rovnica z Photoshop-like modelu)
    double factor = (259 * (m_contrast + 255)) / (255.0 * (259 - m_contrast));

    for (int y = 0; y < image.height(); y++) 
    {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); x++) 
        {
            QColor pixelColor = QColor::fromRgb(line[x]);

            int r = std::clamp(int(factor * (pixelColor.red() - 128) + 128), 0, 255);
            int g = std::clamp(int(factor * (pixelColor.green() - 128) + 128), 0, 255);
            int b = std::clamp(int(factor * (pixelColor.blue() - 128) + 128), 0, 255);

            line[x] = qRgb(r, g, b);
        }
    }
}

void PhotoEditorDialog::applyBrightness(QImage& image) 
{
    if (m_brightness == 0) return; // ziadna zmena

    for (int y = 0; y < image.height(); y++) 
    {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); x++) 
        {
            QColor pixelColor = QColor::fromRgb(line[x]);

            int r = std::clamp(pixelColor.red() + m_brightness, 0, 255);
            int g = std::clamp(pixelColor.green() + m_brightness, 0, 255);
            int b = std::clamp(pixelColor.blue() + m_brightness, 0, 255);

            line[x] = qRgb(r, g, b);
        }
    }
}

void PhotoEditorDialog::applySaturation(QImage& image) 
{
    if (m_saturation == 0) // Ak je saturacia 0, netreba nic robit
        return;

    double factor = 1.0 + (m_saturation / 100.0); // -100 --> 0.0, 0 --> 1.0, 100 --> 2.0

    for (int y = 0; y < image.height(); ++y) 
    {
        for (int x = 0; x < image.width(); ++x) 
        {
            QColor color = image.pixelColor(x, y);

            float h, s, l;
            color.getHslF(&h, &s, &l);

            // uprava saturacie
            s = std::clamp(s * factor, 0.0, 1.0);

            color.setHslF(h, s, l);
            image.setPixelColor(x, y, color);
        }
    }
}


void PhotoEditorDialog::displayScaledPreview() 
{
    QPixmap scaled = m_editedPixmap.scaled(
        previewLabel->size(),
        Qt::KeepAspectRatio,
        Qt::FastTransformation
    );
    previewLabel->setPixmap(scaled);
}

// --- Actions ---

void PhotoEditorDialog::applyChanges() {
  
}

void PhotoEditorDialog::resetChanges() {
    // Reset all values
    m_rotation = 0;
    m_brightness = 0;
    m_contrast = 0;
    m_saturation = 0;
    m_activeFilter = 0;
    m_watermarkPixmap = QPixmap();
    m_watermarkOpacity = DEFAULT_WATERMARK_OPACITY;
    m_watermarkPosition = DEFAULT_WATERMARK_POSITION;

    // Reset UI controls
    brightnessSlider->setValue(DEFAULT_ADJUSTMENT);
    contrastSlider->setValue(DEFAULT_ADJUSTMENT);
    saturationSlider->setValue(DEFAULT_ADJUSTMENT);
    filterCombo->setCurrentIndex(0);
    watermarkOpacitySlider->setValue(DEFAULT_WATERMARK_OPACITY);
    watermarkPositionCombo->setCurrentIndex(DEFAULT_WATERMARK_POSITION);

    // Reload original image
    m_originalPixmap = QPixmap(m_originalPhoto.filePath());
    updatePreview();
}

// --- Filters ---

void PhotoEditorDialog::applyFilter(int filterIndex) 
{
    m_activeFilter = filterIndex;
    updatePreview();
}

void PhotoEditorDialog::applyActiveFilter(QImage& image) 
{
    if (m_activeFilter == 0) return; // None

    // Show progress dialog for large images
    bool showProgress = (image.width() * image.height() > PROGRESS_THRESHOLD_PIXELS);
    QProgressDialog* progress = nullptr;

    if (showProgress) 
    {
        progress = new QProgressDialog("Applying filter...", "Cancel", 0, 100, this);
        progress->setWindowModality(Qt::WindowModal);
        progress->setMinimumDuration(0);
    }

    switch (m_activeFilter) 
    {
    case 1: applyGrayscaleFilter(image, progress); break;
    case 2: applySepiaFilter(image, progress); break;
    case 3: applyNegativeFilter(image, progress); break;
    case 4: applyPastelFilter(image, progress); break;
    case 5: applyVintageFilter(image, progress); break;
    }

    if (progress) 
    {
        progress->setValue(100);
        delete progress;
    }
}

void PhotoEditorDialog::applyGrayscaleFilter(QImage& image, QProgressDialog* progress) {
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32) 
        image = image.convertToFormat(QImage::Format_RGB32);

    int height = image.height();
    int width = image.width();

    if (progress) 
    {
        progress->setMaximum(height);
        progress->show();
    }

    for (int y = 0; y < height; y++) 
    {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; x++) 
        {
            QColor pixelColor(line[x]);
            int gray = static_cast<int>(0.2126 * pixelColor.red() + 0.7152 * pixelColor.green() + 0.0722 * pixelColor.blue());
            line[x] = qRgb(gray, gray, gray);
        }

        if (progress) 
        {
            progress->setValue(y);
            QCoreApplication::processEvents(); // aktualizovanie dialogu
        }
    }

    if (progress) 
    {
        progress->hide();
    }
}


void PhotoEditorDialog::applySepiaFilter(QImage& image, QProgressDialog* progress) 
{
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32)
        image = image.convertToFormat(QImage::Format_RGB32);
   
    int height = image.height();
    int width = image.width();

    if (progress) 
    {
        progress->setMaximum(height);
        progress->show();
    }

    for (int y = 0; y < height; y++) 
    {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; x++) 
        {
            QColor pixelColor(line[x]);

            int newR = static_cast<int>(0.393 * pixelColor.red() + 0.769 * pixelColor.green() + 0.189 * pixelColor.blue());
            int newG = static_cast<int>(0.349 * pixelColor.red() + 0.686 * pixelColor.green() + 0.168 * pixelColor.blue());
            int newB = static_cast<int>(0.272 * pixelColor.red() + 0.534 * pixelColor.green() + 0.131 * pixelColor.blue());

            line[x] = qRgb(qMin(newR, 255), qMin(newG, 255), qMin(newB, 255));
        }

        if (progress) 
        {
            progress->setValue(y);
            QCoreApplication::processEvents();
        }
    }

    if (progress) 
    {
        progress->hide();
    }
}


void PhotoEditorDialog::applyNegativeFilter(QImage& image, QProgressDialog* progress) {
}

void PhotoEditorDialog::applyPastelFilter(QImage& image, QProgressDialog* progress) {
}

void PhotoEditorDialog::applyVintageFilter(QImage& image, QProgressDialog* progress) {
}

// --- Watermark ---

void PhotoEditorDialog::addWatermark() 
{
    QString file = QFileDialog::getOpenFileName(
        this, "Select Watermark Image", "", "Images (*.png *.jpg *.jpeg *.bmp)"
    );

    if (!file.isEmpty())
    {
        m_watermarkPixmap = QPixmap(file);
        updatePreview();
    }
}

void PhotoEditorDialog::applyWatermark(QImage& image) {
  
}

QPoint PhotoEditorDialog::calculateWatermarkPosition(const QSize& imageSize,const QSize& watermarkSize) {

    return QPoint();
}
