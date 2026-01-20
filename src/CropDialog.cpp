#include "CropDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QPainter>
#include <QMouseEvent>
#include <QStyle>

// ===== Overlay widget: Handles the visual crop UI and mouse interaction =====

class CropOverlay : public QWidget
{
public:
    // Enumeration to identify which part of the crop rectangle the user is interacting with
    enum Handle { None, TopLeft, TopRight, BottomLeft, BottomRight, Top, Bottom, Left, Right, Move };

    explicit CropOverlay(QWidget* parent = nullptr)
        : QWidget(parent), m_aspectRatio(0.0), m_activeHandle(None)
    {
        // Enable mouse tracking to change cursor icons even when no button is pressed
        setMouseTracking(true);
    }

    // Setters for image data provided by the parent dialog
	void setSourceSize(QSize size) { m_sourceSize = size; } // Original full-resolution image size
    void setPixmapRect(QRect r) { m_pixmapRect = r; }      // Geometry of the scaled image on screen
    void setFullSize(const QSize& size) { m_fullSize = size; update(); }
    QSize fullSize() const { return m_fullSize; }

    // Updates the aspect ratio and recalculates the rectangle to stay centered
    void setFixedAspectRatio(double ratio)
    {
		m_aspectRatio = ratio; // current aspect ratio (0 = free)

        if (m_aspectRatio > 0 && !m_pixmapRect.isEmpty())
        {
            // Calculate a default size (80% of image width)
            int newW = m_pixmapRect.width() * 0.8;
            int newH = qRound(newW / m_aspectRatio);

            // If calculated height exceeds available image height, shrink width instead
            if (newH > m_pixmapRect.height() * 0.8) 
            {
                newH = m_pixmapRect.height() * 0.8;
                newW = qRound(newH * m_aspectRatio);
            }

            // Center the new rectangle within the displayed image bounds
            int cx = m_pixmapRect.center().x();
            int cy = m_pixmapRect.center().y();

			// Set the crop rectangle
            m_cropRect = QRect(cx - newW / 2, cy - newH / 2, newW, newH);
        }
        update(); // Trigger a repaint
    }

	void setInitialRect(const QRect& r) { m_cropRect = r; update(); } // Set initial crop rectangle
	QRect cropRect() const { return m_cropRect.normalized(); } // Get the current crop rectangle

    QRect m_pixmapRect;     // Coordinates of the scaled image within the overlay

protected:
    // Core drawing logic for the darkening effect and crop handles
    void paintEvent(QPaintEvent*) override
    {
		if (m_pixmapRect.isEmpty()) return; // No image to overlay on

        QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing); // Smooth edges

		QRect normRect = m_cropRect.normalized(); // Ensure rectangle is normalized

        // Create the "dimmed" background effect using regions
        QRegion backgroundRegion(rect());
        backgroundRegion -= QRegion(normRect); // Subtract the crop area from the total widget area

		p.setClipRegion(backgroundRegion); // Clip to the background region
        p.fillRect(rect(), QColor(0, 0, 0, 150)); // 150 = semi-transparent black
		p.setClipping(false); // Disable clipping for further drawing

        // Draw the white border of the crop box
        p.setPen(QPen(Qt::white, 2));
        p.drawRect(normRect);

        // Draw the white square handles
        p.setBrush(Qt::white);
        p.setPen(QPen(Qt::black, 1));

		// Get all handle rectangles
        QList<QRect> handles = getHandles(normRect);

        // If aspect ratio is fixed, only draw the 4 corner handles (indices 0-3)
        for (int i = 0; i < (m_aspectRatio > 0 ? 4 : handles.size()); ++i) 
        {
            p.drawRect(handles[i]);
        }

        // Draw text showing real-world pixel dimensions
        if (m_pixmapRect.width() > 0) 
        {
			// Calculate scaling factors between displayed image and original image
            double scaleX = double(m_fullSize.width()) / m_pixmapRect.width();
            double scaleY = double(m_fullSize.height()) / m_pixmapRect.height();
            int realW = qRound(normRect.width() * scaleX);
            int realH = qRound(normRect.height() * scaleY);

            QString sizeText = QString("%1 x %2 px").arg(realW).arg(realH);
            QPoint textPos = normRect.topLeft() + QPoint(0, -10);
            p.setPen(Qt::black); // Shadow for readability
			p.drawText(textPos + QPoint(1, 1), sizeText); // Shadow
			p.setPen(Qt::yellow); // Main text
            p.drawText(textPos, sizeText);
        }
    }

	// Determine which handle (if any) the mouse is over
    void mousePressEvent(QMouseEvent* e) override 
    {
		// Identify which handle is being interacted with
        m_activeHandle = hitTest(e->pos());
		m_lastPos = e->pos(); // Store the last mouse position

        // If user clicks outside the box but inside the image, start a new 1x1 rect
        if (m_activeHandle == None && m_pixmapRect.contains(e->pos())) 
        {
            m_cropRect = QRect(e->pos(), QSize(1, 1));
			m_activeHandle = BottomRight; // Start resizing from bottom-right
        }
        update();
    }

    void mouseMoveEvent(QMouseEvent* e) override
    {
        QPoint mousePos = e->pos(); // Current mouse position

        // --- 1. Only move/resize if left button is pressed and a handle is active ---
        if ((e->buttons() & Qt::LeftButton) && m_activeHandle != None)
        {
            // Clamp mouse position to stay inside the image boundaries
            mousePos.setX(qBound(m_pixmapRect.left(), mousePos.x(), m_pixmapRect.right()));
            mousePos.setY(qBound(m_pixmapRect.top(), mousePos.y(), m_pixmapRect.bottom()));

            QRect newRect = m_cropRect.normalized(); // Start with current crop rectangle

            // --- 2. Move the entire rectangle ---
            if (m_activeHandle == Move)
            {
                QPoint delta = mousePos - m_lastPos; // How much the mouse moved
                newRect.translate(delta);

                // Keep rectangle inside image boundaries
                if (newRect.left() < m_pixmapRect.left()) newRect.moveLeft(m_pixmapRect.left());
                if (newRect.top() < m_pixmapRect.top()) newRect.moveTop(m_pixmapRect.top());
                if (newRect.right() > m_pixmapRect.right()) newRect.moveRight(m_pixmapRect.right());
                if (newRect.bottom() > m_pixmapRect.bottom()) newRect.moveBottom(m_pixmapRect.bottom());
            }
            else
            {
                // --- 3. Resize based on active handle ---
                switch (m_activeHandle)
                {
                case TopLeft:     newRect.setTopLeft(mousePos); break;
                case TopRight:    newRect.setTopRight(mousePos); break;
                case BottomLeft:  newRect.setBottomLeft(mousePos); break;
                case BottomRight: newRect.setBottomRight(mousePos); break;

                // Side handles only work if aspect ratio is free
                case Top:    if (m_aspectRatio <= 0) newRect.setTop(mousePos.y()); break;
                case Bottom: if (m_aspectRatio <= 0) newRect.setBottom(mousePos.y()); break;
                case Left:   if (m_aspectRatio <= 0) newRect.setLeft(mousePos.x()); break;
                case Right:  if (m_aspectRatio <= 0) newRect.setRight(mousePos.x()); break;
                default: break;
                }

				// --- 4. Enforce aspect ratio if enabled ---
                if (m_aspectRatio > 0)
                {
                    int width = newRect.width();
                    int height = qRound(width / m_aspectRatio); // Calculate height from width

                    // Adjust top/bottom depending on corner handle
                    if (m_activeHandle == TopLeft || m_activeHandle == TopRight)
                        newRect.setTop(newRect.bottom() - height); // move top edge
                    else
                        newRect.setBottom(newRect.top() + height); // move bottom edge

                    // Make sure rectangle stays inside image
                    if (newRect.top() < m_pixmapRect.top() || newRect.bottom() > m_pixmapRect.bottom())
                    {
                        // If dragging top-left or top-right, max height = distance from bottom to top of image
                        // Otherwise (bottom-left/bottom-right), max height = distance from top to bottom of image
                        int maxHeight = (m_activeHandle == TopLeft || m_activeHandle == TopRight)
                            ? newRect.bottom() - m_pixmapRect.top()
                            : m_pixmapRect.bottom() - newRect.top();

                        //Calculate the corresponding width for this max height to maintain aspect ratio
                        int maxWidth = qRound(maxHeight * m_aspectRatio);

                        // Adjust the left or right side to match the max width
                        // If dragging left corners, move left edge; else move right edge
                        if (m_activeHandle == TopLeft || m_activeHandle == BottomLeft)
                            newRect.setLeft(newRect.right() - maxWidth);
                        else
                            newRect.setRight(newRect.left() + maxWidth);

                        // Recalculate final height based on the new width (still keeping aspect ratio)
                        int finalHeight = qRound(newRect.width() / m_aspectRatio);

                        // Adjust top or bottom edge to match final height
                        // Top corners: move top; Bottom corners: move bottom
                        if (m_activeHandle == TopLeft || m_activeHandle == TopRight)
                            newRect.setTop(newRect.bottom() - finalHeight);
                        else
                            newRect.setBottom(newRect.top() + finalHeight);
                    }
                }
            }

            // --- 5. Update crop rectangle and last mouse position ---
            m_cropRect = newRect.normalized();
            m_lastPos = mousePos;
            update();
        }
        else
        {
            // --- 6. Update cursor based on hover position ---
            Handle h = hitTest(mousePos);
            switch (h)
            {
				// Show 'move' cursor when hovering inside the box
			    case Move: setCursor(Qt::SizeAllCursor); break; 

                // Show diagonal resize(\) cursor
                case TopLeft:
                case BottomRight: setCursor(Qt::SizeFDiagCursor); break;

				// Show diagonal resize(/) cursor
                case TopRight:
			    case BottomLeft: setCursor(Qt::SizeBDiagCursor); break; 

                case Top:
                case Bottom:
                    if (m_aspectRatio > 0)
                        setCursor(Qt::ArrowCursor); // Fixed aspect ratio - don't allow vertical resizing
                    else
                        setCursor(Qt::SizeVerCursor); // Free aspect ratio - vertical resize
                    break;

                case Left:
                case Right:
                    if (m_aspectRatio > 0)
                        setCursor(Qt::ArrowCursor); // Fixed aspect ratio - don't allow horizontal resizing
                    else
                        setCursor(Qt::SizeHorCursor); // Free aspect ratio - horizontal resize
                    break;

                default:
                    setCursor(Qt::CrossCursor); // Mouse is outside any handle - show crosshair cursor
                    break;
            }
        }
    }


private:
    // Helper to calculate the 8 handle rectangles (4 corners + 4 centers of sides)
    QList<QRect> getHandles(const QRect& r) const 
    {
        int s = 8; // Handle size in pixels
        return 
        {
			// Corner handles
            QRect(r.left() - s / 2, r.top() - s / 2, s, s),
            QRect(r.right() - s / 2, r.top() - s / 2, s, s),
            QRect(r.left() - s / 2, r.bottom() - s / 2, s, s),
            QRect(r.right() - s / 2, r.bottom() - s / 2, s, s),
			// Handles for sides
            QRect(r.center().x() - s / 2, r.top() - s / 2, s, s),
            QRect(r.center().x() - s / 2, r.bottom() - s / 2, s, s),
            QRect(r.left() - s / 2, r.center().y() - s / 2, s, s),
            QRect(r.right() - s / 2, r.center().y() - s / 2, s, s)
        };
    }

    // Logic to detect if a point (mouse) is over a handle or inside the box
    Handle hitTest(QPoint p) const 
    {
		QList<QRect> hs = getHandles(m_cropRect.normalized()); // Get handle rectangles

        if (hs[0].contains(p)) return TopLeft;
        if (hs[1].contains(p)) return TopRight;
        if (hs[2].contains(p)) return BottomLeft;
        if (hs[3].contains(p)) return BottomRight;

        if (m_aspectRatio <= 0) // Side handles disabled in fixed ratio modes
        { 
            if (hs[4].contains(p)) return Top;
            if (hs[5].contains(p)) return Bottom;
            if (hs[6].contains(p)) return Left;
            if (hs[7].contains(p)) return Right;
        }
        if (m_cropRect.normalized().contains(p)) return Move;
        return None;
    }

    QRect m_cropRect;       // Current crop rectangle in overlay coordinates
    QPoint m_lastPos;       // Previous mouse position for delta calculation
    Handle m_activeHandle;  // Part currently being dragged
    double m_aspectRatio;   // Width/Height ratio (0 = free)
    QSize m_sourceSize;     // Full original resolution of the image
	QSize m_fullSize;	   // Full original image size for reference
};


// ===== CropDialog Implementation =====

CropDialog::CropDialog(const QPixmap& source, const QSize& originalSize, QWidget* parent)
    : QDialog(parent), m_sourcePixmap(source), m_originalSize(originalSize)
{
    setWindowTitle("Crop Image");
    resize(800, 600);
    buildUI();
}

void CropDialog::buildUI() 
{
    // --- Main layout ---
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // --- Image display ---
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);

    // --- Overlay for cropping ---
    overlay = new CropOverlay(imageLabel);
    CropOverlay* cropOverlay = static_cast<CropOverlay*>(overlay);
    cropOverlay->setSourceSize(m_sourcePixmap.size());
	cropOverlay->setFullSize(m_originalSize);

    // --- Aspect ratio selection row ---
    QHBoxLayout* aspectLayout = new QHBoxLayout;
    QPushButton* btnFree = new QPushButton("Free", this);
    QPushButton* btn169 = new QPushButton("16/9", this);
    QPushButton* btn43 = new QPushButton("4/3", this);
    QPushButton* btn11 = new QPushButton("1/1", this);

    // Add buttons to the layout
    aspectLayout->addWidget(btnFree);
    aspectLayout->addWidget(btn169);
    aspectLayout->addWidget(btn43);
    aspectLayout->addWidget(btn11);
    aspectLayout->addStretch(); // push buttons to the left

    // --- Bottom row with Apply/Cancel buttons ---
    QHBoxLayout* bottom = new QHBoxLayout;
    applyBtn = new QPushButton("Apply", this);
    cancelBtn = new QPushButton("Cancel", this);
    bottom->addStretch(); // push buttons to the right
    bottom->addWidget(cancelBtn);
    bottom->addWidget(applyBtn);

    // --- Add everything to the main layout ---
    mainLayout->addWidget(imageLabel, 1);   // image takes most space
    mainLayout->addLayout(aspectLayout);    // aspect ratio buttons
    mainLayout->addLayout(bottom);          // action buttons

    // --- Connect aspect ratio buttons --
    connect(btnFree, &QPushButton::clicked, [=]() { cropOverlay->setFixedAspectRatio(0.0); });
    connect(btn169, &QPushButton::clicked, [=]() { cropOverlay->setFixedAspectRatio(16.0 / 9.0); });
    connect(btn43, &QPushButton::clicked, [=]() { cropOverlay->setFixedAspectRatio(4.0 / 3.0); });
    connect(btn11, &QPushButton::clicked, [=]() { cropOverlay->setFixedAspectRatio(1.0); });

    // -- - Connect Apply / Cancel buttons-- -
    connect(applyBtn, &QPushButton::clicked, this, &CropDialog::applyCrop);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void CropDialog::updatePreview() 
{
    if (m_sourcePixmap.isNull()) return;

    // --- Scale the source image to fit the label while keeping aspect ratio ---
    QPixmap scaled = m_sourcePixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(scaled);

    // --- Calculate the rectangle where the pixmap is actually drawn inside the label ---
    QRect pixRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, scaled.size(), imageLabel->rect());

    overlay->setGeometry(imageLabel->rect()); // overlay covers the whole label

    CropOverlay* cropOverlay = static_cast<CropOverlay*>(overlay);
    cropOverlay->setPixmapRect(pixRect); // tell overlay where image is

    // --- Initialize default center crop (50% of visible image) ---
    int w = pixRect.width() / 2;
    int h = pixRect.height() / 2;
    int x = pixRect.center().x() - w / 2;
    int y = pixRect.center().y() - h / 2;
    cropOverlay->setInitialRect(QRect(x, y, w, h)); // center crop
}

void CropDialog::showEvent(QShowEvent* event) 
{
    QDialog::showEvent(event);
    updatePreview(); // update image & crop overlay whenever dialog is shown
}

void CropDialog::applyCrop()
{
    CropOverlay* o = static_cast<CropOverlay*>(overlay);
    QRect crop = o->cropRect(); // get crop rectangle in overlay coords
    QPixmap displayed = imageLabel->pixmap();
    if (displayed.isNull() || crop.isEmpty()) return;

    QRect pixRect = o->m_pixmapRect;

    double scaleX = double(o->fullSize().width()) / pixRect.width();
    double scaleY = double(o->fullSize().height()) / pixRect.height();

    QRect finalRect(
        qRound((crop.x() - pixRect.x()) * scaleX),
        qRound((crop.y() - pixRect.y()) * scaleY),
        qRound(crop.width() * scaleX),
        qRound(crop.height() * scaleY)
    );
    finalRect = finalRect.intersected(QRect(QPoint(0, 0), m_originalSize));

    // Uloûiù normalizovanÈ crop koordin·ty**
    m_normalizedCropRect = QRectF(
        double(finalRect.x()) / m_originalSize.width(),
        double(finalRect.y()) / m_originalSize.height(),
        double(finalRect.width()) / m_originalSize.width(),
        double(finalRect.height()) / m_originalSize.height()
    );
   
    accept();
}

QPixmap CropDialog::applyCropToPixmap(const QPixmap& source, const QRectF& normalizedCrop)
{
    if (source.isNull() || normalizedCrop.isEmpty())
        return QPixmap();

    // Konvertovaù normalizovanÈ hodnoty na absol˙tne pixely
    QRect cropRect(
        qRound(normalizedCrop.x() * source.width()),
        qRound(normalizedCrop.y() * source.height()),
        qRound(normalizedCrop.width() * source.width()),
        qRound(normalizedCrop.height() * source.height())
    );

    // Intersection pre istotu
    cropRect = cropRect.intersected(source.rect());

    return source.copy(cropRect);
}