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
#include <iostream>
#include <map>
#include <vector>
using std::map;
using std::vector;
using std::string;

#include <QPainter>
#include <QTabWidget>
#include <QGridLayout>
#include <QMouseEvent>

#include "BasicBox.hpp"
#include "CurvesWidget.hpp"
#include "CurveWidget.hpp"
#include "Maquette.hpp"

const int CurvesWidget::WIDTH = 400;
const int CurvesWidget::HEIGHT = 400;

CurvesWidget::CurvesWidget(QWidget *parent)
  : QTabWidget(parent) {
  setBackgroundRole(QPalette::Base);
  update();
  setFixedSize(WIDTH,HEIGHT);
  _parentWidget = parent;
}

CurvesWidget::~CurvesWidget() {

}

void
CurvesWidget::updateMessages(unsigned int boxID) {
/*	map<unsigned int,CurveWidget*>::iterator curveIt;
	for (curveIt = _curves.begin() ; curveIt != _curves.end() ; ++curveIt) {
		removeTab(curveIt->first);
	}*/
	clear(); 	// TODO :: delete curves

	if (boxID != NO_ID) {
		vector<string> curvesAddresses = Maquette::getInstance()->getCurvesAddresses(boxID);
		vector<string>::const_iterator curveAddressIt;

		for (curveAddressIt = curvesAddresses.begin() ; curveAddressIt != curvesAddresses.end() ; ++curveAddressIt) {
			unsigned int sampleRate;
			bool redundancy;
			vector<float> values;
			vector<string> argTypes;
			vector<float> xPercents;
			vector<float> yValues;
			vector<short> sectionType;
			vector<float> coeff;

			if (Maquette::getInstance()->getCurveAttributes(boxID,*curveAddressIt,0,sampleRate,redundancy,values,argTypes,xPercents,yValues,sectionType,coeff)) {
				CurveWidget *curve = new CurveWidget(_parentWidget,boxID,*curveAddressIt,0,values,sampleRate,redundancy,argTypes,xPercents,yValues,sectionType,coeff);
				unsigned int curveIndex = addTab(curve,QString::fromStdString(*curveAddressIt));
				_curves[curveIndex] = curve;
			}
			else {
				std::cerr << "CurvesWidget::updateMessages : getCurveAttributes returned false" << std::endl;
			}
		}
	}
}
