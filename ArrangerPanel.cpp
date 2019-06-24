#include <QMenu>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QAction>

#include "ArrangerPanel.h"

ArrangerPanel::ArrangerPanel(int pad)
	: align(true)
	, border(true)
	, padding(pad)
{
	setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, QColor(220, 220, 220));
	setPalette(palette);
	setFocusPolicy(Qt::StrongFocus);
	resize(1000, 1000);

	packleft = new QAction("Pack &Left", this);
	packup = new QAction("Pack &Up", this);

	QObject::connect(packleft, &QAction::triggered, this, &ArrangerPanel::pack_left);
	QObject::connect(packup, &QAction::triggered, this, &ArrangerPanel::pack_up);

	active.anchor_x = 0;
	active.anchor_y = 0;

	adding.adding = false;
}

void ArrangerPanel::add(const std::string &filename)
{
	for(const auto &[name, _] : textures)
		if(name == filename)
			throw std::runtime_error("Already exists");

	adding.adding = true;
	adding.filename = filename;

	repaint();
}

void ArrangerPanel::add(const std::string &filename, int x, int y)
{
	if(x < padding)
		x = padding;
	if(y < padding)
		y = padding;

	for(const auto &[name, _] : textures)
		if(name == filename)
			throw std::runtime_error("Already exists");

	Targa tga(filename.c_str());
	Texture tex(tga, x, y);
	std::pair<std::string, Texture> pair(filename, std::move(tex));
	textures.push_back(std::move(pair));

	repaint();
}

int ArrangerPanel::get_largest_index() const
{
	return textures.size() - 1;
}

void ArrangerPanel::reload(int i)
{
	if(i >= (int)textures.size() || i < 0)
		throw new std::runtime_error("no index " + std::to_string(i));

	auto it = textures.begin() + i;

	Targa tga(it->first.c_str());
	it->second.replace_img(tga);

	repaint();
}

std::vector<std::string> ArrangerPanel::get_list() const
{
	std::vector<std::string> items;

	for(int i = 0; i < (int)textures.size(); ++i)
	{
		items.push_back(std::to_string(i) + ": " + textures[i].first);
	}

	if(adding.adding)
		items.push_back(std::to_string(textures.size()) + ": " + adding.filename);

	return items;
}

void ArrangerPanel::flip()
{
	int largest_y = 0;
	for(const auto &[name, tex] : textures)
	{
		if(tex.y + tex.h > largest_y)
			largest_y = tex.y + tex.h;
	}

	for(auto &[name, tex] : textures)
	{
		tex.y = largest_y - (tex.y + tex.h);
		if(tex.y < 0)
			fprintf(stderr, "ERROR ----------------------- %s is at %d\n", name.c_str(), tex.y);
	}

	repaint();
}

void ArrangerPanel::clear()
{
	textures.clear();
	repaint();
}

void ArrangerPanel::remove(int index)
{
	if(index >= (int)textures.size() || index < 0)
		throw std::runtime_error("Couldn't find texture index \"" + std::to_string(index) + "\"");

	textures.erase(textures.begin() + index);

	active.name = "";
	repaint();
}

int ArrangerPanel::move_up(int index)
{
	if(index < 1)
		return index;

	std::pair<std::string, Texture> temp = std::move(textures[index - 1]);
	textures[index - 1] = std::move(textures[index]);
	textures[index] = std::move(temp);

	return index - 1;
}

int ArrangerPanel::move_down(int index)
{
	if(index >= ((int)textures.size()) - 1)
		return index;

	std::pair<std::string, Texture> temp = std::move(textures[index]);
	textures[index] = std::move(textures[index + 1]);
	textures[index + 1] = std::move(temp);

	return index + 1;
}

void ArrangerPanel::set_align(bool yes)
{
	align = yes;
	repaint();
}

void ArrangerPanel::set_border(bool on)
{
	border = on;
	repaint();
}

bool ArrangerPanel::get_border() const
{
	return border;
}

std::string ArrangerPanel::info() const
{
	int maxwidth = atlas_width();
	int maxheight = atlas_height();

	char sizestr[50];
	snprintf(sizestr, sizeof(sizestr), "%.2f", (maxwidth * maxheight * 4) / 1024.0 / 1024.0);

	return "[" + std::to_string(maxwidth) + "x" + std::to_string(maxheight) + "], " +
	sizestr + "MB";
}
std::vector<Entry> ArrangerPanel::get_entries() const
{
	std::vector<Entry> entries;

	for(const auto &[name, texture] : textures)
		entries.push_back(Entry(name, texture.x, atlas_height() - texture.y - texture.h + padding));

	return entries;
}

bool ArrangerPanel::pack_left()
{
	return pack(true);
}

bool ArrangerPanel::pack_up()
{
	return pack(false);
}

void ArrangerPanel::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	int maxwidth = atlas_width();
	int maxheight = atlas_height();

	for(const auto &[name, texture] : textures)
	{
		painter.drawImage(QPoint(texture.x, texture.y), *texture.img.get());

		if(border)
		{
			if(active.name == name)
				painter.setPen(QColor(255, 0, 0));
			else
				painter.setPen(QColor(0, 0, 0));
			painter.drawRect(QRect(QPoint(texture.x, texture.y), QPoint(texture.x + texture.w - 1, texture.y + texture.h - 1)));
		}
	}

	if(textures.size() > 0)
	{
		// draw bounding box
		painter.setPen(QColor(255, 0, 255));
		painter.drawRect(QRect(QPoint(0, 0), QPoint(maxwidth, maxheight)));
	}

#if 0
	// draw active texture line
	if(active.name.length() > 0)
		painter.drawText(QPoint(0, painter.fontMetrics().height()), press::swrite("Active Texture: {} [{}x{}]{}", active.name, textures[active.name].w, textures[active.name].h, align ? " ALIGNED" : "").c_str());
#endif

	// add mode
	if(adding.adding)
	{
		QColor color(200, 0, 200, 100);
		painter.setPen(color);
		painter.setBrush(color);
		painter.drawRect(QRect(QPoint(0, 0), QPoint(width(), height())));
	}
}

void ArrangerPanel::mousePressEvent(QMouseEvent *event)
{
	const int x = event->pos().x();
	const int y = event->pos().y();

	if(adding.adding)
	{
		adding.adding = false;
		add(adding.filename, x, y);
		return;
	}

	if(textures.size() > 0)
	{
		for(auto it = textures.end() - 1; it >= textures.begin(); --it)
		{
			Texture &tex = it->second;
			const std::string &name = it->first;

			if(x > tex.x && x < tex.x + tex.w && y > tex.y && y < tex.y + tex.h)
			{
				active.name = name;
				active.anchor_x = x - tex.x;
				active.anchor_y = y - tex.y;
				repaint();
				return;
			}
		}
	}

	active.name = "";
	repaint();
}

void ArrangerPanel::mouseMoveEvent(QMouseEvent *event)
{
	if(active.name.length() == 0)
		return;

	auto it = textures.begin();
	for(;it != textures.end(); it++)
		if(it->first == active.name)
			break;
	Texture &tex = it->second;

	tex.x = event->pos().x() - active.anchor_x;
	tex.y = event->pos().y() - active.anchor_y;

	// correct collisions
	if(align)
	{
		for(auto &[_, focus] : textures)
		{
			if(&tex == &focus)
				continue;

			if(tex.collide(focus, padding))
				tex.correct(focus, padding);
		}
	}

	if(tex.x < padding)
		tex.x = padding;
	if(tex.y < padding)
		tex.y = padding;

	repaint();
}

void ArrangerPanel::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu m(this);
	m.addAction(packleft);
	m.addAction(packup);
	m.exec(event->globalPos());
}

int ArrangerPanel::atlas_width() const
{
	int width = 0;

	for(const auto &[_, tex] : textures)
	{
		if(tex.x + tex.w > width)
			width = tex.x + tex.w;
	}

	return width;
}

int ArrangerPanel::atlas_height() const
{
	int height = 0;

	for(const auto &[_, tex] : textures)
	{
		if(tex.y + tex.h > height)
			height = tex.y + tex.h;
	}

	return height;
}

bool ArrangerPanel::pack(bool left)
{
	// double check there are no textures out of bounds
	for(const auto &[n, t] : textures)
	{
		if(t.x < padding || t.y < padding)
		{
			QMessageBox::critical(this, "Can't pack", (n + " is out of bounds!").c_str());
			return false;
		}
	}

	if(Texture::colliding(textures, padding))
	{
		QMessageBox::critical(this, "Can't pack", "There are collisions!");
		return false;
	}

	bool changed = false;
	int moved = 0;
	do
	{
		moved = 0;
		for(auto &[_, current] : textures)
		{
			const int original_x = current.x;
			const int original_y = current.y;

			// move it till it doesn't fit
			while(!Texture::colliding(current, textures, padding) && current.x >= padding && current.y >= padding)
			{
				++moved;
				if(left)
					--current.x;
				else
					--current.y;
			}

			// make it fit again
			--moved;
			if(left)
				++current.x;
			else
				++current.y;

			if(original_x != current.x || original_y != current.y)
				changed = true;
		}
	}while(moved);

	repaint();

	return changed;
}
