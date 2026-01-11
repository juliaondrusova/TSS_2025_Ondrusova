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
#include <QScrollArea>

// Constants
namespace {
    constexpr int MIN_ADJUSTMENT = -100;
    constexpr int MAX_ADJUSTMENT = 100;
    constexpr int DEFAULT_ADJUSTMENT = 0;
	constexpr int TIMER_DELAY_MS = 50; // Delay for debouncing updates
    constexpr int DEFAULT_WATERMARK_OPACITY = 70;
    constexpr int DEFAULT_WATERMARK_POSITION = 3; // Bottom Right
	constexpr int WATERMARK_MARGIN = 20; // Margin from edges
    constexpr int PROGRESS_THRESHOLD_PIXELS = 1000000; // 1 megapixel
}

// Constructor
PhotoEditorDialog::PhotoEditorDialog(Photo* photo, QWidget* parent)
    : QDialog(parent),
    m_photoPtr(photo),
    m_originalPhoto(*photo),
    m_showingOriginal(false),
    m_rotation(0),
    m_brightness(0),
    m_contrast(0),
    m_saturation(0),
	m_cropMode(false),
    m_rubberBand(nullptr),
    m_activeFilter(0),
    m_watermarkOpacity(DEFAULT_WATERMARK_OPACITY),
    m_watermarkPosition(DEFAULT_WATERMARK_POSITION),
    m_temperature(0)
{
    setWindowTitle("Photo Editor");
    resize(900, 700);

	// load edited or original pixmap
    m_originalPixmap = photo->hasEditedVersion() 
        ? photo->editedPixmap() 
        : QPixmap(photo->filePath());

    m_editedPixmap = m_originalPixmap;

	buildUI(); // Setup UI components
	connectSignals(); // Connect signals and slots

    // Schedule update after the constructor finishes to ensure correct label dimensions.
    QTimer::singleShot(0, this, [this]() {
        updatePreview();
        });
}

// --- UI Construction ---

void PhotoEditorDialog::buildUI()
{
    setFixedSize(1000, 650);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ================= CONTENT =================
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // ---------- LEFT PANEL ----------
    QWidget* leftPanel = new QWidget(this);

    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 20, 20, 20);
    leftLayout->setSpacing(12);

    createPreviewArea(leftLayout);
    createToolbar(leftLayout);

    contentLayout->addWidget(leftPanel, 3);

    // ---------- RIGHT PANEL ----------
    QWidget* rightPanel = new QWidget(this);
    rightPanel->setFixedWidth(340);

    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(0, 5, 0, 5);

    QScrollArea* scrollArea = new QScrollArea(rightPanel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(20);
    scrollLayout->setContentsMargins(20, 20, 20, 20);

    createAdjustmentPanel(scrollLayout);
    createFilterPanel(scrollLayout);
    createWatermarkPanel(scrollLayout);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);

    rightLayout->addWidget(scrollArea);
    createActionButtons(rightLayout);

    contentLayout->addWidget(rightPanel);
    mainLayout->addLayout(contentLayout);
}


void PhotoEditorDialog::createPreviewArea(QVBoxLayout* layout)
{
    previewLabel = new QLabel(this);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(400, 400);
    previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    previewLabel->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(0, 0, 0, 15);"
        "   border: 1px solid rgba(128, 128, 128, 40);"
        "   border-radius: 20px;"
        "}"
    );
    layout->addWidget(previewLabel, 1);
}

void PhotoEditorDialog::createToolbar(QVBoxLayout* layout)
{
    QHBoxLayout* toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);

    cropBtn = new QPushButton("Crop", this);
    cropBtn->setCheckable(true);

    rotateLeftBtn = new QPushButton("Rotate Left", this);
    rotateRightBtn = new QPushButton("Rotate Right", this);
    resetBtn = new QPushButton("Reset", this);

    toolbar->addWidget(cropBtn);
    toolbar->addWidget(rotateLeftBtn);
    toolbar->addWidget(rotateRightBtn);
    toolbar->addWidget(resetBtn);
    toolbar->addStretch();

    layout->addLayout(toolbar);
}

void PhotoEditorDialog::createAdjustmentPanel(QVBoxLayout* layout)
{
    // Zmena QWidget na QGroupBox
    QGroupBox* sectionGroup = new QGroupBox("ADJUSTMENTS", this);
    QVBoxLayout* adjustLayout = new QVBoxLayout(sectionGroup);

    adjustLayout->setSpacing(10);
    adjustLayout->setContentsMargins(0, 0, 0, 0);

    createAdjustmentSlider("Brightness", brightnessSlider, brightnessValue, adjustLayout);
    createAdjustmentSlider("Contrast", contrastSlider, contrastValue, adjustLayout);
    createAdjustmentSlider("Saturation", saturationSlider, saturationValue, adjustLayout);
    createAdjustmentSlider("Temperature", temperatureSlider, temperatureValue, adjustLayout);

    layout->addWidget(sectionGroup);
}

void PhotoEditorDialog::createAdjustmentSlider(
    const QString& label,
    QSlider*& slider,
    QSpinBox*& spinbox,
    QVBoxLayout* layout)
{
    QVBoxLayout* block = new QVBoxLayout();
    block->setSpacing(8);

    QHBoxLayout* header = new QHBoxLayout();
    QLabel* lbl = new QLabel(label, this);

    spinbox = new QSpinBox(this);
    spinbox->setRange(MIN_ADJUSTMENT, MAX_ADJUSTMENT);
    spinbox->setValue(DEFAULT_ADJUSTMENT);
    spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);

    spinbox->setFocusPolicy(Qt::StrongFocus);
    spinbox->installEventFilter(this);

    header->addWidget(lbl);
    header->addStretch();
    header->addWidget(spinbox);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(MIN_ADJUSTMENT, MAX_ADJUSTMENT);
    slider->setValue(DEFAULT_ADJUSTMENT);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->installEventFilter(this);

    block->addLayout(header);
    block->addWidget(slider);
    layout->addLayout(block);
}


bool PhotoEditorDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Wheel) {
        if (qobject_cast<QSlider*>(obj) ||
            qobject_cast<QSpinBox*>(obj) ||
            qobject_cast<QComboBox*>(obj))
        {
            event->ignore(); 
            return true; 
        }
    }
    return QDialog::eventFilter(obj, event);
}


void PhotoEditorDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);

    if (!m_editedPixmap.isNull()) {
        previewLabel->setPixmap(
            m_editedPixmap.scaled(
                previewLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
    }
}

void PhotoEditorDialog::createSliderWithSpinbox(const QString& label, QSlider*& slider, QSpinBox*& spinbox, QVBoxLayout* layout)
{
    createAdjustmentSlider(label, slider, spinbox, layout);
}

void PhotoEditorDialog::createFilterPanel(QVBoxLayout* layout)
{
    // Zmena QWidget na QGroupBox
    QGroupBox* filterGroup = new QGroupBox("FILTER PRESETS", this);
    QVBoxLayout* filterLayout = new QVBoxLayout(filterGroup);

    filterLayout->setSpacing(10);
    filterLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* filterLabel = new QLabel("Preset Filter", this);
    filterLayout->addWidget(filterLabel);

    filterCombo = new QComboBox(this);
    filterCombo->addItems({ "None", "Grayscale", "Sepia", "Negative", "Pastel", "Vintage" });
    filterCombo->setFixedHeight(38);

    filterCombo->setFocusPolicy(Qt::StrongFocus);
    filterCombo->installEventFilter(this);

    filterLayout->addWidget(filterCombo);

    layout->addWidget(filterGroup);
}

void PhotoEditorDialog::createWatermarkPanel(QVBoxLayout* layout)
{
    // Zmena QWidget na QGroupBox
    QGroupBox* watermarkGroup = new QGroupBox("WATERMARK", this);
    QVBoxLayout* watermarkLayout = new QVBoxLayout(watermarkGroup);

    watermarkLayout->setSpacing(10);
    watermarkLayout->setContentsMargins(0, 0, 0, 0);

    watermarkBtn = new QPushButton("+ Select Watermark Image", this);
    watermarkBtn->setFixedHeight(40);
    watermarkLayout->addWidget(watermarkBtn);

    watermarkLayout->addWidget(new QLabel("Position", this));
    watermarkPositionCombo = new QComboBox(this);
    watermarkPositionCombo->addItems({ "Top Left", "Top Right", "Bottom Left", "Bottom Right", "Center" });
    watermarkPositionCombo->setCurrentIndex(DEFAULT_WATERMARK_POSITION);
    watermarkPositionCombo->setFixedHeight(38);
    watermarkPositionCombo->setFocusPolicy(Qt::StrongFocus);
    watermarkPositionCombo->installEventFilter(this);
    watermarkLayout->addWidget(watermarkPositionCombo);

    watermarkLayout->addWidget(new QLabel("Opacity", this));
    QHBoxLayout* opacityRow = new QHBoxLayout();
    watermarkOpacitySlider = new QSlider(Qt::Horizontal, this);
    watermarkOpacitySlider->setRange(0, 100);
    watermarkOpacitySlider->setValue(DEFAULT_WATERMARK_OPACITY);
    watermarkOpacitySlider->setFocusPolicy(Qt::StrongFocus);
    watermarkOpacitySlider->installEventFilter(this);

    QLabel* percentLabel = new QLabel("70%", this);
    percentLabel->setFixedWidth(40);
    percentLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    connect(watermarkOpacitySlider, &QSlider::valueChanged, [percentLabel](int val) {
        percentLabel->setText(QString("%1%").arg(val));
        });

    opacityRow->addWidget(watermarkOpacitySlider);
    opacityRow->addWidget(percentLabel);
    watermarkLayout->addLayout(opacityRow);

    layout->addWidget(watermarkGroup);
}
 
void PhotoEditorDialog::createActionButtons(QVBoxLayout* layout)
{
    QWidget* buttonWidget = new QWidget(this);
    buttonWidget->setFixedHeight(70);

    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(20, 12, 20, 12);

    cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setFixedHeight(44);

    applyBtn = new QPushButton("Apply Changes", this);
    applyBtn->setFixedHeight(44);
   
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(applyBtn);

    layout->addWidget(buttonWidget);
}


// --- Signal Connections ---

void PhotoEditorDialog::connectSignals() 
{
    // Update timer for debouncing
    updateTimer = new QTimer(this);
    updateTimer->setSingleShot(true);
	updateTimer->setInterval(TIMER_DELAY_MS); // 50 ms delay
	connect(updateTimer, &QTimer::timeout, this, &PhotoEditorDialog::updatePreview); // if no changes in 50ms, update preview

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
    connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PhotoEditorDialog::applyFilter);

    // Watermark
    connect(watermarkBtn, &QPushButton::clicked, this, &PhotoEditorDialog::addWatermark);
    connect(watermarkPositionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_watermarkPosition = index;
            updateTimer->start();
     });
    connect(watermarkOpacitySlider, &QSlider::valueChanged, this, [this](int value) {
        m_watermarkOpacity = value;
        updateTimer->start();
        });

    connectSliderWithSpinbox(temperatureSlider, temperatureValue, m_temperature);
}

void PhotoEditorDialog::connectSliderWithSpinbox(QSlider* slider, QSpinBox* spinbox, int& value) 
{
	// Synchronize slider and spinbox, update value and start timer on change
    connect(slider, &QSlider::valueChanged, spinbox, &QSpinBox::setValue);
    connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);
    connect(slider, &QSlider::valueChanged, this, [this, &value](int newValue) {
        value = newValue;
        updateTimer->start();
     });
}

// --- Basic Operations ---

void PhotoEditorDialog::rotateLeft() 
{
    m_rotation = (m_rotation + 90 + 360) % 360; 
    updatePreview();
}

void PhotoEditorDialog::rotateRight() 
{
    m_rotation = (m_rotation - 90) % 360;
    updatePreview();
}

void PhotoEditorDialog::cropClicked(bool checked)
{
    m_cropMode = checked;

    if (!m_cropMode && m_rubberBand) // If crop mode is turned off and rubber band exists, hide it
        m_rubberBand->hide();

    // Change cursor depending on crop mode
    if (m_cropMode)
        previewLabel->setCursor(Qt::CrossCursor);
    else
        previewLabel->setCursor(Qt::ArrowCursor);
}

// --- Mouse Events for Crop ---

void PhotoEditorDialog::mousePressEvent(QMouseEvent* event)
{
    // Prepoèítame pozíciu kliknutia relatívnu k previewLabel
    QPoint posInLabel = previewLabel->mapFromGlobal(event->globalPos());

    // Kliknutie je vo vnútri labelu
    if (previewLabel->rect().contains(posInLabel))
    {
        if (m_cropMode)
        {
            // Spustíme crop
            m_cropOrigin = posInLabel;

            if (!m_rubberBand)
                m_rubberBand = new QRubberBand(QRubberBand::Rectangle, previewLabel);

            m_rubberBand->setGeometry(QRect(m_cropOrigin, QSize()));
            m_rubberBand->show();
        }
        else
        {
            // Zobrazenie pôvodného obrázka
            QPixmap originalFromFile(m_originalPhoto.filePath());
            QPixmap scaled = originalFromFile.scaled(
                previewLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );

            previewLabel->setPixmap(scaled);
            m_showingOriginal = true;
        }

        event->accept();
        return;
    }

    // Ak klik nie je na obrázku, spracujeme default
    QDialog::mousePressEvent(event);
}


void PhotoEditorDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (m_cropMode && m_rubberBand && m_rubberBand->isVisible())
    {
        QPoint currentPos = previewLabel->mapFromGlobal(event->globalPos());

        // Obmedzíme výber len na plochu labelu, aby RubberBand "neutiekol" von
        currentPos.setX(qBound(0, currentPos.x(), previewLabel->width()));
        currentPos.setY(qBound(0, currentPos.y(), previewLabel->height()));

        m_rubberBand->setGeometry(QRect(m_cropOrigin, currentPos).normalized());
    }
    QDialog::mouseMoveEvent(event);
}


void PhotoEditorDialog::mouseReleaseEvent(QMouseEvent* event)
{
    // If showing original, return to edited version
    if (m_showingOriginal)
    {
        m_showingOriginal = false;
		displayScaledPreview();  // Show edited version
        event->accept();
        return;
    }

    // When mouse is released, apply crop if rectangle is visible
    if (m_cropMode && m_rubberBand && m_rubberBand->isVisible())
    {
        applyCrop();            // Crop the selected area
        m_rubberBand->hide();   // Hide selection rectangle

        // Turn off crop mode and reset cursor
        cropBtn->setChecked(false);
        m_cropMode = false;
		previewLabel->setCursor(Qt::ArrowCursor); // Reset cursor
    }

    // Pass event to base class
    QDialog::mouseReleaseEvent(event);
}

void PhotoEditorDialog::applyCrop()
{
    // If rubber band does not exist, nothing to crop
    if (!m_rubberBand)
        return;

    QRect cropRect = m_rubberBand->geometry();
	QPixmap displayedPixmap = previewLabel->pixmap(Qt::ReturnByValue); // Get currently displayed pixmap

    if (displayedPixmap.isNull()) // Stop if there is no image loaded
        return;

    QSize labelSize = previewLabel->size();
    QSize pixmapSize = displayedPixmap.size();

    // Adjust for centering: if the image is centered in the label,
    // the crop rectangle must be shifted accordingly
    int offsetX = (labelSize.width() - pixmapSize.width()) / 2;
    int offsetY = (labelSize.height() - pixmapSize.height()) / 2;
    cropRect.translate(-offsetX, -offsetY);

    // Compute scale ratio between displayed image and original image
    double scaleX = (double)m_editedPixmap.width() / pixmapSize.width(); 
    double scaleY = (double)m_editedPixmap.height() / pixmapSize.height();

    // Convert crop rectangle to original image coordinates
    m_cropRect = QRect(
        cropRect.x() * scaleX,
        cropRect.y() * scaleY,
        cropRect.width() * scaleX,
        cropRect.height() * scaleY
    );

    // If valid crop area, perform crop and refresh preview
    if (m_cropRect.isValid() && !m_cropRect.isEmpty())
    {
        m_originalPixmap = m_editedPixmap.copy(m_cropRect);
        m_cropRect = QRect();   // Reset stored crop area
        updatePreview();        // Update displayed image
    }
}

// --- Image Processing ---

void PhotoEditorDialog::updatePreview() 
{
    QImage image = m_originalPixmap.toImage(); 

	// Apply all adjustments in sequence
    applyRotation(image);
    applyBrightness(image);
    applyContrast(image);
    applySaturation(image);
    applyActiveFilter(image);
    applyWatermark(image);
    applyTemperature(image);

    m_editedPixmap = QPixmap::fromImage(image);
	displayScaledPreview(); // Show updated image in preview
}

void PhotoEditorDialog::applyRotation(QImage& image) 
{
    if (m_rotation == 0) 
        return;

    QTransform transform;
	transform.rotate(m_rotation); // Rotate by the specified angle
	image = image.transformed(transform, Qt::SmoothTransformation); 
}

void PhotoEditorDialog::applyContrast(QImage& image) 
{
	if (m_contrast == 0) 
        return; // No change

	double factor = (259 * (m_contrast + 255)) / (255.0 * (259 - m_contrast)); // Contrast factor calculation, photoshop formula

    for (int y = 0; y < image.height(); y++) 
    {
		QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y)); // Get pointer to the start of the line
        for (int x = 0; x < image.width(); x++) 
        {
			QColor pixelColor = QColor::fromRgb(line[x]); // Get pixel color

			// Apply contrast formula
            int r = std::clamp(int(factor * (pixelColor.red() - 128) + 128), 0, 255);
            int g = std::clamp(int(factor * (pixelColor.green() - 128) + 128), 0, 255);
            int b = std::clamp(int(factor * (pixelColor.blue() - 128) + 128), 0, 255);

			line[x] = qRgb(r, g, b); // Set new pixel color
        }
    }
}

void PhotoEditorDialog::applyBrightness(QImage& image) 
{
	if (m_brightness == 0) return; // No change

    for (int y = 0; y < image.height(); y++) 
    {
		QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y)); // Get pointer to the start of the line
        for (int x = 0; x < image.width(); x++) 
        {
			QColor pixelColor = QColor::fromRgb(line[x]); // Get pixel color

			// Adjust brightness
            int r = std::clamp(pixelColor.red() + m_brightness, 0, 255);
            int g = std::clamp(pixelColor.green() + m_brightness, 0, 255);
            int b = std::clamp(pixelColor.blue() + m_brightness, 0, 255);

            line[x] = qRgb(r, g, b);
        }
    }
}

void PhotoEditorDialog::applySaturation(QImage& image) 
{
	if (m_saturation == 0) // No change
        return;

    double factor = 1.0 + (m_saturation / 100.0); // -100 --> 0.0, 0 --> 1.0, 100 --> 2.0

    for (int y = 0; y < image.height(); ++y) 
    {
        for (int x = 0; x < image.width(); ++x) 
        {
			QColor color = image.pixelColor(x, y); // Get pixel color

            float h, s, l;
			color.getHslF(&h, &s, &l); // Get HSL components

			// Adjust saturation and clamp to [0, 1]
            s = std::clamp(s * factor, 0.0, 1.0);

            color.setHslF(h, s, l);
			image.setPixelColor(x, y, color); // Set new pixel color
        }
    }
}


void PhotoEditorDialog::displayScaledPreview() 
{
	// Scale the edited pixmap to fit the preview label while maintaining aspect ratio
    QPixmap scaled = m_editedPixmap.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    previewLabel->setPixmap(scaled);
}

// --- Actions ---

void PhotoEditorDialog::applyChanges() 
{
    if (m_photoPtr)
		m_photoPtr->setEditedPixmap(m_editedPixmap); // Save edited pixmap to photo object

	accept(); // Close dialog with success
}

void PhotoEditorDialog::resetChanges() 
{
    // Reset all values
    m_rotation = 0;
    m_brightness = 0;
    m_contrast = 0;
    m_saturation = 0;
    m_activeFilter = 0;
    m_watermarkPixmap = QPixmap();
    m_watermarkOpacity = DEFAULT_WATERMARK_OPACITY;
    m_watermarkPosition = DEFAULT_WATERMARK_POSITION;
    m_temperature = 0;
    temperatureSlider->setValue(DEFAULT_ADJUSTMENT);

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
	updatePreview(); // Update preview with new filter
}

void PhotoEditorDialog::applyActiveFilter(QImage& image) 
{
    if (m_activeFilter == 0) 
        return; // None

    // Show progress dialog for large images
	bool showProgress = (image.width() * image.height() > PROGRESS_THRESHOLD_PIXELS); // 1 megapixel
    QProgressDialog* progress = nullptr;

	if (showProgress) // Create progress dialog if needed
    {
        progress = new QProgressDialog("Applying filter...", "Cancel", 0, 100, this);
        progress->setWindowModality(Qt::WindowModal);
        progress->setMinimumDuration(0);
    }

    processImagePixels(image, progress, m_activeFilter);

	if (progress) // Clean up progress dialog
    {
        progress->setValue(100);
        delete progress;
    }
}

void PhotoEditorDialog::processImagePixels(
    QImage& image,
    QProgressDialog* progress,
    int filterNumber) // filterNumber: 0 = none, 1 = grayscale, 2 = sepia, 3 = negative, 4 = pastel, 5 = vintage
{
    if (image.format() != QImage::Format_RGB32)
        image = image.convertToFormat(QImage::Format_RGB32);

    int width = image.width();
    int height = image.height();

    if (progress)
    {
        progress->setMaximum(height);
        progress->show();
    }

    for (int y = 0; y < height; y++)
    {
        QRgb* line = (QRgb*)image.scanLine(y);

        for (int x = 0; x < width; x++)
        {
            QRgb pixel = line[x];

            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            int nr = r;
            int ng = g;
            int nb = b;

            switch (filterNumber)
            {
            case 1: // Grayscale
            {
                int gray = qGray(pixel);
                nr = ng = nb = gray;
                break;
            }
            case 2: // Sepia
            {
                nr = qMin(255, int(0.393 * r + 0.769 * g + 0.189 * b));
                ng = qMin(255, int(0.349 * r + 0.686 * g + 0.168 * b));
                nb = qMin(255, int(0.272 * r + 0.534 * g + 0.131 * b));
                break;
            }
            case 3: // Negative
            {
                nr = 255 - r;
                ng = 255 - g;
                nb = 255 - b;
                break;
            }
            case 4: // Pastel
            {
                nr = qBound(0, int(r * 0.8 + 70), 255);
                ng = qBound(0, int(g * 0.8 + 60), 255);
                nb = qBound(0, int(b * 0.9 + 95), 255);
                break;
            }
            case 5: // Vintage
            {
                int gray = qGray(pixel);
                int dr = (r + gray * 2) / 3;
                int dg = (g + gray * 2) / 3;
                int db = (b + gray * 2) / 3;

                nr = qBound(0, int(dr * 0.9 + dg * 0.5 + db * 0.2), 255);
                ng = qBound(0, int(dr * 0.3 + dg * 0.7 + db * 0.2), 255);
                nb = qBound(0, int(dr * 0.1 + dg * 0.3 + db * 0.6), 255);

                nr = (nr + 255) / 2;
                ng = (ng + 255) / 2;
                nb = (nb + 255) / 2;
                break;
            }
            default:
                // 0 alebo neznámy filter = niè sa nemení
                break;
            }

            line[x] = qRgb(nr, ng, nb);
        }

        if (progress && y % 10 == 0)
        {
            progress->setValue(y);
            QCoreApplication::processEvents();
        }
    }
}

// --- Watermark ---

void PhotoEditorDialog::addWatermark() 
{
	// Open file dialog to select watermark image
    QString file = QFileDialog::getOpenFileName(
        this, "Select Watermark Image", "", "Images (*.png *.jpg *.jpeg *.bmp)"
    );

	if (!file.isEmpty()) // If a file was selected
    {
        m_watermarkPixmap = QPixmap(file);
		updatePreview(); // Refresh preview to show watermark
    }
}

void PhotoEditorDialog::applyWatermark(QImage& image) 
{
	if (m_watermarkPixmap.isNull()) return; // No watermark to apply

    QPainter painter(&image);
	painter.setOpacity(m_watermarkOpacity / 100.0); // UI slider gives 0-100, convert to 0.0-1.0

    // Scale watermark to 1/4 of image width
    int wmWidth = image.width() / 4;
    QPixmap scaledWm = m_watermarkPixmap.scaled(wmWidth, wmWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    
    painter.drawPixmap(calculateWatermarkPosition(image.size(), scaledWm.size()), scaledWm); // Draw watermark at calculated position
	painter.end(); // End painting
}

QPoint PhotoEditorDialog::calculateWatermarkPosition(const QSize& imageSize, const QSize& watermarkSize) 
{
    int x = 0, y = 0;

    switch (m_watermarkPosition) 
    {
    case 0: // Top Left
        x = WATERMARK_MARGIN;
        y = WATERMARK_MARGIN;
        break;

    case 1: // Top Right
        x = imageSize.width() - watermarkSize.width() - WATERMARK_MARGIN;
        y = WATERMARK_MARGIN;
        break;

    case 2: // Bottom Left
        x = WATERMARK_MARGIN;
        y = imageSize.height() - watermarkSize.height() - WATERMARK_MARGIN;
        break;

    case 3: // Bottom Right
        x = imageSize.width() - watermarkSize.width() - WATERMARK_MARGIN;
        y = imageSize.height() - watermarkSize.height() - WATERMARK_MARGIN;
        break;

    case 4: // Center
        x = (imageSize.width() - watermarkSize.width()) / 2;
        y = (imageSize.height() - watermarkSize.height()) / 2;
        break;
    }

	return QPoint(x, y); // Return calculated position
}


void PhotoEditorDialog::applyTemperature(QImage& image)
{
    if (m_temperature == 0) return;

    double factor = m_temperature / 100.0; // -1.0 to +1.0

    for (int y = 0; y < image.height(); y++) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); x++) {
            QColor color = QColor::fromRgb(line[x]);

            int r = std::clamp(int(color.red() + 30 * factor), 0, 255);
            int b = std::clamp(int(color.blue() - 30 * factor), 0, 255);

            line[x] = qRgb(r, color.green(), b);
        }
    }
}