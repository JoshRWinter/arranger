#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Arranger.h"

Arranger::Arranger()
{
	resize(900, 700);

	auto hbox = new QHBoxLayout;
	setLayout(hbox);
	auto vbox = new QVBoxLayout;

	auto list = new QListWidget;
	auto newtexture = new QPushButton("Add Texture");
	m_panel = new ArrangerPanel();

	const int sidebar_width = 200;
	list->setMaximumWidth(sidebar_width);
	newtexture->setMaximumWidth(sidebar_width);

	vbox->addWidget(list);
	vbox->addWidget(newtexture);
	hbox->addLayout(vbox);
	hbox->addWidget(m_panel);
}
