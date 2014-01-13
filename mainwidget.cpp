#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent) : QGLWidget(parent)
{
	setMouseTracking(true);
	showFullScreen();
	
	for(int i = 0; i < 9; ++i)
	{
		chunkList[i] = 0;
	}
	
	mousePosition.setX(width() / 2);
	mousePosition.setY(height() / 2);
	QCursor::setPos(mousePosition);

	dontMove = false;
	moveSpeed = 1.0f;
	lastMoveTime = QTime::currentTime();
	debugMode = false;
	xRot = 0.0f;
	yRot = 0.0f;
	zRot = 0.0f;
	
	xTrn = 0.0f;
	yTrn = 0.0f;
	zTrn = 0.0f;
	
	chunkController = new ChunkController(xTrn, zTrn, this);
	
	connect(chunkController, &ChunkController::ready, this, &MainWidget::updateDisplayList);
	connect(chunkController, &ChunkController::updateCoord, this, &MainWidget::updateCoord);

	chunkController -> generateMap();
	chunkController -> start();
	
	
	QTimer *paintTimer = new QTimer(this);
	connect(paintTimer, &QTimer::timeout, this, &MainWidget::updateGL);
	paintTimer -> start(16);
	
	QTimer *mapUpdateTimer = new QTimer(this);
	connect(mapUpdateTimer, &QTimer::timeout, this, &MainWidget::update);
	mapUpdateTimer -> start(16);
}

void MainWidget::keyPressEvent(QKeyEvent *event)
{
	switch(event -> key())
	{
	case Qt::Key_Escape:
		qApp -> quit();
		break;
	case Qt::Key_F5:
		chunkController -> generateMap();
		break;
	case Qt::Key_W:
		if(lastMoveTime.msecsTo(QTime::currentTime()) > 16)
		{
			lastMoveTime = QTime::currentTime();
			
			xTrn += qCos(xRot / 180.0f * M_PI) * qSin(-yRot / 180.0f * M_PI) * moveSpeed;
			yTrn += qSin(xRot / 180.0f * M_PI) * moveSpeed;
			zTrn += qCos(xRot / 180.0f * M_PI) * qCos(-yRot / 180.0f * M_PI) * moveSpeed;
		}
		break;
	case Qt::Key_S:
		if(lastMoveTime.msecsTo(QTime::currentTime()) > 16)
		{
			lastMoveTime = QTime::currentTime();
			
			xTrn -= qCos(xRot / 180.0f * M_PI) * qSin(-yRot / 180.0f * M_PI) * moveSpeed;
			yTrn -= qSin(xRot / 180.0f * M_PI) * moveSpeed;
			zTrn -= qCos(xRot / 180.0f * M_PI) * qCos(-yRot / 180.0f * M_PI) * moveSpeed;
		}
		break;
	case Qt::Key_I:
		debugMode = !debugMode;
		break;
	default:
		break;
	}
}

void MainWidget::mousePressEvent(QMouseEvent *event)
{
}

void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
	if(dontMove)
	{
		dontMove = false;
	} else {
		if((xRot + 180.0f * (GLfloat)(event -> y() - mousePosition.y()) / height() < 85.0f) &&
		(xRot + 180.0f * (GLfloat)(event -> y() - mousePosition.y()) / height() > -85.0f))
		{
			xRot += 180.0f * (GLfloat)(event -> y() - mousePosition.y()) / height();
		}
		yRot += 180.0f * (GLfloat)(event -> x() - mousePosition.x()) / width();
		if(yRot < 0.0f)
		{
			yRot += 360.0f;
		} else if(yRot >= 360.0f)
		{
			yRot -= 360.0f;
		}
		QCursor::setPos(width() / 2, height() / 2);
		dontMove = true;
	}
}

void MainWidget::initializeGL()
{
	static GLfloat fogColor[4] = {0.9f, 0.9f, 1.0f, 1.0f};

	glCullFace(GL_BACK);
	glLineWidth(5.0f);
	
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 0.35f);
	glHint(GL_FOG_HINT, GL_NICEST);
	glFogf(GL_FOG_START, 50.0f);
	glFogf(GL_FOG_END, 100.0f);
	
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	glAlphaFunc(GL_GEQUAL, 0.5F);
	
	loadTextures();
}

void MainWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, (GLint)w, (GLint)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    perspective(45.0, (GLdouble)w / (GLdouble)h, 0.1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
	glClearColor(0.9f, 0.9f, 1.0f, 1.0f);
}

void MainWidget::paintGL()
{
	glEnable(GL_TEXTURE_2D);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	
	glRotatef(xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);
	glRotatef(zRot, 0.0f, 0.0f, 1.0f);
	
	glTranslatef(xTrn, yTrn, zTrn);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	
	GLfloat cellSize = chunkController -> getCellSize();
	Chunk* chunk;
	if(!debugMode)
	{
		glTranslatef(-(MAP_SIZE - 1) * cellSize, 0.0f, -(MAP_SIZE - 1) * cellSize);
		
		for(int i = 0; i < 9; ++i)
		{
			glCallList(chunkList[i]);
			for(int j = 0; j < treeLists[i].size(); ++j)
			{
				chunk = chunkController -> getChunk(i);
				glTranslatef(chunk -> treesCoord[j].x() * cellSize, 0.0f, chunk -> treesCoord[j].y() * cellSize);
				glCallList(treeLists[i][j]);
				glTranslatef(-chunk -> treesCoord[j].x() * cellSize, 0.0f, -chunk -> treesCoord[j].y() * cellSize);
			}
			if((i + 1) % 3 == 0)
			{
				glTranslatef(-(MAP_SIZE - 1) * cellSize * 2.0f, 0.0f, (MAP_SIZE - 1) * cellSize);
			} else {
				glTranslatef((MAP_SIZE - 1) * cellSize, 0.0f, 0.0f);
			}
		}
	}
	
	glDisable(GL_TEXTURE_2D);
	
	// GLfloat deltaTextureSize = 1.0f / MAP_SIZE;
	// GLfloat textureX = 0.0f;
	// GLfloat textureY = 0.0f;
	// GLfloat mapX = -MAP_SIZE / 2 * cellSize;
	// GLfloat mapZ = -MAP_SIZE / 2 * cellSize;
	
	// if(debugMode)
	// {
		// glColor3f(0.5f, 0.5f, 0.5f);
		// glTranslatef(-(MAP_SIZE - 1) * cellSize, 0.0f, -(MAP_SIZE - 1) * cellSize);
		// for(int i = 0; i < 9; ++i)
		// {
			// textureX = 0.0f;
			// textureY = 0.0f;
			// mapX = -MAP_SIZE / 2 * cellSize;
			// mapZ = -MAP_SIZE / 2 * cellSize;
			// for(int j = 0; j < MAP_SIZE - 1; ++j)
			// {
				// glBegin(GL_LINE_STRIP);
				// for(int k = 0; k < MAP_SIZE - 1; ++k)
				// {
					// First triangle
					// glVertex3f(mapX, chunk[i].heightMap[j][k] + 0.01f, mapZ);
					// glVertex3f(mapX, chunk[i].heightMap[j][k + 1] + 0.01f, mapZ + cellSize);
					// glVertex3f(mapX + cellSize, chunk[i].heightMap[j + 1][k + 1] + 0.01f, mapZ + cellSize);
					// glVertex3f(mapX, chunk[i].heightMap[j][k] + 0.01f, mapZ);
					
					// Second triangle
					// glVertex3f(mapX + cellSize, chunk[i].heightMap[j + 1][k] + 0.01f, mapZ);
					// glVertex3f(mapX + cellSize, chunk[i].heightMap[j + 1][k + 1] + 0.01f, mapZ + cellSize);
					
					// mapZ += cellSize;
					// textureY += deltaTextureSize;
				// }
				// glEnd();
				// textureX += deltaTextureSize;
				// mapX += cellSize;
				// mapZ = -MAP_SIZE / 2 * cellSize;
				// textureY = 0.0f;
			// }
			
			// if((i + 1) % 3 == 0)
			// {
				// glTranslatef(-(MAP_SIZE - 1) * cellSize * 2.0f, 0.0f, (MAP_SIZE - 1) * cellSize);
			// } else {
				// glTranslatef((MAP_SIZE - 1) * cellSize, 0.0f, 0.0f);
			// }
		// }
	// }
}

void MainWidget::perspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    GLdouble xmin, xmax, ymin, ymax;

    ymax = zNear * tan(fovy * M_PI / 180.0);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void MainWidget::loadTextures()
{
	QImage image;
	
	image.load("grass.jpg");
	glGenTextures(1, &grassTexture);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)image.width(), (GLsizei)image.height(),
		0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	image.load("bush.png");
	glGenTextures(1, &bushTexture);
	glBindTexture(GL_TEXTURE_2D, bushTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)image.width(), (GLsizei)image.height(),
		0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	image.load("trunk.png");
	glGenTextures(1, &trunkTexture);
	glBindTexture(GL_TEXTURE_2D, trunkTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)image.width(), (GLsizei)image.height(),
		0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	image.load("trunk.png");
	glGenTextures(1, &branchesTexture);
	glBindTexture(GL_TEXTURE_2D, branchesTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)image.width(), (GLsizei)image.height(),
		0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	image.load("leaf.png");
	glGenTextures(1, &leavesTexture);
	glBindTexture(GL_TEXTURE_2D, leavesTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLsizei)image.width(), (GLsizei)image.height(),
		0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void MainWidget::updateDisplayList(int id)
{
	updateCoord();
	GLfloat deltaTextureSize;
	GLfloat textureX;
	GLfloat textureY;
	GLfloat mapX;
	GLfloat mapZ;
	GLfloat cellSize = chunkController -> getCellSize();
	GLfloat treeScaleCoefficient = chunkController -> getTreeScaleCoefficient();
	int first, second;
	Chunk *chunk;
	int totalTrees = 0;
	glDeleteLists(chunkList[id], 1);
	chunkList[id] = glGenLists(1);

	chunk = chunkController -> getChunk(id);
	glNewList(chunkList[id], GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	
	deltaTextureSize = 1.0f / MAP_SIZE;
	textureX = 0.0f;
	textureY = 0.0f;
	mapX = -MAP_SIZE / 2 * cellSize;
	mapZ = -MAP_SIZE / 2 * cellSize;
	for(int j = 0; j < MAP_SIZE - 1; ++j)
	{
		glBegin(GL_TRIANGLE_STRIP);
		
		for(int k = 0; k < MAP_SIZE; ++k)
		{
			// First triangle
			glTexCoord2f(textureX + deltaTextureSize, textureY);
			glVertex3f(mapX + cellSize, chunk -> heightMap[j + 1][k], mapZ);
			
			// Second triangle
			glTexCoord2f(textureX, textureY);
			glVertex3f(mapX, chunk -> heightMap[j][k], mapZ);
			
			mapZ += cellSize;
			textureY += deltaTextureSize;
		}
		glEnd();
		textureX += deltaTextureSize;
		mapX += cellSize;
		mapZ = -MAP_SIZE / 2 * cellSize;
		textureY = 0.0f;
	}

	glDisable(GL_CULL_FACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, bushTexture);
	glBegin(GL_TRIANGLES);
	
	GLfloat bushSize = chunkController -> getBushSize();
	mapX = -MAP_SIZE / 2 * cellSize + cellSize;
	mapZ = -MAP_SIZE / 2 * cellSize + cellSize;
	for(int i = 1; i < MAP_SIZE - 1; ++i)
	{
		glColor3ub(97, 151, 0);
		for(int j = 1; j < MAP_SIZE - 1; ++j)
		{
			if(chunk -> cellType[i][j] == Chunk::BUSH)
			{
				glTexCoord2f(0.0f, 1.0);
				glVertex3f(-bushSize + mapX, chunk -> heightMap[i][j] - 0.1, mapZ);
				
				glTexCoord2f(1.0f, 1.0);
				glVertex3f(bushSize + mapX, chunk -> heightMap[i][j] - 0.1, mapZ);
				
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-bushSize + mapX, chunk -> heightMap[i][j] - 0.1 + bushSize * 2.0, mapZ);
				
				
				glTexCoord2f(1.0f, 1.0);
				glVertex3f(bushSize + mapX, chunk -> heightMap[i][j] - 0.1, mapZ);
				
				glTexCoord2f(1.0f, 0.0);
				glVertex3f(bushSize + mapX, chunk -> heightMap[i][j] - 0.1 + bushSize * 2.0, mapZ);
				
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-bushSize + mapX, chunk -> heightMap[i][j] - 0.1 + bushSize * 2.0, mapZ);
				
				
				
				glTexCoord2f(0.0f, 1.0);
				glVertex3f(mapX, chunk -> heightMap[i][j] - 0.1, -bushSize + mapZ);
				
				glTexCoord2f(1.0f, 1.0);
				glVertex3f(mapX, chunk -> heightMap[i][j] - 0.1, bushSize + mapZ);
				
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(mapX, chunk -> heightMap[i][j] - 0.1 + bushSize * 2.0, -bushSize + mapZ);
				
				
				glTexCoord2f(1.0f, 1.0);
				glVertex3f(mapX, chunk -> heightMap[i][j] - 0.1, bushSize + mapZ);
				
				glTexCoord2f(1.0f, 0.0);
				glVertex3f(mapX, chunk -> heightMap[i][j] - 0.1 + bushSize * 2.0, bushSize + mapZ);
				
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(mapX, chunk -> heightMap[i][j] - 0.1 + bushSize * 2.0, -bushSize + mapZ);
			}
			
			mapZ += cellSize;
		}
		
		mapZ = -MAP_SIZE / 2 * cellSize + cellSize;
		mapX += cellSize;
	}
	
	glEnd();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEndList();
	
	if(treeLists[id].size() > 0)
	{
		glDeleteLists(treeLists[id][0], treeLists[id].size());
	}
	if(chunk -> trees.size() > 0)
	{
		treeLists[id].resize(chunk -> trees.size());
		treeLists[id][0] = glGenLists(chunk -> trees.size());
	}
	for(int j = 0; j < chunk -> trees.size(); ++j)
	{
		first = chunk -> treesCoord[j].x() + MAP_SIZE / 2;
		second = chunk -> treesCoord[j].y() + MAP_SIZE / 2;
		
		treeLists[id][j] = (j > 0) ? treeLists[id][j - 1] + 1 : treeLists[id][0];
		glNewList(treeLists[id][j], GL_COMPILE);
		
		glColor3ub(104, 72, 47);
		glBindTexture(GL_TEXTURE_2D, trunkTexture);
		glBegin(GL_TRIANGLES);
		for(unsigned int k = 0; k < chunk -> trees[j].trunk.size(); ++k)
		{
			glTexCoord2f(chunk -> trees[j].trunk[k].v[0].u, chunk -> trees[j].trunk[k].v[0].v);
			glVertex3f(chunk -> trees[j].trunk[k].v[0].x * treeScaleCoefficient,
				chunk -> trees[j].trunk[k].v[0].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				chunk -> trees[j].trunk[k].v[0].z * treeScaleCoefficient);
			
			glTexCoord2f(chunk -> trees[j].trunk[k].v[1].u, chunk -> trees[j].trunk[k].v[1].v);
			glVertex3f(chunk -> trees[j].trunk[k].v[1].x * treeScaleCoefficient,
				chunk -> trees[j].trunk[k].v[1].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				chunk -> trees[j].trunk[k].v[1].z * treeScaleCoefficient);
			
			glTexCoord2f(chunk -> trees[j].trunk[k].v[2].u, chunk -> trees[j].trunk[k].v[2].v);
			glVertex3f(chunk -> trees[j].trunk[k].v[2].x * treeScaleCoefficient,
				chunk -> trees[j].trunk[k].v[2].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				chunk -> trees[j].trunk[k].v[2].z * treeScaleCoefficient);
		}
		glEnd();
		
		glColor3ub(104, 72, 47);
		glBindTexture(GL_TEXTURE_2D, branchesTexture);
		glBegin(GL_TRIANGLES);
		for(unsigned int k = 0; k < chunk -> trees[j].branches.size(); ++k)
		{
			glTexCoord2f(chunk -> trees[j].branches[k].v[0].u, chunk -> trees[j].branches[k].v[0].v); 
			glVertex3f(chunk -> trees[j].branches[k].v[0].x * treeScaleCoefficient,
				chunk -> trees[j].branches[k].v[0].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				chunk -> trees[j].branches[k].v[0].z * treeScaleCoefficient);
			
			glTexCoord2f(chunk -> trees[j].branches[k].v[1].u, chunk -> trees[j].branches[k].v[1].v);
			glVertex3f(chunk -> trees[j].branches[k].v[1].x * treeScaleCoefficient,
				chunk -> trees[j].branches[k].v[1].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				chunk -> trees[j].branches[k].v[1].z * treeScaleCoefficient);
			
			glTexCoord2f(chunk -> trees[j].branches[k].v[2].u, chunk -> trees[j].branches[k].v[2].v);
			glVertex3f(chunk -> trees[j].branches[k].v[2].x * treeScaleCoefficient,
				chunk -> trees[j].branches[k].v[2].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				chunk -> trees[j].branches[k].v[2].z * treeScaleCoefficient);
		}
		glEnd();
		
		std::vector <TreeLibPlain4>& leaves = chunk -> trees[j].leaves;
		glBindTexture(GL_TEXTURE_2D, leavesTexture);
		glBegin(GL_QUADS);
		for(unsigned int id = 0; id < leaves.size(); ++id)
		{
			
			glTexCoord2f(leaves[id].v[0].u, leaves[id].v[0].v);
			glVertex3f(leaves[id].v[0].x * treeScaleCoefficient, leaves[id].v[0].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				leaves[id].v[0].z * treeScaleCoefficient);
			
			glTexCoord2f(leaves[id].v[1].u, leaves[id].v[1].v);
			glVertex3f(leaves[id].v[1].x * treeScaleCoefficient, leaves[id].v[1].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				leaves[id].v[1].z * treeScaleCoefficient);
			
			glTexCoord2f(leaves[id].v[2].u, leaves[id].v[2].v);
			glVertex3f(leaves[id].v[2].x * treeScaleCoefficient, leaves[id].v[2].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				leaves[id].v[2].z * treeScaleCoefficient);
			
			glTexCoord2f(leaves[id].v[3].u, leaves[id].v[3].v);
			glVertex3f(leaves[id].v[3].x * treeScaleCoefficient, leaves[id].v[3].y * treeScaleCoefficient + chunk -> heightMap[first][second],
				leaves[id].v[3].z * treeScaleCoefficient);
		}
		glEnd();
		
		glEndList();
	}
}

void MainWidget::update()
{
	if(chunkController -> isFinished())
	{
		chunkController -> setXTrn(xTrn);
		chunkController -> setZTrn(zTrn);
		chunkController -> start();
	}
}

void MainWidget::updateCoord()
{
	GLfloat cellSize = chunkController -> getCellSize();
	if(-xTrn > (MAP_SIZE - 1) * cellSize * 0.5f)
	{
		xTrn += (MAP_SIZE - 1) * cellSize;
		chunkController -> setXTrn(xTrn);
	}
	if(-xTrn < -(MAP_SIZE - 1) * cellSize * 0.5f)
	{
		xTrn -= (MAP_SIZE - 1) * cellSize;
		chunkController -> setXTrn(xTrn);
	}
	if(-zTrn > (MAP_SIZE - 1) * cellSize * 0.5f)
	{	
		zTrn += (MAP_SIZE - 1) * cellSize;
		chunkController -> setZTrn(zTrn);
	}
	if(-zTrn < -(MAP_SIZE - 1) * cellSize * 0.5f)
	{	
		zTrn -= (MAP_SIZE - 1) * cellSize;
		chunkController -> setZTrn(zTrn);
	}
}