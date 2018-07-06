#include <QListWidget>
#include <QFileDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

#include "Arranger.h"

Arranger::Arranger()
{
	resize(900, 700);
	setWindowTitle("Atlas Arranger");

	// layouts
	auto hbox = new QHBoxLayout;
	setLayout(hbox);
	auto vbox = new QVBoxLayout;

	// some widgets
	auto list = new QListWidget;
	auto newtexture = new QPushButton("Add Texture");
	m_panel = new ArrangerPanel();

	// sidebar widget settings
	const int sidebar_width = 200;
	list->setMaximumWidth(sidebar_width);
	newtexture->setMaximumWidth(sidebar_width);

	// hook up dem buttuns
	QObject::connect(newtexture, &QPushButton::clicked, this, &Arranger::slot_add_texture);

	// stir it all together
	vbox->addWidget(list);
	vbox->addWidget(newtexture);
	hbox->addLayout(vbox);
	hbox->addWidget(m_panel);
}

void Arranger::slot_add_texture()
{
	const QString import = QFileDialog::getOpenFileName(this, "Add Texture", "", "Targa TGA Images (*.tga)");
	if(import.isNull())
		return;

	try
	{
		m_panel->add(import.toStdString());
	}
	catch(const std::exception &e)
	{
		QMessageBox::critical(this, "Error", press::swrite("could not load file {}: {}", import.toStdString(), e.what()).c_str());
	}
}
