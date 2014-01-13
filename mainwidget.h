#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtWidgets>
#include <QGLWidget>

#include "chunkcontroller.h"

class MainWidget : public QGLWidget
{
	Q_OBJECT

public:
	explicit MainWidget(QWidget *parent = 0);
	
public slots:
	void updateDisplayList(int id);
	void update();
	void updateCoord();
	
protected:
	void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	
private:
	void perspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
	void loadTextures();
	
	QVector <GLuint> treeLists[9];
	GLuint chunkList[9];
	ChunkController* chunkController;
	
	QPoint mousePosition;
	GLfloat xRot, yRot, zRot;
	GLfloat xTrn, yTrn, zTrn;
	
	QTime lastMoveTime;
	
	GLfloat moveSpeed;
	
	GLuint grassTexture;
	GLuint bushTexture;
	GLuint trunkTexture;
	GLuint branchesTexture;
	GLuint leavesTexture;
	
	bool dontMove;
	bool debugMode;
};

#endif