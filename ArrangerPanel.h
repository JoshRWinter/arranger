#ifndef ARRANGERPANEL_H
#define ARRANGERPANEL_H

#include <unordered_map>
#include <memory>
#include <vector>
#include <cmath>

#include <QWidget>
#include <QImage>

#include "Targa.h"
#include "press.h"

struct Texture
{
	Texture() : w(0), h(0), x(0), y(0) {}
	Texture(const Targa &tga, int xc, int yc)
		: w(tga.get_width())
		, h(tga.get_height())
		, x(xc)
		, y(yc)
	{
		raw.reset(new unsigned char[w * h * 4]);
		tga.get_bitmap(raw.get());
		img.reset(new QImage(raw.get(), w, h, QImage::Format_RGBA8888_Premultiplied));
		QTransform tr;
		tr.rotate(180);
		img.get()->operator=(img.get()->transformed(tr));
	}

	bool collide(const Texture &other) const
	{
		return x + w > other.x && x < other.x + other.w && y + h > other.y && y < other.y + other.h;
	}

	void correct(const Texture &other)
	{
		const float top_diff = std::abs(y - (other.y + other.h));
		const float left_diff = std::abs(x - (other.x + other.w));
		const float right_diff = std::abs((x + w) - other.x);
		const float bottom_diff = std::abs((y + h) - other.y);

		float smallest = top_diff;
		if(left_diff < smallest)
			smallest = left_diff;
		if(right_diff < smallest)
			smallest = right_diff;
		if(bottom_diff < smallest)
			smallest = bottom_diff;

		if(smallest == top_diff)
			y = other.y + other.h;
		else if(smallest == left_diff)
			x = other.x + other.w;
		else if(smallest == right_diff)
			x = other.x - w;
		else if(smallest == bottom_diff)
			y = other.y - h;
	}

	static bool colliding(const Texture &check, const std::unordered_map<std::string, Texture> &list)
	{
		for(auto &[_, test] : list)
		{
			if(&check == &test)
				continue;

			if(check.collide(test))
				return true;
		}

		return false;
	}

	static bool colliding(const std::unordered_map<std::string, Texture> &list)
	{
		for(auto &[_, test] : list)
		{
			if(colliding(test, list))
				return true;
		}

		return false;
	}

	std::unique_ptr<unsigned char[]> raw;
	std::unique_ptr<QImage> img;
	int w;
	int h;
	int x;
	int y;
};

struct Entry
{
	Entry(const std::string &n, int xx, int yy) : name(n), x(xx), y(yy) {}

	std::string name;
	int x;
	int y;
};

class ArrangerPanel : public QWidget
{
public:
	ArrangerPanel();

	void add(const std::string&);
	void remove(const std::string&);
	void set_align(bool);
	void pack(bool);
	void set_border(bool);
	bool get_border() const;
	std::string info() const;
	std::vector<Entry> get_entries() const;

protected:
	virtual void paintEvent(QPaintEvent*) override;
	virtual void mousePressEvent(QMouseEvent*) override;
	virtual void mouseMoveEvent(QMouseEvent*) override;

private:
	bool align;
	bool border;
	std::unordered_map<std::string, Texture> textures;
	struct { std::string name; int anchor_x, anchor_y; } active;
};

#endif
