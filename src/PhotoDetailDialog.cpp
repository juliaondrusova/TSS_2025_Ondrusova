#include "PhotoDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QPixmap>
#include <QPushButton>

// Helper function to calculate scale factor to fit a pixmap into a viewport
static double fitScale(const QSize& viewportSize, const QPixmap& pixmap) 
{
    if (pixmap.isNull()) 
        return 1.0;

    double factorW = static_cast<double>(viewportSize.width()) / pixmap.width();
    double factorH = static_cast<double>(viewportSize.height()) / pixmap.height();

    return qMin(factorW, factorH);
}

// Constructor
PhotoDetailDialog::PhotoDetailDialog(QWidget* parent)
    : QDialog(parent), isFullscreen(false)
{
    setWindowTitle("Photo Detail");
    resize(600, 600);

    // Create main vertical layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Scroll area for the image
    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Base);
    scrollArea->setWidgetResizable(false);
    scrollArea->setAlignment(Qt::AlignCenter);

    // Label for displaying image
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    scrollArea->setWidget(imageLabel);
    mainLayout->addWidget(scrollArea);

    // Bottom panel with zoom slider and fullscreen button
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(10, 400);  // 10% - 400% zoom
    zoomSlider->setValue(100);       // default 100%
    bottomLayout->addWidget(zoomSlider);

    fullscreenBtn = new QPushButton("Fullscreen", this);
    bottomLayout->addWidget(fullscreenBtn);
    mainLayout->addLayout(bottomLayout);

    // Connect signals for zoom and fullscreen
    connect(zoomSlider, &QSlider::valueChanged, this, &PhotoDetailDialog::zoomChanged);
    connect(fullscreenBtn, &QPushButton::clicked, this, &PhotoDetailDialog::toggleFullscreen);
}

// Set the photo to be displayed
void PhotoDetailDialog::setPhoto(const Photo& photo)
{
    currentPhoto = photo;

    // Load either edited pixmap or original photo file
    originalPixmap = photo.hasEditedVersion() ? photo.editedPixmap() : QPixmap(photo.filePath());
}

// Show event to set initial zoom
void PhotoDetailDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    if (originalPixmap.isNull())
        return;

    // Calculate and apply initial fit-to-viewport zoom
    int fitZoom = static_cast<int>(fitScale(scrollArea->viewport()->size(), originalPixmap) * 100);

    zoomSlider->blockSignals(true);
    zoomSlider->setValue(fitZoom);
    zoomSlider->blockSignals(false);

    zoomChanged(fitZoom);
}

// Handle mouse wheel events for zooming
void PhotoDetailDialog::wheelEvent(QWheelEvent* event)
{
    // Ctrl + wheel changes zoom
    if (event->modifiers() & Qt::ControlModifier)
    {
        // Zoom step
		int delta = event->angleDelta().y() > 0 ? 10 : -10; 

		// Calculate new zoom value within slider range
        int newValue = qBound(zoomSlider->minimum(), zoomSlider->value() + delta, zoomSlider->maximum());

        zoomSlider->setValue(newValue);
        event->accept();
    }
    else
    {
        QDialog::wheelEvent(event); 
    }
}

// Toggle fullscreen mode
void PhotoDetailDialog::toggleFullscreen()
{
    // Toggle between fullscreen and normal
    if (isFullscreen)
        showNormal();
    else
        showFullScreen();

	isFullscreen = !isFullscreen; // Update state
	fullscreenBtn->setText(isFullscreen ? "Exit Fullscreen" : "Fullscreen"); // Update button text

    // Adjust zoom for new window size
    if (!originalPixmap.isNull())
    {
		// Calculate and apply fit-to-viewport zoom
        int fitZoom = static_cast<int>(fitScale(scrollArea->viewport()->size(), originalPixmap) * 100);

        zoomSlider->blockSignals(true);
        zoomSlider->setValue(fitZoom);
        zoomSlider->blockSignals(false);

        zoomChanged(fitZoom);
    }
}

// Update image display based on zoom slider value
void PhotoDetailDialog::zoomChanged(int value) 
{
    if (originalPixmap.isNull()) 
        return;

    // Scale original pixmap according to slider value
    double scale = value / 100.0;
    QPixmap scaled = originalPixmap.scaled(
        originalPixmap.size() * scale,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    // Update image label
    imageLabel->setPixmap(scaled);
    imageLabel->resize(scaled.size());
}
