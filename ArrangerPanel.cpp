#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>

#include "ArrangerPanel.h"

ArrangerPanel::ArrangerPanel()
	: align(false)
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

void ArrangerPanel::set_align(bool yes)
{
	if(yes)
	{
		bool colliding = false;
		if(active.name.length() > 0)
		{
			Texture &focus = textures.find(active.name)->second;
			for(auto &[_, current] : textures)
			{
				if(&focus == &current)
					continue;

				if(focus.collide(current))
				{
					colliding = true;
					break;
				}
			}

			if(!colliding)
				align = true;
		}
	}
	else
		align = false;

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

void ArrangerPanel::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	for(const auto &[_, texture] : textures)
	{
		painter.drawImage(QPoint(texture.x, texture.y), *texture.img.get());
	}

	// draw active texture line
	if(active.name.length() > 0)
		painter.drawText(QPoint(0, painter.fontMetrics().height()), press::swrite("Active Texture: {} [{}x{}]{}", active.name, textures[active.name].w, textures[active.name].h, align ? " ALIGNED" : "").c_str());
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
