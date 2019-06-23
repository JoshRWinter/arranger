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
}

void ArrangerPanel::add(const std::string &filename)
{
	add(filename, 0, 0);
}

void ArrangerPanel::add(const std::string &filename, int x, int y)
{
	if(x < 0 || y < 0)
		throw std::runtime_error("invalid placement");

	if(textures.find(filename) != textures.end())
		throw std::runtime_error("Already exists");

	Targa tga(filename.c_str());

	std::pair<std::string, Texture> pair(filename, std::move(Texture(tga, x, y)));
	textures.insert(std::move(pair));

	repaint();
}

void ArrangerPanel::reload(const std::string &filename)
{
	auto it = textures.find(filename);
	if(it == textures.end())
		throw std::runtime_error(filename + " doesn't exist");

	Targa tga(filename.c_str());
	it->second.replace_img(tga);

	repaint();
}

void ArrangerPanel::flip()
{
	int largest_y = 0;
	for(std::pair<const std::string, Texture> &item : textures)
	{
		if(item.second.y + item.second.h > largest_y)
			largest_y = item.second.y + item.second.h;
	}

	for(std::pair<const std::string, Texture> &item : textures)
	{
		item.second.y = largest_y - (item.second.y + item.second.h);
		if(item.second.y < 0)
			fprintf(stderr, "ERROR ----------------------- %s is at %d\n", item.first.c_str(), item.second.y);
	}

	repaint();
}

void ArrangerPanel::clear()
{
	textures.clear();
	repaint();
}

void ArrangerPanel::remove(const std::string &name)
{
	for(auto it = textures.begin(); it != textures.end(); ++it)
	{
		if(it->first == name)
		{
			textures.erase(it);
			active.name = "";
			repaint();
			return;
		}
	}

	throw std::runtime_error("Couldn't find texture \"" + name + "\"");
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

	// draw bounding box
	painter.setPen(QColor(255, 0, 255));
	painter.drawRect(QRect(QPoint(0, 0), QPoint(maxwidth, maxheight)));

#if 0
	// draw active texture line
	if(active.name.length() > 0)
		painter.drawText(QPoint(0, painter.fontMetrics().height()), press::swrite("Active Texture: {} [{}x{}]{}", active.name, textures[active.name].w, textures[active.name].h, align ? " ALIGNED" : "").c_str());
#endif
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

void ArrangerPanel::pack_left()
{
	pack(true);
}

void ArrangerPanel::pack_up()
{
	pack(false);
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

void ArrangerPanel::pack(bool left)
{
	if(Texture::colliding(textures, padding))
	{
		QMessageBox::critical(this, "Can't pack", "There are collisions!");
		return;
	}

	int moved = 0;
	do
	{
		moved = 0;
		for(auto &[_, current] : textures)
		{
			while(!Texture::colliding(current, textures, padding) && current.x >= padding && current.y >= padding)
			{
				++moved;
				if(left)
					--current.x;
				else
					--current.y;
			}

			--moved;
			if(left)
				++current.x;
			else
				++current.y;
		}
	}while(moved);

	repaint();
}
