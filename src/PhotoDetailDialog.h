#pragma once
#include <QDialog>
#include <QShowEvent>
#include <QWheelEvent>
#include "Photo.h"

class QLabel;
class QScrollArea;
class QSlider;
class QPushButton;

class PhotoDetailDialog : public QDialog {
    Q_OBJECT
public:
    PhotoDetailDialog(QWidget* parent = nullptr);
    void setPhoto(const Photo& photo);

protected:
    void showEvent(QShowEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void zoomChanged(int value);
    void toggleFullscreen();

private:
    QLabel* imageLabel;
    QScrollArea* scrollArea;
    QSlider* zoomSlider;
    QPushButton* fullscreenBtn;
    QPixmap originalPixmap;
    Photo currentPhoto;
    bool isFullscreen;
};