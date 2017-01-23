#pragma once

#include <QWidget>
#include <QPainter>

class LabelWindow : public QWidget
{
	const int SHADOW_RADIUS = 20;
	QString _text;
	QColor _bg;
	QColor _fg;
public:
	LabelWindow() : QWidget(nullptr, Qt::Widget | Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowTransparentForInput)
	{
		_bg = QColor(255, 255, 255, 240);
		_fg = QColor(255, 95, 10);

		setParent(0); // Create TopLevel-Widget
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_TranslucentBackground);
		setAttribute(Qt::WA_AlwaysStackOnTop);
		//setAttribute(Qt::WA_DeleteOnClose);

		//auto* effect = new QGraphicsDropShadowEffect();
		//effect->setOffset(0, SHADOW_RADIUS / 4);
		//effect->setBlurRadius(SHADOW_RADIUS);
		//setGraphicsEffect(effect);
	}

	void setText(const QString& s)
	{
		_text = s;
		update();
	}

	void setColors(const QColor& bg, const QColor& fg, bool repaintNow = true)
	{
		_bg = bg;
		_fg = fg;
		if (repaintNow)
			repaint();
		else
			update();
	}

protected:
	void paintEvent(QPaintEvent* e) override
	{
		auto rec = rect();
		auto shadowOffsetY = 0;//((QGraphicsDropShadowEffect*)graphicsEffect())->yOffset();
		rec.adjust(SHADOW_RADIUS, SHADOW_RADIUS - shadowOffsetY, -SHADOW_RADIUS, -SHADOW_RADIUS - shadowOffsetY);

		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);

		QPainterPath path;
		//path.addRoundedRect(rec, corner, corner);
		path.addEllipse(rec);
		QPen pen(_fg, 2);
		painter.setPen(pen);
		painter.fillPath(path, _bg);
		painter.drawPath(path);

		painter.setPen(QColor(255, 95, 10));
		painter.setFont(QFont(QString::fromWCharArray(L"Consolas"), rec.height() / 3));

		painter.setPen(QColor(255, 255, 255));
		painter.drawText(rec.adjusted(0, 2, 0, 2), Qt::AlignCenter, _text);

		painter.setPen(_fg);
		painter.drawText(rec, Qt::AlignCenter, _text);
	}
};
