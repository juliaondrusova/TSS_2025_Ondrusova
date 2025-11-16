#include "PhotoDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QPixmap>
#include <QPushButton>

// Helper function to calculate scale factor to fit a pixmap into a viewport
static double fitScale(const QSize& viewportSize, const QPixmap& pixmap) {
    if (pixmap.isNull()) return 1.0;
    double factorW = static_cast<double>(viewportSize.width()) / pixmap.width();
    double factorH = static_cast<double>(viewportSize.height()) / pixmap.height();
    return qMin(factorW, factorH);
}

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

void PhotoDetailDialog::setPhoto(const Photo& photo) 
{
    // Load either edited pixmap or original photo file
    if(photo.hasEditedVersion())
        originalPixmap = photo.editedPixmap();
    else
     originalPixmap = QPixmap(photo.filePath());

    if (originalPixmap.isNull()) return;
    
    currentPhoto = photo;
}


void PhotoDetailDialog::showEvent(QShowEvent* event) 
{
    QDialog::showEvent(event);
    if (originalPixmap.isNull()) return;

    // Calculate scale to fit image in viewport
    double scale = fitScale(scrollArea->viewport()->size(), originalPixmap);

    // Update zoom slider without triggering slot
    zoomSlider->blockSignals(true);
    zoomSlider->setValue(static_cast<int>(scale * 100));
    zoomSlider->blockSignals(false);

    // Apply initial zoom
    zoomChanged(static_cast<int>(scale * 100));
}


void PhotoDetailDialog::wheelEvent(QWheelEvent* event) 
{
    // Ctrl + wheel changes zoom
    if (event->modifiers() & Qt::ControlModifier) 
    {
        int delta = event->angleDelta().y();
        int newValue = qBound(zoomSlider->minimum(),
            zoomSlider->value() + (delta > 0 ? 10 : -10),
            zoomSlider->maximum());
        zoomSlider->setValue(newValue);
        event->accept();
    }
    else
        QDialog::wheelEvent(event);
}


void PhotoDetailDialog::toggleFullscreen() 
{
    // Toggle between fullscreen and normal
    isFullscreen ? showNormal() : showFullScreen();
    fullscreenBtn->setText(isFullscreen ? "Fullscreen" : "Exit Fullscreen");
    isFullscreen = !isFullscreen;

    // Adjust zoom for new window size
    if (!originalPixmap.isNull()) 
    {
        double scale = fitScale(scrollArea->viewport()->size(), originalPixmap);
        zoomSlider->blockSignals(true);
        zoomSlider->setValue(static_cast<int>(scale * 100));
        zoomSlider->blockSignals(false);
        zoomChanged(static_cast<int>(scale * 100));
    }
}


void PhotoDetailDialog::zoomChanged(int value) 
{
    if (originalPixmap.isNull()) return;

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
