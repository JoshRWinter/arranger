#include <QPainter>
#include <QMouseEvent>

#include "ArrangerPanel.h"

ArrangerPanel::ArrangerPanel()
{
	setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, QColor(220, 220, 220));
	setPalette(palette);
}

void ArrangerPanel::add(const std::string &filename)
{
	if(textures.find(filename) != textures.end())
		throw std::runtime_error("Already exists");

	Targa tga(filename.c_str());

	std::pair<std::string, Texture> pair(filename, std::move(Texture(tga, 0, height() - tga.get_height())));
	textures.insert(std::move(pair));

	repaint();
}

void ArrangerPanel::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	// draw active texture line
	painter.drawText(QPoint(0, painter.fontMetrics().height()), press::swrite("Active Texture: {}", active.name).c_str());

	for(const auto &[_, texture] : textures)
	{
		painter.drawImage(QPoint(texture.x, texture.y), *texture.img.get());
	}
}

void ArrangerPanel::mousePressEvent(QMouseEvent *event)
{
	const int x = event->pos().x();
	const int y = event->pos().y();

	for(const auto &[name, tex] : textures)
	{
		if(x > tex.x && x < tex.x + tex.w && y > tex.y && y < tex.y + tex.h)
		{
			active.name = name;
			active.anchor_x = x - tex.x;
			active.anchor_y = y - tex.y;
			repaint();
			return;
		}
	}

	active.name = "";
	repaint();
}

void ArrangerPanel::mouseMoveEvent(QMouseEvent *event)
{
	if(active.name.length() == 0)
		return;

	Texture &tex = textures.find(active.name)->second;
	tex.x = event->pos().x() - active.anchor_x;
	tex.y = event->pos().y() - active.anchor_y;

	repaint();
}
