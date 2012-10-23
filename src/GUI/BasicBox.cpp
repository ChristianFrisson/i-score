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
#include "BasicBox.hpp"
#include "MaquetteScene.hpp"
#include "Maquette.hpp"
#include "AbstractBox.hpp"
#include "AbstractComment.hpp"
#include "AbstractCurve.hpp"
#include "AbstractTriggerPoint.hpp"
#include "Comment.hpp"
#include "TriggerPoint.hpp"
#include "TextEdit.hpp"
#include "MainWindow.hpp"
#include "Relation.hpp"
#include "CurveWidget.hpp"
#include "BoxCurveEdit.hpp"

#include <algorithm>
#include <iostream>

#include <QGraphicsView>
#include <QPainter>
#include <QCursor>
#include <QMenu>
#include <QBrush>
#include <QColor>
#include <QEvent>
#include <QGraphicsSceneHoverEvent>
#include <QColorDialog>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QString>
#include <QTranslator>
#include <QGraphicsWidget>
#include <QGraphicsLayout>
#include <QGridLayout>
#include <QToolTip>
#include <QAbstractItemView>
#include <QStyleOptionViewItem>

using std::string;
using std::vector;
using std::map;

const int BasicBox::COMBOBOX_HEIGHT = 25;
const int BasicBox::COMBOBOX_WIDTH = 120;
const float BasicBox::TRIGGER_ZONE_WIDTH = 15.;
const float BasicBox::TRIGGER_ZONE_HEIGHT = 20.;

BasicBox::BasicBox(const QPointF &press, const QPointF &release, MaquetteScene *parent)
: QGraphicsItem()
{
	_scene = parent;

	int xmin = 0, xmax = 0, ymin = 0, ymax = 0;

	xmin = (int)(std::min(press.x(),release.x()));
	ymin = (int)(std::min(press.y(),release.y()));
	xmax = (int)(std::max(press.x(),release.x()));
	ymax = (int)(std::max(press.y(),release.y()));

	_abstract = new AbstractBox();

	_abstract->setTopLeft(QPointF(xmin,ymin));
	_abstract->setWidth(xmax-xmin);
	_abstract->setHeight(ymax-ymin);

    createWidget();

	init();

    update();
    connect(_comboBox,SIGNAL(currentIndexChanged(const QString&)),_curvesWidget, SLOT(displayCurve(const QString&)));
    connect(_comboBox,SIGNAL(activated(const QString&)),_curvesWidget, SLOT(displayCurve(const QString&)));    

}

void
BasicBox::centerWidget(){

    _boxWidget->move(-(width())/2+LINE_WIDTH,-(height())/2 + (1.2*RESIZE_TOLERANCE));
    _boxWidget->resize(width()-2*LINE_WIDTH,height()-1.5*RESIZE_TOLERANCE);

    _comboBox->move(0,-(height()/2+LINE_WIDTH));
    _comboBox->resize((width() - 2*LINE_WIDTH)/2,COMBOBOX_HEIGHT);
}

void
BasicBox::updateWidgets(){
    centerWidget();
}

void
BasicBox::createWidget(){

    QBrush brush;
    QPixmap pix(200,70);
    pix.fill(Qt::transparent);
    brush.setTexture(pix);
    QPalette palette;
    palette.setBrush(QPalette::Background,brush);


    //---------------------- Curve widget ----------------------//

    _boxWidget = new QWidget();
    _curvesWidget = new BoxWidget(_boxWidget);
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(_curvesWidget);
    layout->setMargin(0);
    layout->setContentsMargins(0,0,0,0);
    layout->setAlignment(_boxWidget,Qt::AlignLeft);
    _boxWidget->setLayout(layout);

    _curvesWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    _curveProxy = new QGraphicsProxyWidget(this);
    _curveProxy->setCacheMode(QGraphicsItem::ItemCoordinateCache);
    _curveProxy->setFlag(QGraphicsItem::ItemIsMovable, false);
    _curveProxy->setFlag(QGraphicsItem::ItemIsFocusable, true);
    _curveProxy->setAttribute(Qt::WA_PaintOnScreen);
    _curveProxy->setVisible(true);
    _curveProxy->setAcceptsHoverEvents(true);
    _curveProxy->setWidget(_boxWidget);
//    _curveProxy->setPalette(palette);


    //---------------- ComboBox (curve list) ------------------//
    _comboBox = new QComboBox;
    _comboBox->view()->setTextElideMode(Qt::ElideMiddle);
    _comboBox->setInsertPolicy(QComboBox::InsertAtTop);
    _comboBox->setBaseSize(_comboBox->width(),COMBOBOX_HEIGHT);
    QFont font;
    font.setPointSize(10);
    _comboBox->setFont(font);


    _comboBoxProxy = new QGraphicsProxyWidget(this);
    _comboBoxProxy->setWidget(_comboBox);
    _comboBoxProxy->setPalette(palette);
    _curvesWidget->setComboBox(_comboBox);
    _comboBox->resize((width() - 2*LINE_WIDTH)/2,RESIZE_TOLERANCE+1);

}

BasicBox::BasicBox(AbstractBox *abstract, MaquetteScene *parent)
: QGraphicsItem()
{
	_scene = parent;

	_abstract = new AbstractBox(*abstract);

	init();

	update();
}

BasicBox::~BasicBox()
{
	if (_abstract) {
		delete static_cast<AbstractBox*>(_abstract);
	}
}

void
BasicBox::init()
{
	_hasContextMenu = false;
	_shift = false;

	_playing = false;

	_comment = NULL;
    updateBoxSize();

    setCacheMode(QGraphicsItem::ItemCoordinateCache);
    setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	setFlag(ItemSendsGeometryChanges,true);
	setVisible(true);
	setAcceptsHoverEvents(true);

    setZValue(0);
}

void
BasicBox::updateCurves(){    
    _curvesWidget->updateMessages(_abstract->ID(),true);
}

int
BasicBox::type() const
{
	return BASIC_BOX_TYPE;
}

Abstract*
BasicBox::abstract() const
{
	return _abstract;
}

QPointF
BasicBox::getTopLeft() const
{
    return QPointF(_abstract->topLeft().x(),_abstract->topLeft().y());
}

QPointF
BasicBox::getBottomRight() const
{
    return QPointF(_abstract->topLeft().x() + _abstract->width(), _abstract->topLeft().y() + _abstract->height());
}

QPointF
BasicBox::getCenter() const
{
	return QPointF(_abstract->topLeft().x() + _abstract->width()/2., _abstract->topLeft().y() + _abstract->height()/2.);
}

QPointF
BasicBox::getRelativeCenter() const
{
	return QPointF(relativeBeginPos() + _abstract->width()/2., _abstract->topLeft().y() + _abstract->height()/2.);
}

QPointF
BasicBox::getSize() const
{
	return QPointF(_abstract->width(),_abstract->height());
}

QPointF
BasicBox::getLeftGripPoint()
{
    return QPointF(_abstract->topLeft().x()-leftEar().width(), _abstract->topLeft().y() + _abstract->height()/2.);
}

QPointF
BasicBox::getRightGripPoint()
{
    return QPointF(_abstract->topLeft().x() + _abstract->width() + _rightEar.width(), _abstract->topLeft().y() + _abstract->height()/2.);
}

QPointF
BasicBox::getMiddleRight() const
{
	return QPointF(_abstract->topLeft().x() + _abstract->width(), _abstract->topLeft().y() + _abstract->height()/2.);
}

QPointF
BasicBox::getMiddleLeft() const
{
	return QPointF(_abstract->topLeft().x(), _abstract->topLeft().y() + _abstract->height()/2.);
}

QPointF
BasicBox::getShapeTopLeft() const
{ 
    return _abstract->topLeft();
}

QPointF
BasicBox::getShapeTopRight() const
{   
    return QPointF(_abstract->topLeft().x() + _abstract->width(),
            _abstract->topLeft().y());
}

void
BasicBox::setTopLeft(const QPointF & topLeft)
{
	_abstract->setTopLeft(topLeft);

	updateStuff();
}

void
BasicBox::setRelativeTopLeft(const QPointF & rTopLeft)
{
	QPointF relTopLeft(rTopLeft);
	int relBeginPos = relTopLeft.x();
	if (_abstract->mother() != NO_ID && _abstract->mother() != ROOT_BOX_ID) {
		BasicBox *motherBox = _scene->getBox(_abstract->mother());
		if (motherBox != NULL) {
			relBeginPos += motherBox->beginPos();
			relTopLeft.setX(relBeginPos);
		}
		else {
			_abstract->setMother(ROOT_BOX_ID);
		}
	}
	else {
		_abstract->setMother(ROOT_BOX_ID);
	}
	setTopLeft(relTopLeft);
}

void
BasicBox::setSize(const QPointF & size)
{
	_abstract->setWidth(std::max((float)size.x(),MaquetteScene::MS_PRECISION / MaquetteScene::MS_PER_PIXEL));
	_abstract->setHeight(size.y());

	updateStuff();
}

float
BasicBox::beginPos() const
{
	return _abstract->topLeft().x();
}

float
BasicBox::relativeBeginPos() const
{
	float relBeginPos = _abstract->topLeft().x();
	if (_abstract->mother() != NO_ID && _abstract->mother() != ROOT_BOX_ID) {
		BasicBox *motherBox = _scene->getBox(_abstract->mother());
		if (motherBox != NULL) {
			relBeginPos -= _scene->getBox(_abstract->mother())->beginPos();
		}
		else {
			_abstract->setMother(ROOT_BOX_ID);
		}
	}
	else {
		_abstract->setMother(ROOT_BOX_ID);
	}
	return relBeginPos;
}

float
BasicBox::width() const
{
	return _abstract->width();
}

float
BasicBox::height() const
{
	return _abstract->height();
}

void
BasicBox::resizeWidthEdition(float width)
{
	float newWidth = std::max(width,MaquetteScene::MS_PRECISION / MaquetteScene::MS_PER_PIXEL);

	if (hasMother()) {
		BasicBox *motherBox = _scene->getBox(_abstract->mother());
		if (motherBox != NULL) {
			if ((motherBox->getBottomRight().x() - width) <= _abstract->topLeft().x()) {
				if (_scene->resizeMode() == HORIZONTAL_RESIZE || _scene->resizeMode() == DIAGONAL_RESIZE) { // Trying to escape by a resize to the right
					newWidth = motherBox->getBottomRight().x() - _abstract->topLeft().x();
				}
			}
		}
	}
	_abstract->setWidth(newWidth);
    centerWidget();
}

void
BasicBox::resizeHeightEdition(float height)
{
	_abstract->setHeight(height);

	if (_comment != NULL) {
		_comment->updatePos();
	}
    centerWidget();
}

void
BasicBox::resizeAllEdition(float width,float height)
{
	resizeWidthEdition(width);
	resizeHeightEdition(height);
}

QString
BasicBox::name() const
{
	return QString::fromStdString(_abstract->name());
}

void
BasicBox::setName(const QString & name)
{
	_abstract->setName(name.toStdString());
}

QColor
BasicBox::color() const
{
	return QColor(_abstract->color());
}

void
BasicBox::setColor(const QColor & color)
{
	_abstract->setColor(color);
}

void
BasicBox::updateStuff()
{
    updateBoxSize();
	if (_comment != NULL) {
		_comment->updatePos();
	}

	map<BoxExtremity,map<unsigned int,Relation*> >::iterator extIt;
	map<unsigned int,Relation*>::iterator relIt;
	for (extIt = _relations.begin() ; extIt != _relations.end() ; ++extIt) {
		for (relIt = extIt->second.begin() ; relIt != extIt->second.end() ; ++relIt) {
			relIt->second->updateCoordinates();
		}
	}
	map<BoxExtremity,TriggerPoint*>::iterator it2;
	for (it2 = _triggerPoints.begin() ; it2 != _triggerPoints.end() ; it2++) {
		it2->second->updatePosition();
	}
    setFlag(QGraphicsItem::ItemIsMovable,true);
}


void
BasicBox::addRelation(BoxExtremity extremity, Relation *rel)
{
	map<BoxExtremity,Relation*>::iterator it;
	_relations[extremity][rel->ID()] = rel;
	_relations[extremity][rel->ID()]->updateCoordinates();
}

void
BasicBox::removeRelation(BoxExtremity extremity, unsigned int relID) {
	map<BoxExtremity,map<unsigned int,Relation*> >::iterator it;
	if ((it = _relations.find(extremity)) != _relations.end()) {
		map<unsigned int,Relation*>::iterator it2;
		if ((it2 = it->second.find(relID)) != it->second.end()) {
			it->second.erase(it2);
			if (it->second.empty()) {
				_relations.erase(it);
			}
		}
	}
}

void
BasicBox::removeRelations(BoxExtremity extremity) {
	map<BoxExtremity,map<unsigned int,Relation*> >::iterator it;
	if ((it = _relations.find(extremity)) != _relations.end()) {
		it->second.clear();
		_relations.erase(it);
	}
}

void
BasicBox::addComment()
{
	addComment(AbstractComment(QObject::tr("Comment").toStdString(),_abstract->ID()));
}

void
BasicBox::addComment(const AbstractComment &abstract)
{
	_comment = new Comment(abstract.text(),abstract.ID(),_scene);
	_comment->updatePos();
}

bool
BasicBox::hasComment() const
{
	return (_comment != NULL);
}

Comment*
BasicBox::comment() const
{
	return _comment;
}

void
BasicBox::removeComment()
{
	if (_comment != NULL) {
		_comment = NULL;
	}
}

bool
BasicBox::playing() const {
	return _playing;
}

void
BasicBox::setCrossedExtremity(BoxExtremity extremity)
{
	if (extremity == BOX_START) {
		_playing = true;
	}
	else if (extremity == BOX_END) {
		_playing = false;
		setCrossedTriggerPoint(false,BOX_START);
		setCrossedTriggerPoint(false,BOX_END);
	}
	_scene->setPlaying(_abstract->ID(),_playing);
}

void
BasicBox::setCrossedTriggerPoint(bool waiting, BoxExtremity extremity)
{
	map<BoxExtremity,TriggerPoint*>::iterator it;
	if ((it = _triggerPoints.find(extremity)) != _triggerPoints.end()) {
		it->second->setWaiting(waiting);
	}
	update();
}


bool
BasicBox::hasTriggerPoint(BoxExtremity extremity)
{
	return (_triggerPoints.find(extremity) != _triggerPoints.end());
}

bool
BasicBox::addTriggerPoint(BoxExtremity extremity)
{
    std::string trgName;

    switch (extremity) {
    case BOX_START :
        trgName = "/"+_abstract->name()+"/start";
		break;
	case BOX_END :
        trgName = "/"+_abstract->name()+"/end";
		break;

	default :
        trgName = "/"+_abstract->name()+MaquetteScene::DEFAULT_TRIGGER_MSG;
		break;
	}

    bool ret = false;
    if (_triggerPoints.find(extremity) == _triggerPoints.end()) {
        int trgID = _scene->addTriggerPoint(_abstract->ID(),extremity,trgName);
        if (trgID > NO_ID) {
            _triggerPoints[extremity] = _scene->getTriggerPoint(trgID);
            _triggerPoints[extremity]->updatePosition();
        }
        ret = true;
    }

    unlock();

	return ret;
}

void
BasicBox::addTriggerPoint(const AbstractTriggerPoint &abstractTP)
{
	if (_triggerPoints.find(abstractTP.boxExtremity()) == _triggerPoints.end()) {
		int trgID = abstractTP.ID();
		if (trgID == NO_ID) {
			trgID = _scene->addTriggerPoint(_abstract->ID(),abstractTP.boxExtremity(),abstractTP.message());
		}
		if (trgID != NO_ID) {
			_triggerPoints[abstractTP.boxExtremity()] = _scene->getTriggerPoint(trgID);
			_triggerPoints[abstractTP.boxExtremity()]->updatePosition();
			_scene->addItem(_triggerPoints[abstractTP.boxExtremity()]);
		}
	}
}


void
BasicBox::addTriggerPoint(BoxExtremity extremity, TriggerPoint *tp)
{
	map<BoxExtremity,TriggerPoint*>::iterator it;
	if ((it = _triggerPoints.find(extremity)) == _triggerPoints.end()) {
		_triggerPoints[extremity] = tp;
		_triggerPoints[extremity]->updatePosition();
	}
	else {
		std::cerr << "BasicBox::addTriggerPoint : already existing" <<  std::endl;
	}
}

void
BasicBox::removeTriggerPoint(BoxExtremity extremity) {
	map<BoxExtremity,TriggerPoint*>::iterator it;
	if ((it = _triggerPoints.find(extremity)) != _triggerPoints.end()) {
		_triggerPoints.erase(it);
	}
}

void
BasicBox::setTriggerPointMessage(BoxExtremity extremity, const string &message)
{
	map<BoxExtremity,TriggerPoint*>::iterator it;
	if ((it = _triggerPoints.find(extremity)) != _triggerPoints.end()) {
		it->second->setMessage(message);
	}
}

string
BasicBox::triggerPointMessage(BoxExtremity extremity)
{
	map<BoxExtremity,TriggerPoint*>::iterator it;
	if ((it = _triggerPoints.find(extremity)) != _triggerPoints.end()) {
		return it->second->message();
	}
	else {
		return "";
	}
}

void
BasicBox::setFirstMessagesToSend(const vector<string> &messages) {    
	_abstract->setFirstMsgs(messages);
}

void
BasicBox::setStartMessages(NetworkMessages *messages) {
    _abstract->setStartMessages(messages);

}

void
BasicBox::setSelectedItemsToSend(QMap<QTreeWidgetItem*,Data> itemsSelected){
    _abstract->setNetworkTreeItems(itemsSelected);
}


void
BasicBox::setExpandedItemsList(QList<QTreeWidgetItem *> itemsExpanded){
    _abstract->setNetworkTreeExpandedItems(itemsExpanded);
}

void
BasicBox::addToExpandedItemsList(QTreeWidgetItem *item){
    _abstract->addToNetworkTreeExpandedItems(item);
}

void
BasicBox::removeFromExpandedItemsList(QTreeWidgetItem *item){
    _abstract->removeFromNetworkTreeExpandedItems(item);
}

void
BasicBox::clearExpandedItemsList(){
    _abstract->clearNetworkTreeExpandedItems();
}
void
BasicBox::setLastMessagesToSend(const vector<string> &messages) {
    _abstract->setLastMsgs(messages);
}

void
BasicBox::setEndMessages(NetworkMessages *messages) {
    _abstract->setEndMessages(messages);
}

vector<string>
BasicBox::firstMessagesToSend() const {
	return _abstract->firstMsgs();
}

vector<string>
BasicBox::lastMessagesToSend() const {
	return _abstract->lastMsgs();
}

AbstractCurve *
BasicBox::getCurve(const std::string &address)
{
	AbstractCurve * curve = NULL;
	map<string,AbstractCurve*>::iterator it;
	if ((it = _abstractCurves.find(address)) != _abstractCurves.end()) {
		curve = it->second;
	}

	return curve;
}

void
BasicBox::curveActivationChanged(string address, bool activated){
    _curvesWidget->curveActivationChanged(QString::fromStdString(address),activated);
    if(activated){

        ;
    }
    else{
        removeCurve(address);
    }
}

void
BasicBox::setCurve(const string &address, AbstractCurve *curve)
{
	if (curve != NULL) {
        _abstractCurves[address] = curve;
	}
	else {
		removeCurve(address);
	}
}

void
BasicBox::addCurve(const std::string &address){

}

void
BasicBox::removeCurve(const string &address)
{
	map<string,AbstractCurve*>::iterator it = _abstractCurves.find(address);
	if (it != _abstractCurves.end()) {
		_abstractCurves.erase(it);        
	}
    _curvesWidget->removeCurve(address);
}

void
BasicBox::lock()
{
	setFlag(QGraphicsItem::ItemIsMovable, false);
//    setFlag(QGraphicsItem::ItemIsSelectable, false);
	setFlag(QGraphicsItem::ItemIsFocusable, false);
}

void
BasicBox::unlock()
{
	setFlag(QGraphicsItem::ItemIsMovable);
//    setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsFocusable);
}

bool
BasicBox::resizing() const
{
	return (cursor().shape() == Qt::SizeFDiagCursor)
	|| (cursor().shape() == Qt::SizeHorCursor)
	|| (cursor().shape() == Qt::SizeVerCursor);
}

bool
BasicBox::operator<(BasicBox *box) const
{
	return beginPos() < box->beginPos() ;
}

unsigned int
BasicBox::date() const
{
	return beginPos() * MaquetteScene::MS_PER_PIXEL;
}

unsigned int
BasicBox::duration() const
{
	return width() * MaquetteScene::MS_PER_PIXEL;
}

unsigned int
BasicBox::ID() const
{
	return _abstract->ID();
}

void
BasicBox::setID(unsigned int ID)
{
	_abstract->setID(ID);
}

unsigned int
BasicBox::mother() const
{
	return _abstract->mother();
}

void
BasicBox::setMother(unsigned int motherID)
{
	_abstract->setMother(motherID);
}

bool BasicBox::hasMother()
{ 
	return (_abstract->mother() != NO_ID && _abstract->mother() != ROOT_BOX_ID);
}

QVariant
BasicBox::itemChange(GraphicsItemChange change, const QVariant &value)
{
	QVariant newValue = QGraphicsItem::itemChange(change,value);
	//QVariant newValue(value);
	if (change == ItemPositionChange) {
		QPointF newPos = value.toPoint();
		QPointF newnewPos(newPos);
		if (hasMother()) {
			BasicBox *motherBox = _scene->getBox(_abstract->mother());
			if (motherBox != NULL) {
				if ((motherBox->getTopLeft().y() + _abstract->height()/2.) >= newPos.y()) {
					if (_scene->resizeMode() == NO_RESIZE) { // Trying to escape by a move to the top
						newnewPos.setY(motherBox->getTopLeft().y() + _abstract->height()/2.);
					}
					else if (_scene->resizeMode() == VERTICAL_RESIZE
							|| _scene->resizeMode() == DIAGONAL_RESIZE) { // Trying to escape by a resize to the top
						// Not happening
					}
				}
				if ((motherBox->getBottomRight().y() - _abstract->height()/2.) <= newPos.y()) {
					if (_scene->resizeMode() == NO_RESIZE) { // Trying to escape by a move to the bottom
						newnewPos.setY(motherBox->getBottomRight().y() - _abstract->height()/2);
					}
					else if (_scene->resizeMode() == VERTICAL_RESIZE
							|| _scene->resizeMode() == DIAGONAL_RESIZE) { // Trying to escape by a resize to the bottom
						_abstract->setHeight(motherBox->getBottomRight().y() - _abstract->topLeft().y());
						newnewPos.setY(_abstract->topLeft().y() + _abstract->height()/2);
					}
				}
				if ((motherBox->getTopLeft().x() + _abstract->width()/2.) >= newPos.x()) {
					if (_scene->resizeMode() == NO_RESIZE) { // Trying to escape by a move to the left
						newnewPos.setX(motherBox->getTopLeft().x() + _abstract->width()/2.);
					}
					else if (_scene->resizeMode() == HORIZONTAL_RESIZE
							|| _scene->resizeMode() == DIAGONAL_RESIZE) { // Trying to escape by a resize to the left
						// Not happening
					}
				}
				if ((motherBox->getBottomRight().x() - _abstract->width()/2.) <= newPos.x()) {
					if (_scene->resizeMode() == NO_RESIZE) { // Trying to escape by a move to the right
						newnewPos.setX(motherBox->getBottomRight().x() - _abstract->width()/2);
					}
					else if (_scene->resizeMode() == HORIZONTAL_RESIZE
							|| _scene->resizeMode() == DIAGONAL_RESIZE) { // Trying to escape by a resize to the right
						// Handled by BasicBox::resizeWidthEdition()
					}
				}
				newValue = QVariant(newnewPos);
			}
		}
	}

	return newValue;
}

QPainterPath
BasicBox::shape() const
{
	QPainterPath path;

//    path.addRect(boundingRect());
    path.addRect(_boxRect);
    path.addRect(_leftEar);
    path.addRect(_rightEar);
    path.addRect(_startTriggerGrip);
    path.addRect(_endTriggerGrip);

	return path;
}

void
BasicBox::updateBoxSize(){
    _boxRect = QRectF(-_abstract->width()/2, -_abstract->height()/2, _abstract->width(), _abstract->height());
}

// Bounding box of the item - useful to detect mouse interaction
QRectF
BasicBox::boundingRect() const
{
    // Origine du repere = centre de l'objet
    return QRectF(-_abstract->width()/2 - BOX_MARGIN, -_abstract->height()/2 - 3*BOX_MARGIN, _abstract->width() + 2*BOX_MARGIN, _abstract->height()+4*BOX_MARGIN);
}

QRectF
BasicBox::boxRect()
{
    return _boxRect;
}
void
BasicBox::keyPressEvent(QKeyEvent *event){
    QGraphicsItem::keyPressEvent(event);    
}

void
BasicBox::keyReleaseEvent(QKeyEvent *event){
    QGraphicsItem::keyReleaseEvent(event);
}

void
BasicBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        setSelected(true);
        if (cursor().shape() == Qt::ArrowCursor) {
            lock();
        }
        else if (cursor().shape() == Qt::CrossCursor) {
            lock();
            if (event->pos().x() < _boxRect.topLeft().x() + RESIZE_TOLERANCE) {
                _scene->setRelationFirstBox(_abstract->ID(),BOX_START);
            }
            else if (event->pos().x() > _boxRect.topRight().x() - RESIZE_TOLERANCE) {
                _scene->setRelationFirstBox(_abstract->ID(),BOX_END);
            }
        }
        else if (cursor().shape() == Qt::PointingHandCursor) {
            lock();
            if (event->pos().x() < _boxRect.topLeft().x() + RESIZE_TOLERANCE) {addTriggerPoint(BOX_START);
            }
            else if (event->pos().x() > _boxRect.topRight().x() - RESIZE_TOLERANCE) {
                addTriggerPoint(BOX_END);
            }
        }
        else if (cursor().shape() == Qt::SizeHorCursor) {
            _scene->setResizeMode(HORIZONTAL_RESIZE);
            _scene->setResizeBox(_abstract->ID());
        }
        else if (cursor().shape() == Qt::SizeVerCursor) {
            _scene->setResizeMode(VERTICAL_RESIZE);
            _scene->setResizeBox(_abstract->ID());
        }
        else if (cursor().shape() == Qt::SizeFDiagCursor) {
            _scene->setResizeMode(DIAGONAL_RESIZE);
            _scene->setResizeBox(_abstract->ID());
        }
        update();
    }

}

//void
//BasicBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    QGraphicsItem::mousePressEvent(event);
//    if (event->button() == Qt::LeftButton) {
//        if (cursor().shape() == Qt::CrossCursor) {
//            lock();
//            if (event->pos().x() < boundingRect().topLeft().x() + RESIZE_TOLERANCE) {
//                _scene->setRelationFirstBox(_abstract->ID(),BOX_START);
//            }
//            else if (event->pos().x() > boundingRect().topRight().x() - RESIZE_TOLERANCE) {
//                _scene->setRelationFirstBox(_abstract->ID(),BOX_END);
//            }
//        }
//        else if (cursor().shape() == Qt::PointingHandCursor) {
//            lock();
//            if (event->pos().x() < boundingRect().topLeft().x() + RESIZE_TOLERANCE) {
//                addTriggerPoint(BOX_START);
//            }
//            else if (event->pos().x() > boundingRect().topRight().x() - RESIZE_TOLERANCE) {
//                addTriggerPoint(BOX_END);
//            }
//        }
//        else {
//            if (cursor().shape() == Qt::SizeHorCursor) {
//                _scene->setResizeMode(HORIZONTAL_RESIZE);
//            }
//            else if (cursor().shape() == Qt::SizeVerCursor) {
//                _scene->setResizeMode(VERTICAL_RESIZE);
//            }
//            else if (cursor().shape() == Qt::SizeFDiagCursor) {
//                _scene->setResizeMode(DIAGONAL_RESIZE);
//            }
//            _scene->setResizeBox(_abstract->ID());
//        }
//        update();
//    }
//}

void
BasicBox::contextMenuEvent( QGraphicsSceneContextMenuEvent * event )
{
	QGraphicsItem::contextMenuEvent(event);
	if (_hasContextMenu) {
		setSelected(false);
		_contextMenu->exec(event->screenPos());
	}
}

QInputDialog *
BasicBox::nameInputDialog(){

    QInputDialog *nameDialog = new QInputDialog(_scene->views().first(),Qt::Popup);
    nameDialog->setInputMode(QInputDialog::TextInput);
    nameDialog->setLabelText(QObject::tr("Enter the box name :"));
    nameDialog->setTextValue(QString::fromStdString(this->_abstract->name()));
    QPoint position = _scene->views().first()->parentWidget()->pos();
    int MMwidth = _scene->views().first()->parentWidget()->width();
    nameDialog->move(position.x()+MMwidth/2,position.y());

    return nameDialog;
}

void
BasicBox::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){

    QGraphicsItem::mouseDoubleClickEvent(event);
    QRectF textRect(mapFromScene(getTopLeft()).x(),mapFromScene(getTopLeft()).y(), width(), RESIZE_TOLERANCE - LINE_WIDTH);
    qreal x1,x2,y1,y2;
    textRect.getCoords(&x1,&y1,&x2,&y2);
    bool inTextRect = event->pos().x() > x1 && event->pos().x() < x2 && event->pos().y() > y1 && event->pos().y() < y2;


    if (!_scene->playing() && (_scene->resizeMode() == NO_RESIZE && cursor().shape() == Qt::SizeAllCursor) && inTextRect) {
//        QInputDialog *nameDialog = nameInputDialog();

//        bool ok = nameDialog->exec();
//        QString nameValue = nameDialog->textValue();

//        if (ok) {
//            _abstract->setName(nameValue.toStdString());
//            this->update();
//            _scene->displayMessage(QObject::tr("Box's name successfully updated").toStdString(),INDICATION_LEVEL);
//         }

//     delete nameDialog;
//        displayCurveEditWindow();
        BoxCurveEdit *boxCurveEdit = new BoxCurveEdit(0,this);
        boxCurveEdit->exec();
    }

    else{
//        emit(doubleClickInBox());
    }
}

void
BasicBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

    QGraphicsItem::mouseMoveEvent(event);
    // Draw cursor coordinates as a tooltip
//    CurveWidget *curve = (static_cast<CurveWidget *>(_curvesWidget->_stackedLayout->currentWidget()));
//    QPointF mousePos = curve->relativeCoordinates(event->pos());
//    QRect rect;
//    QString posStr = QString("%1 ; %2").arg(mousePos.x(),0,'f',2).arg(mousePos.y(),0,'f',2);
//    QPoint pos = this->getBottomRight().toPoint();
//    QToolTip::showText(pos, posStr);

    if (_scene->resizeMode() == NO_RESIZE && cursor().shape() == Qt::SizeAllCursor) {
        _scene->selectionMoved();
    }
    else if (_scene->resizeMode() != NO_RESIZE && (cursor().shape() == Qt::SizeVerCursor || cursor().shape() == Qt::SizeHorCursor || cursor().shape() == Qt::SizeFDiagCursor)) {
        switch (_scene->resizeMode()) {
        case HORIZONTAL_RESIZE :
                resizeWidthEdition(_abstract->width() + event->pos().x() - _boxRect.topRight().x());
            break;
        case VERTICAL_RESIZE :
                resizeHeightEdition(_abstract->height() + event->pos().y() - _boxRect.bottomRight().y());
            break;
        case DIAGONAL_RESIZE :
                resizeAllEdition(_abstract->width() + event->pos().x() - _boxRect.topRight().x(),
                        _abstract->height() + event->pos().y() - _boxRect.bottomRight().y());
            break;
        }
        QPainterPath nullPath;
        nullPath.addRect(QRectF(QPointF(0.,0.),QSizeF(0.,0.)));
        _scene->setSelectionArea(nullPath);
        setSelected(true);
        _scene->boxResized();
    }
}

void
BasicBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
		_scene->setAttributes(_abstract);
	}

	if (!playing()) {
		unlock();
	}

	_scene->setResizeMode(NO_RESIZE);    
}

void
BasicBox::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
{
    QGraphicsItem::hoverEnterEvent(event);

    const float RESIZE_ZONE_WIDTH = 3*LINE_WIDTH;

    QRectF textRect(_boxRect.topLeft(),_boxRect.topRight() + QPointF(0,RESIZE_TOLERANCE));

    QRectF triggerGripLeft = _startTriggerGrip;
    QRectF triggerGripRight = _endTriggerGrip;

    QRectF relationGripLeft = _leftEar;
    QRectF relationGripRight = _rightEar;

    QRectF vertResize_bottom(_boxRect.bottomLeft()+QPointF(0,-RESIZE_ZONE_WIDTH),_boxRect.bottomRight()-QPointF(RESIZE_ZONE_WIDTH,0));
    QRectF diagResize_bottomRight(_boxRect.bottomRight()-QPointF(RESIZE_ZONE_WIDTH,RESIZE_ZONE_WIDTH),_boxRect.bottomRight());

    //bandeau zone (text rect) - top
    if(textRect.contains(event->pos())){
        setCursor(Qt::SizeAllCursor);
    }

    //Trigger zone - left
    else if(triggerGripLeft.contains(event->pos())){
        setCursor(Qt::PointingHandCursor);
    }

    //Trigger zone - right
    else if(triggerGripRight.contains(event->pos())){
        setCursor(Qt::PointingHandCursor);
    }

    //Relation grip zone - left
    else if(relationGripLeft.contains(event->pos())){
        setCursor(Qt::CrossCursor);
    }

    //Relation grip zone - right
    else if(relationGripRight.contains(event->pos())){
        setCursor(Qt::CrossCursor);
    }

    //Vertical resize zone - bottom
    else if(vertResize_bottom.contains(event->pos()))
        setCursor(Qt::SizeVerCursor);

    //Diag resize zone - Bottom right
    else if(diagResize_bottomRight.contains(event->pos()))
        setCursor(Qt::SizeFDiagCursor);

    else
        setCursor(Qt::ArrowCursor);
}


void
BasicBox::hoverMoveEvent ( QGraphicsSceneHoverEvent * event )
{
    //Bug qui survient parfois :
    //  Le curseur a mémorisé la forme qu'il avait avant de rentrer dans la zone BoxWidget.
    //  Même si il est pourtant visible avec la forme ArrowCursor, il exécute au click l'action associée au curseur précédent.
    //
    //Solution provisoire :
    //  Pour palier à cela il y a une zone "vide" (qui met le curseur à ArrowCursor) avant d'entrer dans le widget.
    //  Seulement si on passe trop vite dessus, il n'a pas le temps de l'affecter.

    QGraphicsItem::hoverEnterEvent(event);

    const float RESIZE_ZONE_WIDTH = 3*LINE_WIDTH;

    QRectF textRect(_boxRect.topLeft(),_boxRect.topRight() + QPointF(0,3*RESIZE_TOLERANCE/4));

    QRectF triggerGripLeft = _startTriggerGrip;
    QRectF triggerGripRight = _endTriggerGrip;

    QRectF relationGripLeft = _leftEar;
    QRectF relationGripRight = _rightEar;

    QRectF vertResize_bottom(_boxRect.bottomLeft()+QPointF(0,-RESIZE_ZONE_WIDTH),_boxRect.bottomRight()-QPointF(RESIZE_ZONE_WIDTH,0));
    QRectF diagResize_bottomRight(_boxRect.bottomRight()-QPointF(RESIZE_ZONE_WIDTH,RESIZE_ZONE_WIDTH),_boxRect.bottomRight());

    //bandeau zone (text rect) - top
    if(textRect.contains(event->pos())){
        setCursor(Qt::SizeAllCursor);
    }

    //Trigger zone - left
    else if(triggerGripLeft.contains(event->pos())){
        setCursor(Qt::PointingHandCursor);
    }

    //Trigger zone - right
    else if(triggerGripRight.contains(event->pos())){
        setCursor(Qt::PointingHandCursor);
    }

    //Relation grip zone - left
    else if(relationGripLeft.contains(event->pos())){
        setCursor(Qt::CrossCursor);
    }

    //Relation grip zone - right
    else if(relationGripRight.contains(event->pos())){
        setCursor(Qt::CrossCursor);
    }

    //Vertical resize zone - bottom
    else if(vertResize_bottom.contains(event->pos()))
        setCursor(Qt::SizeVerCursor);

    //Diag resize zone - Bottom right
    else if(diagResize_bottomRight.contains(event->pos()))
        setCursor(Qt::SizeFDiagCursor);

    else{
        setCursor(Qt::ArrowCursor);
    }
}

void
BasicBox::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
    QGraphicsItem::hoverLeaveEvent(event);
    setCursor(Qt::ArrowCursor);
}

void
BasicBox::drawTriggerGrips(QPainter *painter){

    int earWidth, earHeight;
    float adjust = LINE_WIDTH/2;
    earWidth = TRIGGER_ZONE_WIDTH*2;
    earHeight = TRIGGER_ZONE_HEIGHT;

    int startAngle, spanAngle;
    int newX = -width()/2 - adjust - earWidth/2;
    int newY = -height()/2 - earHeight/2;
    QSize shapeSize(earWidth/2,earHeight/2);

    QPen pen(isSelected() ? Qt::yellow : Qt::white);
    QBrush brush(Qt::SolidPattern);
    brush.setColor(isSelected() ? Qt::yellow : Qt::white);
    painter->setBrush(brush);
    painter->setPen(pen);

    QRectF rect(0, 0, earWidth, earHeight);

    //The left one
    startAngle = 0;
    spanAngle = 90 * 16;
    rect.moveTo(newX,newY);
    _startTriggerGrip = QRectF(QPointF(newX+earWidth/2,newY),shapeSize);
    painter->drawPie(rect,startAngle,spanAngle);


    //The right one
    startAngle = 90 * 16;
    newX = width()/2 - earWidth/2 + adjust;
    rect.moveTo(newX,newY);
    _endTriggerGrip = QRectF(QPointF(newX,newY),shapeSize);
    painter->drawPie(rect,startAngle,spanAngle);
}

void
BasicBox::drawInteractionGrips(QPainter *painter){

    int earWidth, earHeight;
    earWidth = 18;
    earHeight = 30 ;

    QPen pen(isSelected() ? Qt::yellow : Qt::white);
    QBrush brush(Qt::SolidPattern);
    brush.setColor(isSelected() ? Qt::yellow : Qt::white);
    painter->setBrush(brush);
    painter->setPen(pen);

    QRectF rect(0, 0, earWidth, earHeight);

    int startAngle = 30 * 16;
    int spanAngle = 120 * 16;
    painter->rotate(90);
    rect.moveTo(QPointF(-(earWidth/2),-(earHeight/4+width()/2)));

    int newX = -(earHeight/4+width()/2);
    int newY = -(earWidth/2);
    _leftEar = QRectF(QPointF(newX,newY),QSize(earHeight/4,earWidth));

    newX = width()/2;
    _rightEar = QRectF(QPointF(newX,newY),QSize(earHeight/4,earWidth));

    painter->drawChord(rect,startAngle,spanAngle);
    painter->rotate(-180);
    painter->drawChord(rect,startAngle,spanAngle);
    painter->rotate(90);
}

void
BasicBox::drawBox(QPainter *painter){


}

void
BasicBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);  
//    QPen penR(Qt::lightGray,isSelected() ? 2 * LINE_WIDTH : LINE_WIDTH);
    QPen penR(isSelected() ? Qt::yellow : Qt::white,isSelected() ? 1.5 * LINE_WIDTH : LINE_WIDTH);

    //************* pour afficher la shape *************
    QPen penG(Qt::green);
    penG.setWidth(2);
//    painter->setPen(penG);
//    painter->drawRect(boundingRect());
//    painter->drawRect(_leftEar);
//    painter->drawRect(_rightEar);
//    painter->drawRect(_startTriggerGrip);
//    painter->drawRect(_endTriggerGrip);
    //***************************************************/

    painter->setPen(penR);

    painter->setBrush(QBrush(Qt::white,Qt::Dense7Pattern));
    painter->drawRect(_boxRect);
    drawInteractionGrips(painter);
    drawTriggerGrips(painter);

    _comboBoxProxy->setVisible(_abstract->height()>RESIZE_TOLERANCE+LINE_WIDTH);
    _curveProxy->setVisible(_abstract->height()>RESIZE_TOLERANCE+LINE_WIDTH);

    QBrush brush(Qt::lightGray,isSelected() ? Qt::SolidPattern : Qt::SolidPattern);
    QPen pen(color(),isSelected() ? 2 * LINE_WIDTH : LINE_WIDTH);
	painter->setPen(pen);
	painter->setBrush(brush);
	painter->setRenderHint(QPainter::Antialiasing, true);

	QRectF textRect = QRectF(mapFromScene(getTopLeft()).x(),mapFromScene(getTopLeft()).y(), width(), RESIZE_TOLERANCE - LINE_WIDTH);

	QFont font;
	font.setCapitalization(QFont::SmallCaps);
	painter->setFont(font);
	painter->translate(textRect.topLeft());

	if (_abstract->width() <= 3*RESIZE_TOLERANCE) {
		painter->translate(QPointF(RESIZE_TOLERANCE - LINE_WIDTH,0));
		painter->rotate(90);
		textRect.setWidth(_abstract->height());

		//textRect.setHeight(std::min(_abstract->width(),(float)(RESIZE_TOLERANCE - LINE_WIDTH)));
	}

    if (_abstract->width() <= 5*RESIZE_TOLERANCE) {
        _comboBoxProxy->setVisible(false);
        _curveProxy->setVisible(false);
    }

    painter->fillRect(0,0,textRect.width(),textRect.height(),isSelected() ? Qt::yellow : Qt::white);
    painter->drawText(QRectF(10,0,textRect.width(),textRect.height()),Qt::AlignLeft,name());

	if (_abstract->width() <= 3*RESIZE_TOLERANCE) {
		painter->rotate(-90);       
		painter->translate(-QPointF(RESIZE_TOLERANCE - LINE_WIDTH,0));        
	}


	painter->translate(QPointF(0,0)-(textRect.topLeft()));

	if (cursor().shape() == Qt::SizeHorCursor) {
		static const float S_TO_MS = 1000.;
//		painter->drawText(boundingRect().bottomRight() - QPoint(2*RESIZE_TOLERANCE,0),QString("%1s").arg((double)duration() / S_TO_MS));
        painter->drawText(_boxRect.bottomRight() - QPoint(2*RESIZE_TOLERANCE,0),QString("%1s").arg((double)duration() / S_TO_MS));
	}

//	painter->translate(boundingRect().topLeft());
    painter->translate(_boxRect.topLeft());

	if (_playing) {
        QPen pen = painter->pen();
        QBrush brush = painter->brush();
        brush.setStyle(Qt::NoBrush);
        painter->setPen(pen);
        brush.setColor(Qt::blue);
        painter->setBrush(brush);
		const float progressPosX = _scene->getProgression(_abstract->ID())*(_abstract->width());
		painter->fillRect(0,_abstract->height()-RESIZE_TOLERANCE/2.,progressPosX,RESIZE_TOLERANCE/2.,Qt::darkGreen);
		painter->drawLine(QPointF(progressPosX,RESIZE_TOLERANCE),QPointF(progressPosX,_abstract->height()));       
	}
//	painter->translate(QPointF(0,0) - boundingRect().topLeft());
    painter->translate(QPointF(0,0) - _boxRect.topLeft());

}

void
BasicBox::curveShowChanged(const QString &address,bool state){
    _curvesWidget->curveShowChanged(address,state);
}

void
BasicBox::displayCurveEditWindow(){
    QWidget *editWindow=new QWidget;
    editWindow->setWindowModality(Qt::ApplicationModal);
    QGridLayout *layout = new QGridLayout;

//    layout->addWidget(_curvesWidgetCLONE);

    editWindow->setLayout(layout);
    editWindow->setGeometry(QRect(_scene->sceneRect().toRect()));
    editWindow->show();

}

void
BasicBox::refresh(){

}
