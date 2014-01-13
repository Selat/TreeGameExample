#ifndef CHUNKCONTROLLER_H
#define CHUNKCONTROLLER_H

#include <QThread>

#include "chunk.h"

class ChunkController : public QThread
{
	Q_OBJECT

public:
	ChunkController(GLfloat value1, GLfloat value2, QObject* parent = 0);
	
	GLfloat getCellSize();
	GLfloat getBushSize();
	GLfloat getTreeScaleCoefficient();
	Chunk* getChunk(int id);
	
	void setXTrn(GLfloat value);
	void setZTrn(GLfloat value);
	
	void generateMap();
	
signals:
	void ready(int id);
	void updateCoord();
	
private:
	void run();
	
	Chunk chunk[9];
	
	GLfloat cellSize;
	GLfloat bushSize;
	GLfloat treeScaleCoefficient;
	GLfloat xTrn, zTrn;
};

#endif