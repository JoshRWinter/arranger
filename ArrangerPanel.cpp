#include <QPainter>

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

	for(const auto &[_, texture] : textures)
	{
		painter.drawImage(QPoint(texture.x, texture.y), *texture.img.get());
	}
}
