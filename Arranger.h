#ifndef ARRANGER_H
#define ARRANGER_H

#include <QWidget>
#include <QListWidget>
#include <QScrollArea>

#include "press.h"
#include "ArrangerPanel.h"

class Arranger : public QWidget
{
public:
	Arranger();

protected:
	virtual void keyPressEvent(QKeyEvent*) override;
	virtual void keyReleaseEvent(QKeyEvent*) override;

private:
	void refresh_list();
	void slot_add_texture();
	void slot_remove_texture();
	void slot_compact();
	void slot_export();
	void slot_import();
	void slot_reload();
	void slot_movedown();
	void slot_moveup();

	QScrollArea *scroller;
	QListWidget *list;
	ArrangerPanel *m_panel;
};

#endif
