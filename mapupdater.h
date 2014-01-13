#ifndef MAPUPDATER_H
#define MAPUPDATER_H

#include <QThread>

class MapUpdater : public QThread
{
	Q_OBJECT
	
public:
	void run();
	void updateDisplayLists();
};
#endif