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
    m_watermarkPosition(DEFAULT_WATERMARK_POSITION)
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
	updatePreview(); // Initial preview update
}

// --- UI Construction ---

void PhotoEditorDialog::buildUI() 
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

	createPreviewArea(mainLayout); // Preview area at the top
	createToolbar(mainLayout); // Toolbar with basic tools

    // Two-column layout for adjustments and filters
    QHBoxLayout* middleLayout = new QHBoxLayout();

	// Left column: Adjustments
    QVBoxLayout* leftColumn = new QVBoxLayout();
    createAdjustmentPanel(leftColumn);
    middleLayout->addLayout(leftColumn);

    // Right column: Filters and Watermark
    QVBoxLayout* rightColumn = new QVBoxLayout();
    createFilterPanel(rightColumn);
    createWatermarkPanel(rightColumn);
    middleLayout->addLayout(rightColumn);

	// Add middle layout to main layout
    mainLayout->addLayout(middleLayout);
    createActionButtons(mainLayout);
}

void PhotoEditorDialog::createPreviewArea(QVBoxLayout* layout) 
{
    previewLabel = new QLabel(this);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(600, 400);
    previewLabel->setScaledContents(false);
    previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	previewLabel->setMouseTracking(true); // Enable mouse tracking for crop

    layout->addWidget(previewLabel);
}

void PhotoEditorDialog::createToolbar(QVBoxLayout* layout) 
{
    QHBoxLayout* toolbar = new QHBoxLayout();

    cropBtn = new QPushButton("Crop", this);
	cropBtn->setCheckable(true); // Toggle button for crop mode

    rotateLeftBtn = new QPushButton("Rotate Left", this);
    rotateRightBtn = new QPushButton("Rotate Right", this);

    toolbar->addWidget(cropBtn);
    toolbar->addWidget(rotateLeftBtn);
    toolbar->addWidget(rotateRightBtn);
	toolbar->addStretch(); // Push buttons to the left

    layout->addLayout(toolbar);
}

void PhotoEditorDialog::createAdjustmentPanel(QVBoxLayout* layout) 
{
    QGroupBox* group = new QGroupBox("Adjustments", this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

	// sliders with spinboxes for brightness, contrast, saturation
    createSliderWithSpinbox("Brightness:", brightnessSlider, brightnessValue, groupLayout);
    createSliderWithSpinbox("Contrast:", contrastSlider, contrastValue, groupLayout);
    createSliderWithSpinbox("Saturation:", saturationSlider, saturationValue, groupLayout);

    layout->addWidget(group);
}

void PhotoEditorDialog::createSliderWithSpinbox(const QString& label, QSlider*& slider, QSpinBox*& spinbox, QVBoxLayout* layout) 
{
    QHBoxLayout* row = new QHBoxLayout();
    row->addWidget(new QLabel(label, this));

    slider = new QSlider(Qt::Horizontal, this);
	slider->setRange(MIN_ADJUSTMENT, MAX_ADJUSTMENT); // -100 to 100
	slider->setValue(DEFAULT_ADJUSTMENT); // default 0

    spinbox = new QSpinBox(this);
	spinbox->setRange(MIN_ADJUSTMENT, MAX_ADJUSTMENT); // -100 to 100
	spinbox->setValue(DEFAULT_ADJUSTMENT); // default 0

	row->addWidget(slider, 3); // slider takes 3/4 of space
	row->addWidget(spinbox, 1); // spinbox takes 1/4 of space
    layout->addLayout(row);
}

void PhotoEditorDialog::createFilterPanel(QVBoxLayout* layout) 
{
    QGroupBox* group = new QGroupBox("Filters", this);
    QHBoxLayout* groupLayout = new QHBoxLayout(group);

	groupLayout->addWidget(new QLabel("Preset Filter:", this)); // Filter selector

    filterCombo = new QComboBox(this);
	filterCombo->addItems({ "None", "Grayscale", "Sepia", "Negative", "Pastel", "Vintage" }); // Predefined filters

    groupLayout->addWidget(filterCombo);
    groupLayout->addStretch();

    layout->addWidget(group);
}

void PhotoEditorDialog::createWatermarkPanel(QVBoxLayout* layout) 
{
    QGroupBox* group = new QGroupBox("Watermark", this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    // Select watermark button
    watermarkBtn = new QPushButton("Select Watermark Image", this);
    groupLayout->addWidget(watermarkBtn);

    // Position selector
    QHBoxLayout* posLayout = new QHBoxLayout();
    posLayout->addWidget(new QLabel("Position:", this));

    watermarkPositionCombo = new QComboBox(this);
	watermarkPositionCombo->addItems({ "Top Left", "Top Right", "Bottom Left", "Bottom Right", "Center" }); // Preddefined positions
	watermarkPositionCombo->setCurrentIndex(DEFAULT_WATERMARK_POSITION); // Default to Bottom Right
    
    posLayout->addWidget(watermarkPositionCombo);
    groupLayout->addLayout(posLayout);

    // Opacity slider
    QHBoxLayout* opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(new QLabel("Opacity:", this));

    watermarkOpacitySlider = new QSlider(Qt::Horizontal, this);
    watermarkOpacitySlider->setRange(0, 100);
	watermarkOpacitySlider->setValue(DEFAULT_WATERMARK_OPACITY); // Default opacity

    QSpinBox* opacitySpin = new QSpinBox(this);
    opacitySpin->setRange(0, 100);
	opacitySpin->setValue(DEFAULT_WATERMARK_OPACITY); // Default opacity
	opacitySpin->setSuffix("%"); // Show percentage

    connect(watermarkOpacitySlider, &QSlider::valueChanged, opacitySpin, &QSpinBox::setValue);
    connect(opacitySpin, QOverload<int>::of(&QSpinBox::valueChanged),
        watermarkOpacitySlider, &QSlider::setValue);

	opacityLayout->addWidget(watermarkOpacitySlider, 3); // slider takes 3/4 of space
	opacityLayout->addWidget(opacitySpin, 1); // spinbox takes 1/4 of space

    groupLayout->addLayout(opacityLayout);
    layout->addWidget(group);
}

void PhotoEditorDialog::createActionButtons(QVBoxLayout* layout) 
{
    QHBoxLayout* buttonLayout = new QHBoxLayout();

	// Action buttons: Reset, Cancel, Apply
    resetBtn = new QPushButton("Reset", this);
    cancelBtn = new QPushButton("Cancel", this);
    applyBtn = new QPushButton("Apply", this);

    buttonLayout->addWidget(resetBtn);
	buttonLayout->addStretch(); // Push Cancel and Apply to the right
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(applyBtn);

    layout->addLayout(buttonLayout);
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
    m_rotation = (m_rotation - 90 + 360) % 360; 
    updatePreview();
}

void PhotoEditorDialog::rotateRight() 
{
    m_rotation = (m_rotation + 90) % 360;
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
    // Check if clicking on preview (not in crop mode)
    if (previewLabel->underMouse() && !m_cropMode)
    {
        m_showingOriginal = true;
        QPixmap originalFromFile = QPixmap(m_originalPhoto.filePath());
        QPixmap scaled = originalFromFile.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        previewLabel->setPixmap(scaled);
        event->accept();
        return;
    }

    // Only start cropping if crop mode is active and mouse is over the image
    if (m_cropMode && previewLabel->underMouse())
    {
        // Save the position where the user started dragging (relative to the label)
        m_cropOrigin = event->pos() - previewLabel->pos();

        if (!m_rubberBand) // Create the rubber band if it does not already exist
            m_rubberBand = new QRubberBand(QRubberBand::Rectangle, previewLabel);

        // Initialize the rectangle (with 0 size) and make it visible
        m_rubberBand->setGeometry(QRect(m_cropOrigin, QSize()));
        m_rubberBand->show();
    }

    // Pass event to base class
    QDialog::mousePressEvent(event);
}

void PhotoEditorDialog::mouseMoveEvent(QMouseEvent* event)
{
    // If user is dragging and crop rectangle is visible, resize it
    if (m_cropMode && m_rubberBand && m_rubberBand->isVisible())
    {
        // Current mouse position relative to label
        QPoint currentPos = event->pos() - previewLabel->pos();

		QRect rect(m_cropOrigin, currentPos); // Create rectangle from origin to current position
		m_rubberBand->setGeometry(rect.normalized()); // need to normalize to handle negative widths/heights
    }

    // Pass event to base class
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