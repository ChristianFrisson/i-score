/*
Copyright: LaBRI / SCRIME

Authors: Luc Vercellin and Bruno Valeze (08/03/2010)

luc.vercellin@labri.fr

This software is a computer program whose purpose is to provide
notation/composition combining synthesized as well as recorded
sounds, providing answers to the problem of notation and, drawing,
from its very design, on benefits from state of the art research
in musicology and sound/music computing.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#include "MaquetteView.hpp"
#include <iostream>
#include "BasicBox.hpp"
#include "MainWindow.hpp"
#include "MaquetteScene.hpp"
#include "AttributesEditor.hpp"

#include "Maquette.hpp"

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QBuffer>
#include <QKeyEvent>

using namespace SndBoxProp;

MaquetteView::MaquetteView(MainWindow *mw)
  : QGraphicsView(mw)
{
  setRenderHint(QPainter::Antialiasing);
  setBackgroundBrush(QColor(140,176,140));
  setCacheMode(QGraphicsView::CacheBackground);
  setWindowTitle(tr("Maquette"));
  setAlignment(Qt::AlignLeft | Qt::AlignTop);
  centerOn(0,0);
  _zoom = 1;
  _gotoValue = 0;
}

MaquetteView::~MaquetteView()
{
}

void
MaquetteView::setGotoValue(int value) {
	_gotoValue = value;
	updateScene();
}

void
MaquetteView::updateScene()
{
  _scene = static_cast<MaquetteScene*>(scene());
  centerOn(0,0);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setSceneRect(0,0,MaquetteScene::MAX_SCENE_WIDTH,MaquetteScene::MAX_SCENE_HEIGHT);
}

void
MaquetteView::drawBackground(QPainter * painter, const QRectF & rect)
{
  QGraphicsView::drawBackground(painter,rect);

  QPen pen(Qt::darkGreen);

  painter->setPen(pen);

  QPointF upperPoint,downPoint;
  static const int S_TO_MS = 1000;
  const int WIDTH = sceneRect().width();
  const int HEIGHT = sceneRect().height();
  for (int i = 0 ; i <= (WIDTH * MaquetteScene::MS_PER_PIXEL) / S_TO_MS ; i++) { // for each second
  	int i_PXL = i * S_TO_MS / MaquetteScene::MS_PER_PIXEL;
  	if (_zoom < 1 && ((i % (int)(1./_zoom)) != 0 )) {
  		continue;
  	}
 	 painter->drawText(QPointF(i_PXL, 10),QString("%1").arg(i));
 	 painter->drawLine(QPointF(i_PXL, 0), QPointF(i_PXL, HEIGHT));

   if (_zoom > 1) {
	 	 QPen pen = painter->pen();
	 	 QPen savePen = pen;
	 	 pen.setColor(Qt::darkGray);
	 	 painter->setPen(pen);
  	 for (float j = i ; j < i+1 ; j+=1./_zoom) {
    		if (i != j) {
    			float j_PXL =  j * S_TO_MS / (float)MaquetteScene::MS_PER_PIXEL;
    			if (_zoom > 4 || QString("%1").arg(j-(int)j).length() < 5) {
    				painter->drawText(QPointF(j_PXL-10, 15),QString("%1").arg(round(j*1000)/1000.));
    			}
    			painter->drawLine(QPointF(j_PXL, 15), QPointF(j_PXL, HEIGHT));
    		}
    	}
			painter->setPen(savePen);
    }
  }

  double progressBarPosX = _gotoValue/(float)MaquetteScene::MS_PER_PIXEL;

  QPen reSavedPen = painter->pen();
  QPen pen3(Qt::black);
  pen3.setWidth(5);
  painter->setPen(pen3);
  painter->drawLine(QPointF(progressBarPosX,0),QPointF(progressBarPosX,HEIGHT));
  pen3.setColor(Qt::white);
  pen3.setWidth(1);
  painter->setPen(pen3);
  painter->drawLine(QPointF(progressBarPosX,0),QPointF(progressBarPosX,HEIGHT));
  painter->setPen(reSavedPen);

  if (_scene->tracksView()) {
    QPen pen2(Qt::darkGray);
    pen2.setStyle(Qt::SolidLine);
    pen2.setWidth(4);
    painter->setPen(pen2);

    for (int i = 0 ; i <= MaquetteScene::MAX_SCENE_HEIGHT ; i=i+150) {
    	painter->drawLine(QPointF(0,i), QPointF(MaquetteScene::MAX_SCENE_WIDTH, i));
    }
  }
}

void
MaquetteView::keyPressEvent(QKeyEvent *event)
{
	QGraphicsView::keyPressEvent(event);
	if (event->matches(QKeySequence::Copy)) {
		_scene->copyBoxes();
		_scene->displayMessage(tr("Selection copied").toStdString(),INDICATION_LEVEL);
	}
	else if (event->matches(QKeySequence::Cut)) {
		_scene->cutBoxes();
		_scene->displayMessage(tr("Selection cut").toStdString(),INDICATION_LEVEL);
	}
	else if (event->matches(QKeySequence::Paste)) {
		_scene->pasteBoxes();
		_scene->displayMessage(tr("Copied selection pasted").toStdString(),INDICATION_LEVEL);
	}
	else if (event->matches(QKeySequence::SelectAll)) {
		_scene->selectAll();
		_scene->displayMessage(tr("All selected").toStdString(),INDICATION_LEVEL);
	}
    else if (event->matches(QKeySequence::Delete) || event->key()==Qt::Key_Backspace) {
		_scene->removeSelectedItems();
		_scene->displayMessage(tr("Selection removed").toStdString(),INDICATION_LEVEL);
    }
}

/**
 * Perform zoom in
 */
void
MaquetteView::zoomIn()
{
	if (MaquetteScene::MS_PER_PIXEL > 0.125) {
		MaquetteScene::MS_PER_PIXEL /= 2;
		_zoom *= 2;
		resetCachedContent();
		_scene->update();
		Maquette::getInstance()->updateBoxesFromEngines();
	}
}

/**
 * Perform zoom out
 */
void
MaquetteView::zoomOut()
{
	MaquetteScene::MS_PER_PIXEL *= 2;
	_zoom /= 2.;
	resetCachedContent();
	_scene->update();
	Maquette::getInstance()->updateBoxesFromEngines();
}
