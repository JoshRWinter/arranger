#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>

#include "ArrangerPanel.h"

ArrangerPanel::ArrangerPanel()
	: align(true)
	, border(true)
{
	setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, QColor(220, 220, 220));
	setPalette(palette);
	setFocusPolicy(Qt::StrongFocus);
	resize(1000, 1000);
}

void ArrangerPanel::add(const std::string &filename)
{
	if(textures.find(filename) != textures.end())
		throw std::runtime_error("Already exists");

	Targa tga(filename.c_str());

	std::pair<std::string, Texture> pair(filename, std::move(Texture(tga, 0, 0)));
	textures.insert(std::move(pair));

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

void ArrangerPanel::pack(bool left)
{
	if(Texture::colliding(textures))
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
			while(!Texture::colliding(current, textures) && current.x >= 0 && current.y >= 0)
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

void ArrangerPanel::set_border(bool on)
{
	border = on;
	repaint();
}

bool ArrangerPanel::get_border() const
{
	return border;
}

std::vector<Entry> ArrangerPanel::get_entries() const
{
	std::vector<Entry> entries;

	for(const auto &[name, texture] : textures)
		entries.push_back(Entry(name, texture.x, texture.y));

	return entries;
}

void ArrangerPanel::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

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

			if(tex.collide(focus))
				tex.correct(focus);
		}
	}

	if(tex.x < 0)
		tex.x = 0;
	if(tex.y < 0)
		tex.y = 0;

	repaint();
}
