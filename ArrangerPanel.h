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
		: w(tga.get_width())
		, h(tga.get_height())
		, x(xc)
		, y(yc)
	{
		raw.reset(new unsigned char[w * h * 4]);
		tga.get_bitmap(raw.get());
		img.reset(new QImage(raw.get(), w, h, QImage::Format_RGBA8888_Premultiplied));
	}

	std::unique_ptr<unsigned char[]> raw;
	std::unique_ptr<QImage> img;
	int w;
	int h;
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
	virtual void mousePressEvent(QMouseEvent*) override;
	virtual void mouseMoveEvent(QMouseEvent*) override;

private:
	std::unordered_map<std::string, Texture> textures;
	struct { std::string name; int anchor_x, anchor_y; } active;
};

#endif
