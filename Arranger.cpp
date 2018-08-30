#include <fstream>

#include <QFileDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>

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
	list = new QListWidget;
	auto newtexture = new QPushButton("Add Texture");
	auto deletetexture = new QPushButton("Remove Texture");
	auto exportatlas = new QPushButton("Export");
	m_panel = new ArrangerPanel();

	// sidebar widget settings
	const int sidebar_width = 200;
	list->setMaximumWidth(sidebar_width);
	newtexture->setMaximumWidth(sidebar_width);

	// hook up dem buttuns
	QObject::connect(newtexture, &QPushButton::clicked, this, &Arranger::slot_add_texture);
	QObject::connect(deletetexture, &QPushButton::clicked, this, &Arranger::slot_remove_texture);
	QObject::connect(exportatlas, &QPushButton::clicked, this, &Arranger::slot_export);

	// stir it all together
	vbox->addWidget(list);
	vbox->addWidget(newtexture);
	vbox->addWidget(deletetexture);
	vbox->addWidget(exportatlas);
	hbox->addLayout(vbox);
	hbox->addWidget(m_panel);

	// add some dummy textures
	m_panel->add("/home/josh/fishtank/assets_local/mine.tga");
	m_panel->add("/home/josh/fishtank/assets_local/turret.tga");
	m_panel->add("/home/josh/fishtank/assets_local/tank.tga");
	m_panel->add("/home/josh/fishtank/assets_local/dead_fish.tga");
	m_panel->add("/home/josh/fishtank/assets_local/button.tga");
	list->addItem("/home/josh/fishtank/assets_local/mine.tga");
	list->addItem("/home/josh/fishtank/assets_local/turret.tga");
	list->addItem("/home/josh/fishtank/assets_local/tank.tga");
	list->addItem("/home/josh/fishtank/assets_local/dead_fish.tga");
	list->addItem("/home/josh/fishtank/assets_local/button.tga");
}

void Arranger::keyPressEvent(QKeyEvent *key)
{
	if(key->key() == Qt::Key_Control)
		m_panel->set_align(false);
	else if(key->key() == Qt::Key_Left)
		m_panel->pack(true);
	else if(key->key() == Qt::Key_Up)
		m_panel->pack(false);
	else if(key->key() == Qt::Key_Alt)
		m_panel->set_border(!m_panel->get_border());
}

void Arranger::keyReleaseEvent(QKeyEvent *key)
{
	if(key->key() == Qt::Key_Control)
		m_panel->set_align(true);
}

void Arranger::slot_add_texture()
{
	const QString import = QFileDialog::getOpenFileName(this, "Add Texture", "", "Targa TGA Images (*.tga)");
	if(import.isNull())
		return;

	try
	{
		list->addItem(import);
		m_panel->add(import.toStdString());
	}
	catch(const std::exception &e)
	{
		QMessageBox::critical(this, "Error", press::swrite("could not load file {}: {}", import.toStdString(), e.what()).c_str());
	}
}

void Arranger::slot_remove_texture()
{
	QListWidgetItem *current = list->currentItem();
	if(current == NULL)
		return;

	QString name = current->text();
	try
	{
		m_panel->remove(name.toStdString());
	}
	catch(const std::exception &e)
	{
		QMessageBox::warning(this, "Couldn't remove texture", e.what());
	}
	delete list->takeItem(list->currentRow());
}

void Arranger::slot_export()
{
	const QString save = QFileDialog::getSaveFileName(this, "Export to...");
	if(save.isNull())
		return;
	else if(save.toStdString().rfind(".txt") == std::string::npos)
	{
		QMessageBox::critical(this, "Error", "Unwilling to overwrite file \"" + save + "\"");
		return;
	}

	std::ofstream out(save.toStdString());
	if(!out)
	{
		QMessageBox::critical(this, "Error", "Could not open file \"" + save + "\" for writing");
		return;
	}

	const std::vector<Entry> entries = m_panel->get_entries();
	for(const Entry &entry : entries)
		out << entry.name << " " << entry.x << " " << entry.y << "\n";
}
