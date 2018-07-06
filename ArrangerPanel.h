#ifndef ARRANGERPANEL_H
#define ARRANGERPANEL_H

#include <unordered_map>
#include <memory>

#include <QWidget>
#include <QImage>

#include "Targa.h"
#include "press.h"

struct Texture
{
	Texture(const Targa &tga, int xc, int yc)
		: x(xc)
		, y(yc)
	{
		raw.reset(new unsigned char[tga.get_width() * tga.get_height() * 4]);
		tga.get_bitmap(raw.get());
		img.reset(new QImage(raw.get(), tga.get_width(), tga.get_height(), QImage::Format_RGBA8888_Premultiplied));
	}

	std::unique_ptr<unsigned char[]> raw;
	std::unique_ptr<QImage> img;
	int x;
	int y;
};

class ArrangerPanel : public QWidget
{
public:
	ArrangerPanel();

	void add(const std::string&);

protected:
	virtual void paintEvent(QPaintEvent*) override;

private:
	std::unordered_map<std::string, Texture> textures;
};

#endif
