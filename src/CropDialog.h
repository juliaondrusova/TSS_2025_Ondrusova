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
	explicit CropDialog(const QPixmap& source, const QSize& originalSize, QWidget* parent = nullptr);
	QRectF normalizedCropRect() const { return m_normalizedCropRect; }
	static QPixmap applyCropToPixmap(const QPixmap& source, const QRectF& normalizedCrop);

private slots:
	void applyCrop();

protected:
	void showEvent(QShowEvent* event) override;

private:
	void buildUI();
	void updatePreview();

	// data
	QPixmap m_sourcePixmap;
	QRectF m_normalizedCropRect;
	QSize m_originalSize;

	// ui
	QLabel* imageLabel;
	QWidget* overlay;
	QPushButton* applyBtn;
	QPushButton* cancelBtn;
};
