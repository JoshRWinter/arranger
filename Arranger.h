#ifndef ARRANGER_H
#define ARRANGER_H

#include <QWidget>

#include "press.h"
#include "ArrangerPanel.h"

class Arranger : public QWidget
{
public:
	Arranger();

private:
	void slot_add_texture();

	ArrangerPanel *m_panel;
};

#endif
