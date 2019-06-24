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
	Texture(const Texture&) = delete;
	Texture(Texture &&rhs)
	{
		w = rhs.w;
		h = rhs.h;
		x = rhs.x;
		y = rhs.y;
		img = std::move(rhs.img);
		raw = std::move(rhs.raw);
	}
	Texture(const Targa &tga, int xc, int yc)
		: w(tga.get_width())
		, h(tga.get_height())
		, x(xc)
		, y(yc)
	{
		raw.reset(new unsigned char[w * h * 4]);
		tga.get_bitmap(raw.get());
		img.reset(new QImage(raw.get(), w, h, QImage::Format_RGBA8888_Premultiplied));
		img->operator=(img->mirrored(false, true));
	}
	Texture &operator=(Texture &&rhs)
	{
		w = rhs.w;
		h = rhs.h;
		x = rhs.x;
		y = rhs.y;
		raw = std::move(rhs.raw);
		img = std::move(rhs.img);

		return *this;
	}

	bool collide(const Texture &other, int padding) const
	{
		return x + w + padding > other.x && x < other.x + other.w + padding && y + h + padding > other.y && y < other.y + other.h + padding;
	}

	void replace_img(const Targa &tga)
	{
		w = tga.get_width();
		h = tga.get_height();
		raw.reset(new unsigned char[w * h * 4]);
		tga.get_bitmap(raw.get());
		img.reset(new QImage(raw.get(), w, h, QImage::Format_RGBA8888_Premultiplied));
		img->operator=(img->mirrored(false, true));
	}

	void correct(const Texture &other, int padding)
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
			y = other.y + other.h + padding;
		else if(smallest == left_diff)
			x = other.x + other.w + padding;
		else if(smallest == right_diff)
			x = other.x - w - padding;
		else if(smallest == bottom_diff)
			y = other.y - h - padding;
	}

	static bool colliding(const Texture &check, const std::vector<std::pair<std::string, Texture>> &list, int padding)
	{
		for(auto &[_, test] : list)
		{
			if(&check == &test)
				continue;

			if(check.collide(test, padding))
				return true;
		}

		return false;
	}

	static bool colliding(const std::vector<std::pair<std::string, Texture>> &list, int padding)
	{
		for(auto &[_, test] : list)
		{
			if(colliding(test, list, padding))
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
	ArrangerPanel(int);

	void add(const std::string&);
	void add(const std::string&, int, int);
	int get_largest_index() const;
	void reload(int);
	std::vector<std::string> get_list() const;
	void flip();
	void clear();
	void remove(int);
	int move_up(int);
	int move_down(int);
	void set_align(bool);
	void set_border(bool);
	bool get_border() const;
	std::string info() const;
	std::vector<Entry> get_entries() const;

protected:
	virtual void paintEvent(QPaintEvent*) override;
	virtual void mousePressEvent(QMouseEvent*) override;
	virtual void mouseMoveEvent(QMouseEvent*) override;
	virtual void contextMenuEvent(QContextMenuEvent*) override;

private:
	void pack_left();
	void pack_up();
	void pack(bool);
	int atlas_width() const;
	int atlas_height() const;

	bool align;
	bool border;
	std::vector<std::pair<std::string, Texture>> textures;
	struct { std::string name; int anchor_x, anchor_y; } active;
	struct { bool adding; std::string filename; } adding;

	QAction *packleft;
	QAction *packup;
	int padding;
};

#endif
