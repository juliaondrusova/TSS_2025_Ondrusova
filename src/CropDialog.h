#pragma once

#include <QDialog>
#include <QPixmap>
#include <QRect>

class QLabel;
class QPushButton;
class QComboBox;

class CropDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CropDialog(const QPixmap& source, QWidget* parent = nullptr);

    QPixmap croppedPixmap() const;

private slots:
    void applyCrop();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void buildUI();
    void updatePreview();

    // data
    QPixmap m_sourcePixmap;
    QPixmap m_croppedPixmap;
    QRect   m_cropRect;      // v súradniciach zobrazeného obrázka
    double  m_aspectRatio;   // 0 = free

    // ui
    QLabel* imageLabel;
    QWidget* overlay;

    QPushButton* applyBtn;
    QPushButton* cancelBtn;
    QComboBox* ratioCombo;
};
