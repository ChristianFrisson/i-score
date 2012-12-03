/*

Copyright: LaBRI / SCRIME

Authors: Nicolas Hincker (27/11/2012)

luc.vercellin@labri.fr

This software is a computer program whose purpose is to provide
notation/composition combining synthesized as well as recorded
sounds, providing answers to the problem of notation and, drawing,
from its very design, on benefits from state of the art research
in musicology and sound/music computing.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software. You can use,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty and the software's author, the holder of the
economic rights, and the successive licensors have only limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading, using, modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean that it is complicated to manipulate, and that also
therefore means that it is reserved for developers and experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and, more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#include <iostream>
#include <map>
#include <vector>
#include <QPainter>

#include "TimeLineWidget.hpp"

class MaquetteScene;

const float TimeLineWidget::TIME_BAR_HEIGHT =  15.;
const float TimeLineWidget::LEFT_MARGIN =  2.;
const float TimeLineWidget::NUMBERS_POINT_SIZE =  10.;

TimeLineWidget::TimeLineWidget(QWidget *parent, MaquetteScene *scene)
    :QWidget(parent){

    _scene = scene;
    _rect = QRect(LEFT_MARGIN,0,_scene->width(),TIME_BAR_HEIGHT);
    _zoom = 1.;
    setGeometry(_rect);
    setFixedHeight(height());
}

TimeLineWidget::~TimeLineWidget(){

}

void
TimeLineWidget::paintEngine(){

}

void
TimeLineWidget::mousePressEvent(QMouseEvent *){
    std::cout<<"Timeline :: mouse press event"<<std::endl;
}

void
TimeLineWidget::drawBackground(QPainter *painter, QRect rect){
    painter->save();

    static const int S_TO_MS = 1000;
    const int WIDTH = width();
    const int HEIGHT = TIME_BAR_HEIGHT;

    int i_PXL;
    QFont *font = new QFont();
    font->setPointSize(NUMBERS_POINT_SIZE);
    painter->setFont(*font);
    for (double i = 0 ; i <= (WIDTH * MaquetteScene::MS_PER_PIXEL) / S_TO_MS ; i++) { // for each second
        i_PXL = i * S_TO_MS / MaquetteScene::MS_PER_PIXEL + LEFT_MARGIN;
        painter->drawText(QPointF(i_PXL, 2*HEIGHT/3),QString("%1").arg(i));
        painter->drawLine(QPointF(i_PXL, 3*HEIGHT/4), QPointF(i_PXL, HEIGHT));
    }

    painter->restore();
}

void
TimeLineWidget::paintEvent(QPaintEvent *){
    QPainter *painter = new QPainter(this);
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen *pen = new QPen;
    painter->drawRect(_rect);
    drawBackground(painter,_rect);
    delete painter;
}
