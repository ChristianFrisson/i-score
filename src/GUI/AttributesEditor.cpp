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
#include <algorithm>
#include "SoundBox.hpp"
#include "Palette.hpp"
#include "AttributesEditor.hpp"
#include "PreviewArea.hpp"
#include "BasicBox.hpp"
#include "Maquette.hpp"
#include "MaquetteScene.hpp"
#include "Maquette.hpp"

#include <map>
#include <string>
#include <vector>
using std::string;
using std::vector;
using std::map;

#include <QString>
#include <QFont>
#include <QComboBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QColorDialog>
#include <QScrollArea>
#include <QApplication>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSizePolicy>
#include "NetworkMessagesEditor.hpp"
#include "CurvesWidget.hpp"

/* La palette h�rite de QDockWidget, a pour parent la fenetre principale*/

static const int ENGLISH = 0;
static const int FRENCH = 1;

using namespace SndBoxProp;

#define TOP_MARGIN 5
#define BOTTOM_MARGIN 5
#define LEFT_MARGIN 5
#define RIGHT_MARGIN 5

static const float S_TO_MS  = 1000.;

enum {NodeNamespaceType = QTreeWidgetItem::UserType + 1, NodeNoNamespaceType = QTreeWidgetItem::UserType + 2 ,
	LeaveType = QTreeWidgetItem::UserType + 3, AttributeType = QTreeWidgetItem::UserType + 4};

AttributesEditor::AttributesEditor(QWidget* parent) : QDockWidget(tr("AttributesEditor"),parent,0)
{
	Palette* sharedPalette = new Palette;
	sharedPalette->setContainer(NULL);

	_profilesPreviewArea = new PreviewArea(parent,sharedPalette); //Creation de la zone d'apercu
	_generalPreviewArea = new PreviewArea(parent,sharedPalette); //Creation de la zone d'apercu
	_curvesWidget = new CurvesWidget(parent);
	_palette = sharedPalette;
	_boxEdited = NO_ID;

	_palette->setColor(Qt::black);

	setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
}

void
AttributesEditor::init() {
	createWidgets();

	addWidgetsToLayout();

	nameWidgets();

	connectSlots();

	setAcceptDrops(false);

	setMinimumWidth(400);

	noBoxEdited();
}

AttributesEditor::~AttributesEditor()
{
/*	delete _shapeLabel;

	delete _shapeOptionRandom;
	delete _shapeComboBox;

	delete _rythmLabel;
	delete _speedHeldLabel;
	delete _speedOptionRandom;
	delete _speedVariationLabel;
	delete _grainLabel;
	delete _speedHeldComboBox;
	delete _speedVariationComboBox;
	delete _grainComboBox;

	delete _melodyLabel;
	delete _pitchStartLabel;
	delete _gradeLabel;
	delete _amplitudeLabel;

	delete _pitchOptionRandom;
	delete _pitchOptionVibrato;

	delete _pitchEnd;
	delete _pitchStartComboBox;
	delete _pitchEndComboBox;
	delete _pitchAmplitudeComboBox;
	delete _pitchGradeComboBox;

	delete _harmonyLabel;
	delete _harmonyHeldLabel;
	delete _harmonyVariationLabel;
	delete _harmoHeldComboBox;
	delete _harmoVariationComboBox;

	delete _colorButton;
	delete _resetButton;*/

	delete _centralWidget;
/*	delete _shapeLayout;
	delete _speedLayout;
	delete _pitchLayout;
	delete _harmonyLayout;
	delete _paletteLayout;*/

	//delete _profilesPreviewArea;
	delete _palette;
}

void AttributesEditor::setScene(MaquetteScene * scene) {
	_scene = scene;
}

unsigned int AttributesEditor::currentBox()
{
	return _boxEdited;
}

void AttributesEditor::noBoxEdited() {
	_boxEdited = NO_ID;
	setAttributes(new AbstractSoundBox);
	_boxStartValue->clear();
	_boxLengthValue->clear();
	_boxName->clear();
	_generalTab->setEnabled(false);
	_networkTabWidget->setEnabled(false);
	_snapshotTab->setEnabled(false);
	_messagesTab->setEnabled(false);
	_curvesWidget->updateMessages(NO_ID);
	_curvesTab->setEnabled(false);
}

void
AttributesEditor::nameWidgets(int language)
{
	// QComboBoxes containments names
	QStringList shapeList, speedHeldList, speedVariationList, grainList,
	pitchList, pitchVariationList, harmoHeldList, harmoVariationList;

	// QWidgets names
	QString color, reset, shapeLabel, impulsive, held, variation, irregular,
	rythm, grain, vibrato, speedHeld, speedVariation, grade, amplitude,
	pitchStart, melody, pitchEnd, harmony, profiles, messages, msgStart, msgEnd,
	apply,cancel,clear,profilesTab,networkTab,messagesTab,snapshotTab,copy,paste,
	deleteStr,general,start,length,name,assign,assignStart,assignEnd,curves;

	// Used to switch between adding and renaming items
	bool firstTimeCalled = false;
	if (_shapeComboBox->count() == 0)
		firstTimeCalled = true;

	switch(language){
	case 1 :
	{
		color = "Color";
		reset = "Reset";
		held = "Held (Default)";
		impulsive = "Impulsive  ";
		variation = "Variation";
		irregular = "Irregular   ";
		profiles = "<big><b>PROFILES</b></big>";
		shapeLabel = "Dynamics";
		rythm = "Rythm";
		grain = "Grain";
		vibrato = "Vibrato";
		speedHeld = "Speed Held";
		speedVariation = "Speed Variation";
		pitchStart = "Pitch Start";
		melody = "Melody";
		pitchEnd = "Pitch End";
		grade = "Grade :";
		amplitude = "Amplitude :";
		harmony= "Harmony";
		shapeList << "Flat" << "Insistant" << "Steep attack" << "Gentle" << "Steep truncated";
		speedHeldList << "None" << "Slow" << "Moderate" << "Fast";
		speedVariationList << "None" << "Acceleration" << "Deceleration";
		grainList << "Smooth" << "Fat" << "Tidy" << "Thin";
		pitchList << "None" << "Lowest" << "Low" << "Medium" << "High" << "Highest";
		pitchVariationList << "None" << "Fat" << "Tidy" << "Thin" ;
		harmoHeldList << "Pure" << "Key Note" << "Key Note Group" << "Ribbed" << "Node Group"
		<< "Node" << "Fringe";
		harmoVariationList << "None default" << "Getting richer" << "Getting poorer";
		messages = "<big><b>MESSAGES</b></big>";
		msgStart = "Start Message";
		msgEnd = "End Message";
		apply = "Apply";
		cancel = "Cancel";
		clear = "Clear";
		copy = "Copy";
		paste = "Paste";
		deleteStr = "Delete";
		profilesTab = "Profiles";
		networkTab = "Network";
		messagesTab = "Messages";
		snapshotTab = "Snapshot";
		curves = "Curves";
		general = "General";
		start = "Start";
		length = "Length";
		name = "Name";
		assign = "Assign";
		assignStart = "Start";
		assignEnd = "End";
	}
	break;
	case 0:
	{
		color = "Couleur";
		reset = "R�initialiser";
		held = "Tenu (Defaut)";
		impulsive = "Impulsive  ";
		variation = "Variation";
		profiles = "<big><b>PROFILS</b></big>";
		shapeLabel = "Dynamique";
		rythm = "Rythmique";
		grain = "Grain";
		vibrato = "Vibrato";
		irregular = "Irr�gulier   ";
		speedHeld = "Vitesse Tenue";
		speedVariation = "Vitesse Variante";
		pitchStart = "Hauteur D�but:";
		melody = "M�lodique";
		pitchEnd = "Hauteur Fin";
		grade = "Calibre :";
		amplitude = "Amplitude :";
		harmony= "Harmonique";
		shapeList << "Plate" << "Appui" << "Attaque raide" << "Douce" << "Raide tronqu�e";
		speedHeldList << "Aucune" << "Lente" << "Mod�r�e" << "Rapide";
		speedVariationList << "Aucune" << "Acc�l�ration" << "D�c�leration";
		grainList << "Lisse" << "Gros" << "Net" << "Fin";
		pitchList << "Aucune" << "Sur-grave" << "Grave" << "Medium" << "Aigue" << "Sur-aigue";
		pitchVariationList << "Aucune" << "Fort" << "Moyen" << "Faible" ;
		harmoHeldList << "Pur" << "Tonique" << "Groupe Tonique" << "Cannel�" << "Groupe Nodal"
		<< "Noeud" <<  "Frange";
		harmoVariationList << "Aucune" << "Enrichie" << "Appauvrie";
		messages = "<big><b>MESSAGES</b></big>";
		msgStart = "Message debut";
		msgEnd = "Message fin";
		apply = "Appliquer";
		cancel = "Annuler";
		clear = "Vider";
		copy = "Copier";
		paste = "Coller";
		deleteStr = "Supprimer";
		profilesTab = "Profils";
		networkTab = "R�seau";
		messagesTab = "Messages";
		snapshotTab = "Capture";
		curves = "Courbes";
		general = "G�n�ral";
		start = "D�but";
		length = "Dur�e";
		name = "Nom";
		assign = "Assigner";
		assignStart = "D�but";
		assignEnd = "Fin";
	}
	break;
	}

	_profilesColorButton->setText(color);
	_generalColorButton->setText(color);
	_profilesResetButton->setText(reset);
	_profilesLabel->setText(profiles);
	_shapeLabel->setText(shapeLabel);
	_shapeOptionRandom->setText(irregular);
	_shapeOptionImpulsive->setText(impulsive);
	_rythmLabel->setText(rythm);
	_grainLabel->setText(grain);
	_pitchOptionRandom->setText(irregular);
	_pitchOptionVibrato->setText(vibrato);
	_speedOptionRandom->setText(irregular);
	_speedHeldLabel->setText(speedHeld);
	_speedVariationLabel->setText(speedVariation);
	_pitchStartLabel->setText(pitchStart);
	_melodyLabel->setText(melody);
	_pitchEnd->setText(pitchEnd);
	_gradeLabel->setText(grade);
	_amplitudeLabel->setText(amplitude);
	_harmonyHeldLabel->setText(held);
	_harmonyLabel->setText(harmony);
	_harmonyVariationLabel->setText(variation);
	_messagesLabel->setText(messages);
	_startMsgLabel->setText(msgStart);
	_endMsgLabel->setText(msgEnd);
	_startMsgClearButton->setText(clear);
	_endMsgClearButton->setText(clear);
	_startMsgCopyButton->setText(copy);
	_endMsgCopyButton->setText(copy);
	_startMsgPasteButton->setText(paste);
	_endMsgPasteButton->setText(paste);
	_startMsgDeleteButton->setText(deleteStr);
	_endMsgDeleteButton->setText(deleteStr);
	_startLabel->setText(start);
	_lengthLabel->setText(length);
	_nameLabel->setText(name);
	_assignLabel->setText(assign);
	_assignSnapshotStart->setText(assignStart);
	_assignSnapshotEnd->setText(assignEnd);

	if (_generalTabIndex != -1) {
		_tabWidget->setTabText(_generalTabIndex,general);
	}
	if (_profilesTabIndex != -1) {
		_tabWidget->setTabText(_profilesTabIndex,profilesTab);
	}
	if (_networkTabIndex != -1) {
		_tabWidget->setTabText(_networkTabIndex,networkTab);
	}
	if (_messagesTabIndex != -1 ) {
		_tabWidget->setTabText(_messagesTabIndex,messagesTab);
	}
	if (_snapshotTabIndex != -1) {
		_tabWidget->setTabText(_snapshotTabIndex,snapshotTab);
	}
	if (_curvesTabIndex != -1) {
		_tabWidget->setTabText(_curvesTabIndex,curves);
	}
	if (_shapeTabIndex != -1) {
		_profilesTabs->setTabText(_shapeTabIndex,shapeLabel);
	}
	if (_rythmTabIndex != -1) {
		_profilesTabs->setTabText(_rythmTabIndex,rythm);
	}
	if (_melodyTabIndex != -1) {
		_profilesTabs->setTabText(_melodyTabIndex,melody);
	}
	if (_harmonyTabIndex != -1) {
		_profilesTabs->setTabText(_harmonyTabIndex,harmony);
	}
	if (_startMsgsIndex != -1) {
		_messagesTabs->setTabText(_startMsgsIndex,msgStart);
	}
	if (_endMsgsIndex != -1) {
		_messagesTabs->setTabText(_endMsgsIndex,msgEnd);
	}

	if (firstTimeCalled) { // First time function call : add Items
		_shapeComboBox->addItems(shapeList);
		_speedHeldComboBox->addItems(speedHeldList);
		_speedVariationComboBox->addItems(speedVariationList);
		_grainComboBox->addItems(grainList);
		_pitchStartComboBox->addItems(pitchList);
		_pitchEndComboBox->addItems(pitchList);
		_pitchAmplitudeComboBox->addItems(pitchVariationList);
		_pitchGradeComboBox->addItems(pitchVariationList);
		_harmoHeldComboBox->addItems(harmoHeldList);
		_harmoVariationComboBox->addItems(harmoVariationList);
	}
	else { // Gives Items Names
		QStringList::iterator i;
		int count;
		for (i = shapeList.begin() , count = 0; i != shapeList.end(); ++i , ++count) {
			_shapeComboBox->setItemText(count,shapeList[count]);
		}
		for (i = speedHeldList.begin() , count = 0; i != speedHeldList.end(); ++i , ++count) {
			_speedHeldComboBox->setItemText(count,speedHeldList[count]);
		}
		for (i = speedVariationList.begin() , count = 0; i != speedVariationList.end(); ++i , ++count) {
			_speedVariationComboBox->setItemText(count,speedVariationList[count]);
		}
		for (i = grainList.begin() , count = 0; i != grainList.end(); ++i , ++count) {
			_grainComboBox->setItemText(count,grainList[count]);
		}
		for (i = pitchList.begin() , count = 0; i != pitchList.end(); ++i , ++count) {
			_pitchStartComboBox->setItemText(count,pitchList[count]);
			_pitchEndComboBox->setItemText(count,pitchList[count]);
		}
		for (i = pitchVariationList.begin() , count = 0; i != pitchVariationList.end(); ++i , ++count) {
			_pitchAmplitudeComboBox->setItemText(count,pitchVariationList[count]);
			_pitchGradeComboBox->setItemText(count,pitchVariationList[count]);
		}
		for (i = harmoHeldList.begin() , count = 0; i != harmoHeldList.end(); ++i , ++count) {
			_harmoHeldComboBox->setItemText(count,harmoHeldList[count]);
		}
		for (i = harmoVariationList.begin() , count = 0; i != harmoVariationList.end(); ++i , ++count) {
			_harmoVariationComboBox->setItemText(count,harmoVariationList[count]);
		}
	}
}

void
AttributesEditor::createWidgets()
{
	// Language switch
	_languageComboBox = new QComboBox();
	_languageComboBox->addItem(tr("Francais"));
	_languageComboBox->addItem(tr("English"));

	// Allocations and names

	_tabWidget = new QTabWidget;
	_generalTab = new QWidget;
	_profilesTab = new QWidget;
	_profilesTabs = new QTabWidget;
	_networkTabWidget = new QTabWidget;
	_messagesTab = new QWidget;
	_messagesTabs = new QTabWidget;
	_snapshotTab = new QWidget;
	_curvesTab = new QWidget;
	_profilesColorButton = new QPushButton;
	_generalColorButton = new QPushButton;
	_profilesResetButton = new QPushButton;

  _boxStartValue = new QDoubleSpinBox;
  _boxLengthValue = new QDoubleSpinBox;
  _boxName = new QLineEdit;

	_startLabel = new QLabel;
	_endLabel = new QLabel;
	_lengthLabel = new QLabel;
  _nameLabel = new QLabel;
	_profilesLabel = new QLabel;
	_shapeLabel = new QLabel;
	_shapeOptionRandom = new QCheckBox;
	_shapeOptionImpulsive = new QCheckBox;
	_rythmLabel = new QLabel;
	_grainLabel = new QLabel;
	_pitchOptionRandom = new QCheckBox;
	_pitchOptionVibrato = new QCheckBox;
	_speedOptionRandom = new QCheckBox;
	_speedHeldLabel = new QLabel;
	_speedVariationLabel = new QLabel;
	_pitchStartLabel = new QLabel;
	_melodyLabel = new QLabel;
	_amplitudeLabel = new QLabel;
	_gradeLabel = new QLabel;
	_pitchEnd = new QCheckBox;
	_harmonyHeldLabel = new QLabel;
	_harmonyLabel = new QLabel;
	_harmonyVariationLabel = new QLabel;
	_shapeComboBox = new QComboBox;
	_speedHeldComboBox = new QComboBox;
	_speedVariationComboBox = new QComboBox;
	_grainComboBox = new QComboBox;
	_pitchStartComboBox = new QComboBox;
	_pitchEndComboBox = new QComboBox;
	_pitchAmplitudeComboBox = new QComboBox;
	_pitchGradeComboBox = new QComboBox;
	_harmoHeldComboBox = new QComboBox;
	_harmoVariationComboBox = new QComboBox;

  _messagesLabel = new QLabel;
  _startMsgLabel = new QLabel;
  _endMsgLabel = new QLabel;
	_startMsgScrollArea = new QScrollArea;
	_endMsgScrollArea = new QScrollArea;
  _startMsgsEditor = new NetworkMessagesEditor(_startMsgScrollArea);
  _endMsgsEditor = new NetworkMessagesEditor(_endMsgScrollArea);
	_startMsgsAddButton = new QPushButton("+",this);
	_endMsgsAddButton = new QPushButton("+",this);
  _startMsgClearButton = new QPushButton("Clear", this);
  _endMsgClearButton = new QPushButton("Clear", this);
  _startMsgCopyButton = new QPushButton("Copy", this);
  _endMsgCopyButton = new QPushButton("Copy", this);
  _startMsgPasteButton = new QPushButton("Paste", this);
  _endMsgPasteButton = new QPushButton("Paste", this);
  _startMsgDeleteButton = new QPushButton("Delete", this);
  _endMsgDeleteButton = new QPushButton("Delete", this);

  _profilesTabIndex = -1;
  _generalTabIndex = -1;
  _networkTabIndex = -1;
  _messagesTabIndex = -1;
  _snapshotTabIndex = -1;
  _curvesTabIndex = -1;
  _shapeTabIndex = -1;
  _rythmTabIndex = -1;
  _melodyTabIndex = -1;
  _harmonyTabIndex = -1;
  _startMsgsIndex = -1;
  _endMsgsIndex = -1;

	_paletteLayout = new QGridLayout;
	_profilesTopLayout = new QGridLayout;
	_generalTopLayout = new QGridLayout;
	_shapeLayout = new QGridLayout;
	_speedLayout = new QGridLayout;
	_pitchLayout = new QGridLayout;
	_harmonyLayout = new QGridLayout;
	_generalLayout = new QGridLayout;
	_profilesLayout = new QGridLayout;
	_messagesLayout = new QGridLayout;
	_snapshotTopLayout = new QGridLayout;
	_snapshotLayout = new QVBoxLayout;
	_curvesLayout = new QGridLayout;
	_msgStartTopLayout = new QGridLayout;
	_msgStartLayout = new QVBoxLayout;
	_msgEndTopLayout = new QGridLayout;
	_msgEndLayout = new QVBoxLayout;

	_paletteLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_profilesTopLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_generalTopLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_shapeLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_speedLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_pitchLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_harmonyLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_profilesLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_messagesLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_snapshotLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_snapshotTopLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_msgStartTopLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_msgEndTopLayout->setContentsMargins(LEFT_MARGIN , TOP_MARGIN , RIGHT_MARGIN , BOTTOM_MARGIN);
	_msgStartLayout->setContentsMargins(0 , TOP_MARGIN , 0 , BOTTOM_MARGIN);
	_msgEndLayout->setContentsMargins(0 , TOP_MARGIN , 0 , BOTTOM_MARGIN);

	_networkTree = new QTreeWidget;
	_assignSnapshotStart = new QPushButton;
	_assignSnapshotEnd = new QPushButton;
	_assignLabel = new QLabel;

	vector<string> deviceNames;
	vector<bool> deviceRequestable;
	Maquette::getInstance()->getNetworkDeviceNames(deviceNames, deviceRequestable);

	vector<string>::iterator nameIt;
	vector<bool>::iterator requestableIt;

	QList<QTreeWidgetItem*> itemsList;

	for (nameIt = deviceNames.begin(), requestableIt = deviceRequestable.begin() ; nameIt != deviceNames.end(), requestableIt != deviceRequestable.end() ;	++nameIt,++requestableIt) {
		QStringList deviceName;
		deviceName << QString::fromStdString(*nameIt);
		QTreeWidgetItem *curItem = NULL;
		if (!(*requestableIt)) {
			curItem = new QTreeWidgetItem(deviceName,NodeNoNamespaceType);
			curItem->setBackground(0,QBrush(Qt::gray));
		}
		else {
			curItem = new QTreeWidgetItem(deviceName,NodeNamespaceType);
			curItem->setBackground(0,QBrush(Qt::darkCyan));
		}
		itemsList << curItem;
	}

	_networkTree->addTopLevelItems(itemsList);
	//_networkTree->setFixedSize(QSize(2*PreviewArea::WIDTH,5*PreviewArea::HEIGHT));
}

void
AttributesEditor::addWidgetsToLayout()
{
	static const unsigned int LABEL_WIDTH = 1;
	static const unsigned int LABEL_HEIGHT = 1;
	static const unsigned int PREVIEW_AREA_WIDTH = 3;
	static const unsigned int PREVIEW_AREA_HEIGHT = 3;

	_centralWidget = _tabWidget;
	_paletteLayout->setSpacing(2*BasicBox::LINE_WIDTH);

	/* AttributesEditor Preview Area associated Buttons */
	_profilesTopLayout->addWidget(_profilesPreviewArea , 0 , 0 , PREVIEW_AREA_HEIGHT , PREVIEW_AREA_WIDTH , Qt::AlignHCenter);
	_profilesTopLayout->addWidget(_profilesColorButton , 0 , PREVIEW_AREA_WIDTH , Qt::AlignCenter);
	_profilesTopLayout->addWidget(_profilesResetButton , 1 , PREVIEW_AREA_WIDTH , Qt::AlignCenter);

	static const unsigned int BOX_EXTREMITY_PRECISION = 3;
	_boxStartValue->setRange(0.,MaquetteScene::MAX_SCENE_WIDTH * MaquetteScene::MS_PER_PIXEL / S_TO_MS);
	_boxStartValue->setDecimals(BOX_EXTREMITY_PRECISION);
	_boxStartValue->setKeyboardTracking(false);
	_boxLengthValue->setRange(0.,MaquetteScene::MAX_SCENE_WIDTH * MaquetteScene::MS_PER_PIXEL / S_TO_MS);
	_boxLengthValue->setDecimals(BOX_EXTREMITY_PRECISION);
	_boxLengthValue->setKeyboardTracking(false);

	_generalTopLayout->addWidget(_generalPreviewArea , 0 , 0 , PREVIEW_AREA_HEIGHT , PREVIEW_AREA_WIDTH , Qt::AlignHCenter);
	_generalTopLayout->addWidget(_generalColorButton , 0 , PREVIEW_AREA_WIDTH , Qt::AlignCenter);
	_generalTopLayout->addWidget(_languageComboBox , 1 , PREVIEW_AREA_WIDTH , Qt::AlignCenter);

	_generalLayout->setAlignment(Qt::AlignTop);
	_generalLayout->addLayout(_generalTopLayout, 0, 0,2,3, Qt::AlignCenter);
	_generalLayout->addWidget(_startLabel,2,0,Qt::AlignLeft);
	_generalLayout->addWidget(_boxStartValue,2,1,Qt::AlignLeft);
	_generalLayout->addWidget(new QLabel("s"),2,2,Qt::AlignLeft);
	_generalLayout->addWidget(_lengthLabel,3,0,Qt::AlignLeft);
	_generalLayout->addWidget(_boxLengthValue,3,1,Qt::AlignLeft);
	_generalLayout->addWidget(new QLabel("s"),3,2,Qt::AlignLeft);
	_generalLayout->addWidget(_nameLabel,4,0,Qt::AlignLeft);
	_generalLayout->addWidget(_boxName,4,1,Qt::AlignLeft);

	_generalTab->setLayout(_generalLayout);

	/* VOLUME */
	// Options and internal layout
	_shapeLayout->setAlignment(Qt::AlignTop);
	_shapeLayout->addWidget(_shapeComboBox , 0 , 0 , Qt::AlignCenter);
	_shapeLayout->addWidget(_shapeOptionRandom , 0 , 1 , Qt::AlignCenter);
	_shapeLayout->addWidget(_shapeOptionImpulsive , 0 , 2 , 1 , 2 , Qt::AlignCenter);
	_shapeOptionImpulsive->setEnabled(false);
	QWidget *shapeWidget = new QWidget(this);
	shapeWidget->setLayout(_shapeLayout);

	/* RYTHM */
	// Held
	_rythmLabel->setBuddy(_speedHeldComboBox);
	_speedHeldLabel->setBuddy(_speedHeldComboBox);
	// Variation
	_speedVariationLabel->setBuddy(_speedVariationComboBox);
	// Grain
	_grainLabel->setBuddy(_grainComboBox);
	// Internal Layout
	_speedLayout->setAlignment(Qt::AlignTop);
	_speedLayout->addWidget(_speedHeldLabel, 0 , 0 ,  Qt::AlignLeft);
	_speedLayout->addWidget(_speedHeldComboBox, 0 , 1 , Qt::AlignRight);
	_speedLayout->addWidget(_speedOptionRandom, 0 , 2 , Qt::AlignRight);
	_speedLayout->addWidget(_speedVariationLabel, 1 , 0 , Qt::AlignLeft);
	_speedLayout->addWidget(_speedVariationComboBox, 1 , 1 , Qt::AlignRight);
	_speedLayout->addWidget(_grainLabel, 2 , 0 , Qt::AlignLeft);
	_speedLayout->addWidget(_grainComboBox, 2 , 1 , Qt::AlignRight);
	QWidget *speedWidget = new QWidget(this);
	speedWidget->setLayout(_speedLayout);
	/* MELODY */
	// Held or variation of pitch
	// Pitch Start
	_pitchLayout->setAlignment(Qt::AlignTop);
	_pitchStartLabel->setBuddy(_pitchStartComboBox);
	_melodyLabel->setBuddy(_pitchStartComboBox);
	// Pitch Variation
	_pitchEnd->setCheckState(Qt::Unchecked);
	_pitchLayout->addWidget(_pitchStartLabel , 0 , 0);
	_pitchLayout->addWidget(_pitchStartComboBox , 0 , 1 );
	_pitchLayout->addWidget(_pitchOptionRandom , 0 , 2 );
	_pitchLayout->addWidget(_pitchOptionVibrato , 1 , 2);
	_pitchLayout->addWidget(_pitchEnd, 1 , 0 );
	_pitchLayout->addWidget(_pitchEndComboBox , 1 , 1 );
	_pitchLayout->addWidget(_amplitudeLabel , 2 , 0);
	_pitchLayout->addWidget(_pitchAmplitudeComboBox , 2 , 1);
	_pitchLayout->addWidget(_gradeLabel , 2 , 2);
	_pitchLayout->addWidget(_pitchGradeComboBox , 2 , 3);
	QWidget *pitchWidget = new QWidget(this);
	pitchWidget->setLayout(_pitchLayout);
	/* HARMONY*/
	// Held
	_harmonyLayout->setAlignment(Qt::AlignTop);
	_harmonyHeldLabel->setBuddy(_harmoHeldComboBox);
	_harmonyLabel->setBuddy(_harmoHeldComboBox);
	// Variation
	_harmonyVariationLabel->setBuddy(_harmoVariationComboBox);
	_harmonyLayout->addWidget(_harmonyHeldLabel, 0 , 0);
	_harmonyLayout->addWidget(_harmoHeldComboBox, 0 , 1 , Qt::AlignCenter);
	_harmonyLayout->addWidget(_harmonyVariationLabel, 1 , 0);
	_harmonyLayout->addWidget(_harmoVariationComboBox, 1 , 1, Qt::AlignCenter);
	QWidget *harmonyWidget = new QWidget(this);
	harmonyWidget->setLayout(_harmonyLayout);


	unsigned int offsetY = 0;
	unsigned int offsetX = 0;

	/* Add internal layouts and titles to the main Layout */
	// Preview Area Menu
	_profilesLayout->addLayout(_profilesTopLayout, offsetY , offsetX, 2 , 2, Qt::AlignCenter);
	offsetY += 2;
	// Shape Menu
	_shapeTabIndex = _profilesTabs->addTab(shapeWidget,_shapeLabel->text());
	_rythmTabIndex = _profilesTabs->addTab(speedWidget,_rythmLabel->text());
	_melodyTabIndex = _profilesTabs->addTab(pitchWidget,_melodyLabel->text());
	_harmonyTabIndex = _profilesTabs->addTab(harmonyWidget,_harmonyLabel->text());

	_profilesLayout->addWidget(_profilesTabs,offsetY,offsetX);
	_profilesTab->setLayout(_profilesLayout);

	offsetY = 0;
	_msgStartTopLayout->addWidget(_startMsgsAddButton, 0, 0, 1, 1, Qt::AlignLeft);
	_msgStartTopLayout->addWidget(_startMsgDeleteButton, 0, 1, 1, 1, Qt::AlignLeft);
	_msgStartTopLayout->addWidget(_startMsgCopyButton, 0, 2, 1, 1, Qt::AlignLeft);
	_msgStartTopLayout->addWidget(_startMsgPasteButton, 0, 3, 1, 1, Qt::AlignLeft);
	_msgStartTopLayout->addWidget(_startMsgClearButton, 0, 4, 1, 1, Qt::AlignRight);
	_msgStartLayout->addLayout(_msgStartTopLayout);
	offsetY += LABEL_HEIGHT;
	_msgStartLayout->addWidget(_startMsgsEditor);

	QWidget * startMsgsWidget = new QWidget(this);
	startMsgsWidget->setLayout(_msgStartLayout);

	offsetY = 0;
	_msgEndTopLayout->addWidget(_endMsgsAddButton, 0, 0, 1, 1, Qt::AlignLeft);
	_msgEndTopLayout->addWidget(_endMsgDeleteButton, 0, 1, 1, 1, Qt::AlignLeft);
	_msgEndTopLayout->addWidget(_endMsgCopyButton, 0, 2, 1, 1, Qt::AlignLeft);
	_msgEndTopLayout->addWidget(_endMsgPasteButton, 0, 3, 1, 1, Qt::AlignLeft);
	_msgEndTopLayout->addWidget(_endMsgClearButton, 0, 4, 1, 1, Qt::AlignRight);
	_msgEndLayout->addLayout(_msgEndTopLayout);
	offsetY += LABEL_HEIGHT;
	_msgEndLayout->addWidget(_endMsgsEditor);

	QWidget * endMsgsWidget = new QWidget(this);
	endMsgsWidget->setLayout(_msgEndLayout);

	_startMsgsIndex = _messagesTabs->addTab(startMsgsWidget,_startMsgLabel->text());
	_endMsgsIndex = _messagesTabs->addTab(endMsgsWidget,_endMsgLabel->text());

	_messagesLayout->addWidget(_messagesTabs);
	_messagesTab->setLayout(_messagesLayout);

	_networkTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_snapshotTopLayout->addWidget(_assignLabel,0,0,LABEL_HEIGHT,LABEL_WIDTH,Qt::AlignTop);
	_snapshotTopLayout->addWidget(_assignSnapshotStart,0,1,LABEL_HEIGHT,LABEL_WIDTH,Qt::AlignTop);
	_snapshotTopLayout->addWidget(_assignSnapshotEnd,0,2,LABEL_HEIGHT,LABEL_WIDTH,Qt::AlignTop);
	_snapshotLayout->addLayout(_snapshotTopLayout);
	_snapshotLayout->addWidget(_networkTree);

	_snapshotTab->setLayout(_snapshotLayout);


	// Options defaultly disabled
	_pitchOptionRandom->setDisabled(true);
	_pitchOptionVibrato->setDisabled(true);
	_pitchEndComboBox->setDisabled(true);
	_pitchAmplitudeComboBox->setDisabled(true);
	_pitchGradeComboBox->setDisabled(true);

	_curvesLayout->addWidget(_curvesWidget,0,0,3,3);
	_curvesTab->setLayout(_curvesLayout);

	// Set Central Widget
	setWidget(_tabWidget);
	_generalTabIndex = _tabWidget->addTab(_generalTab,"General");
	_profilesTabIndex = _tabWidget->addTab(_profilesTab,"Profiles");
	_messagesTabIndex = _tabWidget->addTab(_messagesTab,"Messages");
	_snapshotTabIndex = _tabWidget->addTab(_snapshotTab,"Snapshot");
	_curvesTabIndex = _tabWidget->addTab(_curvesTab,"Curves");
}

void
AttributesEditor::connectSlots()
{
	/* Events to handle when attributes changed */
	// General
	connect(_languageComboBox, SIGNAL(activated(int)), this, SLOT(languageChanged()));
	connect(_generalColorButton, SIGNAL(clicked()), this, SLOT(changeColor()));
	connect(_boxStartValue, SIGNAL(valueChanged(double)), this, SLOT(startChanged()));
	connect(_boxLengthValue, SIGNAL(valueChanged(double)), this, SLOT(lengthChanged()));
  connect(_boxName, SIGNAL(returnPressed()), this, SLOT(nameChanged()));

	// Profiles
	connect(_profilesColorButton, SIGNAL(clicked()), this, SLOT(changeColor()));
	connect(_profilesResetButton, SIGNAL(clicked()), this, SLOT(resetProfiles()));
	connect(_shapeComboBox, SIGNAL(activated(int)), this, SLOT(shapeChanged()));
	connect(_shapeOptionRandom,SIGNAL(stateChanged(int)),this,SLOT(shapeChanged()));
	connect(_shapeOptionImpulsive,SIGNAL(stateChanged(int)),this,SLOT(impulsiveChanged()));
	connect(_speedHeldComboBox, SIGNAL(activated(int)), this, SLOT(speedHeldChanged()));
	connect(_speedOptionRandom, SIGNAL(stateChanged(int)), this, SLOT(speedHeldChanged()));
	connect(_speedVariationComboBox, SIGNAL(activated(int)), this, SLOT(speedVariationChanged()));
	connect(_grainComboBox, SIGNAL(activated(int)), this, SLOT(grainChanged()));
	connect(_pitchOptionRandom,SIGNAL(stateChanged(int)),this,SLOT(pitchRandomChecked(int)));
	connect(_pitchOptionVibrato,SIGNAL(stateChanged(int)),this,SLOT(pitchVibratoChecked(int)));
	connect(_pitchEnd,SIGNAL(stateChanged(int)),this,SLOT(pitchEndChecked(int)));
	connect(_pitchStartComboBox, SIGNAL(activated(int)), this, SLOT(pitchStartChanged()));
	connect(_pitchEndComboBox, SIGNAL(activated(int)), this, SLOT(pitchEndChanged()));
	connect(_pitchAmplitudeComboBox, SIGNAL(activated(int)), this, SLOT(pitchAmplitudeChanged()));
	connect(_pitchGradeComboBox, SIGNAL(activated(int)), this, SLOT(pitchGradeChanged()));
	connect(_harmoHeldComboBox, SIGNAL(activated(int)), this, SLOT(harmoHeldChanged()));
	connect(_harmoVariationComboBox, SIGNAL(activated(int)), this, SLOT(harmoVariationChanged()));

  connect(_startMsgCopyButton, SIGNAL(clicked()), this, SLOT(startMsgCopied()));
  connect(_startMsgPasteButton, SIGNAL(clicked()), this, SLOT(startMsgPasted()));
  connect(_startMsgClearButton, SIGNAL(clicked()), this, SLOT(startMsgCleared()));
  connect(_endMsgCopyButton, SIGNAL(clicked()), this, SLOT(endMsgCopied()));
  connect(_endMsgPasteButton, SIGNAL(clicked()), this, SLOT(endMsgPasted()));
  connect(_endMsgClearButton, SIGNAL(clicked()), this, SLOT(endMsgCleared()));
  connect(_startMsgsAddButton, SIGNAL(clicked()), _startMsgsEditor, SLOT(addLine()));
  connect(_endMsgsAddButton, SIGNAL(clicked()), _endMsgsEditor, SLOT(addLine()));
  connect(_startMsgDeleteButton, SIGNAL(clicked()), _startMsgsEditor, SLOT(removeLines()));
  connect(_endMsgDeleteButton, SIGNAL(clicked()), _endMsgsEditor, SLOT(removeLines()));
  connect(_startMsgsEditor,SIGNAL(messagesChanged()),this,SLOT(startMsgApplied()));
  connect(_endMsgsEditor,SIGNAL(messagesChanged()),this,SLOT(endMsgApplied()));

  connect(_networkTree, SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(networkTreeCollapsed()));
  connect(_assignSnapshotStart, SIGNAL(clicked()),this,SLOT(assignSnapshotStart()));
  connect(_assignSnapshotEnd, SIGNAL(clicked()),this,SLOT(assignSnapshotEnd()));
}

void
AttributesEditor::resetProfiles()
{
	*_palette = Palette();

	_profilesColorButton->setAutoFillBackground(true);
	_profilesPreviewArea->setColor(Qt::black);
	_palette->setColor(Qt::black);

	_shapeOptionRandom->setCheckState(Qt::Unchecked);
	_shapeOptionImpulsive->setCheckState(Qt::Unchecked);
	_shapeComboBox->setCurrentIndex(0);

	_speedHeldComboBox->setCurrentIndex(0);
	_speedVariationComboBox->setCurrentIndex(0);
	_speedOptionRandom->setCheckState(Qt::Unchecked);
	_grainComboBox->setCurrentIndex(0);

	_pitchEnd->setCheckState(Qt::Unchecked);
	_pitchOptionRandom->setCheckState(Qt::Unchecked);
	_pitchOptionVibrato->setCheckState(Qt::Unchecked);
	_pitchStartComboBox->setCurrentIndex(0);
	_pitchEndComboBox->setCurrentIndex(0);
	_pitchAmplitudeComboBox->setCurrentIndex(0);
	_pitchGradeComboBox->setCurrentIndex(0);

	_harmoHeldComboBox->setCurrentIndex(0);
	_harmoVariationComboBox->setCurrentIndex(0);

	shapeChanged();
	speedHeldChanged();
	speedVariationChanged();
	grainChanged();
	pitchStartChanged();
	pitchEndChanged();
	harmoHeldChanged();
	harmoVariationChanged();

	profilesChanged();
}

void
AttributesEditor::setAttributes(AbstractBox *abBox)
{
	_boxEdited = abBox->ID();

	_startMsgsEditor->clear();
	_startMsgsEditor->addMessages(abBox->firstMsgs());
	_endMsgsEditor->clear();
	_endMsgsEditor->addMessages(abBox->lastMsgs());
	if (_boxEdited != NO_ID) {
		_curvesWidget->updateMessages(_boxEdited);
	}

	if (abBox->type() == ABSTRACT_SOUND_BOX_TYPE) {
		_profilesTab->setEnabled(true);
		*_palette = static_cast<AbstractSoundBox*>(abBox)->pal();
		_palette->setContainer(NULL);
		_profilesPreviewArea->setColor(_palette->color());
		if (_boxEdited == NO_ID) {
			_generalTab->setEnabled(false);
			_networkTabWidget->setEnabled(false);
			_snapshotTab->setEnabled(false);
			_messagesTab->setEnabled(false);
			_curvesTab->setEnabled(false);
		}
		else {
			_generalTab->setEnabled(true);
			_networkTabWidget->setEnabled(true);
			_snapshotTab->setEnabled(true);
			_messagesTab->setEnabled(true);
			_curvesTab->setEnabled(true);
		}
	}
	else if (abBox->type() == ABSTRACT_CONTROL_BOX_TYPE) {
		_profilesTab->setEnabled(false);
		*_palette = Palette();
		if (_boxEdited == NO_ID) {
			_generalTab->setEnabled(false);
			_networkTabWidget->setEnabled(false);
			_profilesTab->setEnabled(true);
			_snapshotTab->setEnabled(false);
			_messagesTab->setEnabled(false);
			_curvesTab->setEnabled(false);
		}
		else {
			_generalTab->setEnabled(true);
			_networkTabWidget->setEnabled(true);
			_snapshotTab->setEnabled(true);
			_messagesTab->setEnabled(true);
			_curvesTab->setEnabled(true);
		}
	}
	else if (abBox->type() == ABSTRACT_PARENT_BOX_TYPE) {
		_profilesTab->setEnabled(false);
		*_palette = Palette();
		if (_boxEdited == NO_ID) {
			_generalTab->setEnabled(false);
			_networkTabWidget->setEnabled(false);
			_profilesTab->setEnabled(true);
			_snapshotTab->setEnabled(false);
			_messagesTab->setEnabled(false);
			_curvesTab->setEnabled(false);
		}
		else {
			_generalTab->setEnabled(true);
			_networkTabWidget->setEnabled(true);
			_snapshotTab->setEnabled(true);
			_messagesTab->setEnabled(true);
			_curvesTab->setEnabled(true);
		}
	}

	updateWidgets();
}

void
AttributesEditor::updateWidgets()
{
	BasicBox * box = _scene->getBox(_boxEdited);
	if (box != NULL) {
		_boxStartValue->setValue(box->beginPos() * MaquetteScene::MS_PER_PIXEL / S_TO_MS);
		_boxLengthValue->setValue(box->width() * MaquetteScene::MS_PER_PIXEL / S_TO_MS);
	  _boxName->setText(box->name());
	}

	Shape shape = Shape(_palette->shape());

	static const int nbShapes = 5;
	if ( ((int)shape) >= (int)Shape(RandomFlat) ) {
		_shapeOptionRandom->setCheckState(Qt::Checked);
		_shapeComboBox->setCurrentIndex((int)shape % nbShapes);
	} else {
		_shapeOptionRandom->setCheckState(Qt::Unchecked);
		_shapeComboBox->setCurrentIndex(shape);
	}

	switch (_palette->impulsive()) {
	case true :
		_shapeOptionImpulsive->setCheckState(Qt::Checked);
		break;
	case false :
		_shapeOptionImpulsive->setCheckState(Qt::Unchecked);
		break;
	}

	static const int nbNoRandomSpeeds = 4;
	_speedHeldComboBox->setCurrentIndex( _palette->speed() % nbNoRandomSpeeds);
	if (_palette->speed() > nbNoRandomSpeeds-1)
		_speedOptionRandom->setCheckState(Qt::Checked);
	else
		_speedOptionRandom->setCheckState(Qt::Unchecked);
	_speedVariationComboBox->setCurrentIndex(_palette->speedVariation());

	Pitch pitchS = Pitch(_palette->pitchStart());
	bool pitchRandom = _palette->pitchRandom();
	bool pitchVibrato = _palette->pitchVibrato();
	Pitch pitchE = Pitch(_palette->pitchEnd());

	_pitchAmplitudeComboBox->setCurrentIndex(_palette->pitchAmplitude());
	_pitchGradeComboBox->setCurrentIndex(_palette->pitchGrade());

	if (pitchRandom) {
		_pitchOptionRandom->setCheckState(Qt::Checked);
		_pitchOptionVibrato->setCheckState(Qt::Unchecked);
	}
	else if (pitchVibrato){
		_pitchOptionRandom->setCheckState(Qt::Unchecked);
		_pitchOptionVibrato->setCheckState(Qt::Checked);
	}
	else {
		_pitchOptionRandom->setCheckState(Qt::Unchecked);
		_pitchOptionVibrato->setCheckState(Qt::Unchecked);
	}

	switch (pitchS) {
	case Lowest :
		_pitchStartComboBox->setCurrentIndex(Lowest);
		break;
	case Low :
		_pitchStartComboBox->setCurrentIndex(Low);
		break;
	case Medium :
		_pitchStartComboBox->setCurrentIndex(Medium);
		break;
	case High :
		_pitchStartComboBox->setCurrentIndex(High);
		break;
	case Highest :
		_pitchStartComboBox->setCurrentIndex(Highest);
		break;
	default :
		_pitchStartComboBox->setCurrentIndex(PitchNone);
		break;
	}

	if ( pitchE != PitchNone ){
		_pitchEndComboBox->setCurrentIndex(pitchE);
		_pitchEnd->setCheckState(Qt::Checked);
	}
	else {
		_pitchEndComboBox->setCurrentIndex(0);
		_pitchEndComboBox->setDisabled(true);
		_pitchEnd->setCheckState(Qt::Unchecked);
	}

	_grainComboBox->setCurrentIndex(_palette->grain());
	_harmoHeldComboBox->setCurrentIndex(_palette->harmo());
	_harmoVariationComboBox->setCurrentIndex(_palette->harmoVariation());

	shapeChanged();
	pitchStartChanged();

	update();
}

Palette
AttributesEditor::getPalette() const
{
	return *_palette;
}

QColor
AttributesEditor::getColor() const
{
	return _profilesPreviewArea->getColor();
}

void
AttributesEditor::languageChanged()
{
	nameWidgets(_languageComboBox->currentIndex());
	update();
}

void AttributesEditor::generalChanged() {

}

void
AttributesEditor::startChanged()
{
	BasicBox * box = _scene->getBox(_boxEdited);
	if (box != NULL) {
		box->moveBy(_boxStartValue->value() * S_TO_MS / MaquetteScene::MS_PER_PIXEL - box->getTopLeft().x(),0);
		_scene->boxMoved(_boxEdited);
	}
}
void
AttributesEditor::lengthChanged() {
	BasicBox * box = _scene->getBox(_boxEdited);
	if (box != NULL) {
		box->resizeWidthEdition(_boxLengthValue->value() * S_TO_MS / MaquetteScene::MS_PER_PIXEL);
		QPainterPath nullPath;
		nullPath.addRect(QRectF(QPointF(0.,0.),QSizeF(0.,0.)));
		_scene->setSelectionArea(nullPath);
		box->setSelected(true);
		_scene->setResizeBox(_boxEdited);
		_scene->boxResized();
	}
}

void
AttributesEditor::nameChanged()
{
	BasicBox * box = _scene->getBox(_boxEdited);
	if (box != NULL) {
		box->setName(_boxName->text());
		_scene->update(box->getTopLeft().x(),box->getTopLeft().y(),box->width(),box->height() + 10);
	}
}

void AttributesEditor::profilesChanged() {
	_profilesPreviewArea->applyChanges();
	_generalPreviewArea->applyChanges();
	BasicBox * box = _scene->getBox(_boxEdited);
	if (box != NULL) {
		if (box->type() == SOUND_BOX_TYPE) {
			static_cast<SoundBox*>(box)->setPalette(*_palette);
		}
	}
}

void
AttributesEditor::impulsiveChanged()
{
	switch (_shapeOptionImpulsive->checkState()){
	case Qt::Unchecked :
		_palette->setImpulsive(false);
		break;
	case Qt::Checked :
		_palette->setImpulsive(true);
		break;
	default :
		_palette->setImpulsive(false);
		break;
	}
	profilesChanged();
}

void
AttributesEditor::shapeChanged() {
	Shape shape = Shape(_shapeComboBox->currentIndex());
	switch (_shapeOptionRandom->checkState()){
	case Qt::Unchecked :
		_palette->setShape(shape);
		break;
	case Qt::Checked :
		static const int numShapes = 5;
		_palette->setShape(Shape((int)shape + numShapes));
		break;
	default :
		_palette->setShape(Shape(Flat));
		break;
	}
	profilesChanged();
}

void
AttributesEditor::speedHeldChanged() {
	Speed speed = Speed(_speedHeldComboBox->currentIndex());

	if (_speedOptionRandom->checkState() == Qt::Checked)
		_palette->setSpeed(Speed((int)speed+4));
	else
		_palette->setSpeed(speed);

	profilesChanged();
}

void
AttributesEditor::speedVariationChanged() {
	SpeedVariation speedVariation = SpeedVariation(_speedVariationComboBox->currentIndex());
	_palette->setSpeedVariation(speedVariation);
	profilesChanged();
}

void
AttributesEditor::grainChanged() {
	Grain grain = Grain(_grainComboBox->currentIndex());
	_palette->setGrain(grain);
	profilesChanged();
}

void
AttributesEditor::pitchEndChecked(int state) {
	if (state == Qt::Checked){
		_pitchEndComboBox->setEnabled(true);
		pitchStartChanged();
		pitchEndChanged();
		pitchAmplitudeChanged();
		pitchGradeChanged();
	}
	else if (state == Qt::Unchecked){
		_pitchEndComboBox->setDisabled(true);
		pitchStartChanged();
		pitchEndChanged();
		pitchAmplitudeChanged();
		pitchGradeChanged();
	}
}

void
AttributesEditor::pitchRandomChecked(int state) {
	if ( state == Qt::Checked ) {
		_pitchOptionVibrato->setCheckState(Qt::Unchecked);
	}
	pitchStartChanged();
	pitchEndChanged();
	pitchAmplitudeChanged();
	pitchGradeChanged();
}

void
AttributesEditor::pitchVibratoChecked(int state) {
	if ( state == Qt::Checked )
		_pitchOptionRandom->setCheckState(Qt::Unchecked);
	pitchStartChanged();
	pitchEndChanged();
	pitchAmplitudeChanged();
	pitchGradeChanged();
}

void
AttributesEditor::pitchStartChanged() {

	Pitch pitchStart = Pitch(_pitchStartComboBox->currentIndex());

	if (pitchStart == PitchNone){
		_pitchOptionRandom->setDisabled(true);
		_pitchOptionVibrato->setDisabled(true);
		_pitchAmplitudeComboBox->setDisabled(true);
		_pitchGradeComboBox->setDisabled(true);
	}
	else{
		_pitchOptionRandom->setEnabled(true);
		_pitchOptionVibrato->setEnabled(true);
		_pitchAmplitudeComboBox->setEnabled(true);
		_pitchGradeComboBox->setEnabled(true);
	}

	_palette->setPitchStart(pitchStart);

	if (_pitchOptionRandom->checkState() == Qt::Checked){ // Random
		_palette->setPitchRandom(true);
	_palette->setPitchVibrato(false);
	}
	else if (_pitchOptionVibrato->checkState() == Qt::Checked) { // Vibrato
		_palette->setPitchVibrato(true);
	_palette->setPitchRandom(false);
	}
	else { // No vibrato No random
		_palette->setPitchRandom(false);
		_palette->setPitchVibrato(false);
	}

	profilesChanged();
}

void
AttributesEditor::pitchEndChanged() {
	if (_pitchEnd->checkState() == Qt::Checked) {
		Pitch pitchEnd = Pitch(_pitchEndComboBox->currentIndex());
		_palette->setPitchEnd(pitchEnd);
	}
	else if (_pitchEnd->checkState() == Qt::Unchecked) {
		_palette->setPitchEnd(PitchNone);
		_palette->setPitchAmplitude(PitchVariationNone);
		_palette->setPitchGrade(PitchVariationNone);
	}
	profilesChanged();
}

void
AttributesEditor::pitchAmplitudeChanged() {
	PitchVariation pitchAmp = PitchVariation(_pitchAmplitudeComboBox->currentIndex());
	_palette->setPitchAmplitude(pitchAmp);
	profilesChanged();
}

void
AttributesEditor::pitchGradeChanged() {
	PitchVariation pitchGrade = PitchVariation(_pitchGradeComboBox->currentIndex());
	_palette->setPitchGrade(pitchGrade);
	profilesChanged();
}

void
AttributesEditor::harmoHeldChanged() {
	Harmo harmoHeld = Harmo(_harmoHeldComboBox->currentIndex());
	_palette->setHarmo(harmoHeld);
	profilesChanged();
}

void
AttributesEditor::harmoVariationChanged() {
	HarmoVariation harmoVariation = HarmoVariation(_harmoVariationComboBox->currentIndex());
	_palette->setHarmoVariation(harmoVariation);
	profilesChanged();
}

void
AttributesEditor::changeColor() {

	// Passer plutot par un QtColorPicker pour limiter le nb de couleurs?
	QColor color = QColorDialog::getColor(Qt::white, this);
	if (color.isValid()) {
		_profilesColorButton->setAutoFillBackground(true);
		_profilesPreviewArea->setColor(color);
		//_profilesPreviewArea->setBrush(QBrush(QColor(color.red(),color.green(),color.blue(),127)));
		_palette->setColor(color);
		profilesChanged();
	}
}

void
AttributesEditor::startMsgApplied()
{
	vector<string> msgs = _startMsgsEditor->computeMessages();
	Maquette::getInstance()->setFirstMessagesToSend(_boxEdited,msgs);
	_curvesWidget->updateMessages(_boxEdited);
}

void
AttributesEditor::startMsgCanceled()
{
/*	_startMsgsEditor->clear();
	_startMsgsEditor->addMessages(Maquette::getInstance()->firstMessagesToSend(_boxEdited));*/
}

void
AttributesEditor::startMsgCleared()
{
	_startMsgsEditor->clear();
}

void
AttributesEditor::startMsgCopied()
{
	_startMsgsEditor->exportMessages();
}

void
AttributesEditor::startMsgPasted()
{
	_startMsgsEditor->importMessages();
	startMsgApplied();
}

void
AttributesEditor::endMsgApplied()
{
	vector<string> msgs = _endMsgsEditor->computeMessages();
	Maquette::getInstance()->setLastMessagesToSend(_boxEdited,msgs);
	_curvesWidget->updateMessages(_boxEdited);
}

void
AttributesEditor::endMsgCanceled()
{
/*	_endMsgsEditor->clear();
	_endMsgsEditor->addMessages(Maquette::getInstance()->lastMessagesToSend(_boxEdited));*/
}

void
AttributesEditor::endMsgCleared()
{
	_endMsgsEditor->clear();
	endMsgApplied();
}

void
AttributesEditor::endMsgCopied()
{
	_endMsgsEditor->exportMessages();
}

void
AttributesEditor::endMsgPasted()
{
	_endMsgsEditor->importMessages();
	endMsgApplied();
}

QString
AttributesEditor::getAbsoluteAddress(QTreeWidgetItem *item) const {
	QString address;
	QTreeWidgetItem * curItem = item;
	while (curItem != NULL) {
		QString node;
		if (curItem->parent() != NULL) {
			node.append("/");
		}
		node.append(curItem->text(0));
		address.insert(0,node);
		curItem = curItem->parent();
	}
	return address;
}

void
AttributesEditor::networkTreeCollapsed() {
	QTreeWidgetItem *selectedItem = _networkTree->currentItem();
	if (!selectedItem->isDisabled()) {
		if (selectedItem->type() == NodeNamespaceType || selectedItem->type() == LeaveType) {
			QString address = getAbsoluteAddress(selectedItem);

			vector<string> nodes,leaves,attributes,attributesValues;

			Maquette::getInstance()->requestNetworkNamespace(address.toStdString(), nodes, leaves, attributes, attributesValues);

			QList<QTreeWidgetItem *> selectedItemChildren = selectedItem->takeChildren();
			QList<QTreeWidgetItem *>::iterator childIt;
			for (childIt = selectedItemChildren.begin() ; childIt != selectedItemChildren.end() ; ++childIt) {
				delete *childIt;
			}
			vector<string>::iterator it;
			vector<string>::iterator it2;
			for (it = nodes.begin() ; it != nodes.end() ; ++it) {
				QStringList list;
				list << QString::fromStdString(*it);
				QTreeWidgetItem *childItem = new QTreeWidgetItem(list,NodeNamespaceType);
				//childItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable);
				childItem->setBackground(0,QBrush(Qt::darkCyan));
				selectedItem->addChild(childItem);
				list.clear();
			}
			for (it = leaves.begin() ; it != leaves.end() ; ++it) {
				QStringList list;
				list << QString::fromStdString(*it);
				QTreeWidgetItem *childItem = new QTreeWidgetItem(list,LeaveType);
				childItem->setBackground(0,QBrush(Qt::darkGreen));
				selectedItem->addChild(childItem);
				list.clear();
			}
			for (it = attributes.begin(),it2 = attributesValues.begin() ; it != attributes.end(), it2 != attributesValues.end() ; ++it , ++it2) {
				QStringList list;
				list << QString::fromStdString(*it + " : " + *it2);
				QTreeWidgetItem *childItem = new QTreeWidgetItem(list,AttributeType);
				childItem->setBackground(0,QBrush(Qt::green));
				selectedItem->addChild(childItem);
				list.clear();
			}
			//_networkTree->adjustSize();
		}
	}
}

void
AttributesEditor::assignSnapshotStart()
{
	QList<QTreeWidgetItem*> selectedItems = _networkTree->selectedItems();
	if (!selectedItems.empty()) {
		QList<QTreeWidgetItem*>::iterator it;
		vector<string> snapshots;
		vector<string>::iterator it2;

		for (it = selectedItems.begin(); it != selectedItems.end() ; ++it) {
			QString address = getAbsoluteAddress(*it);
			if (!address.isEmpty()) {
				vector<string> snapshot = Maquette::getInstance()->requestNetworkSnapShot(address.toStdString());
				snapshots.insert(snapshots.end(),snapshot.begin(),snapshot.end());
			}
		}

		if (Maquette::getInstance()->getBox(_boxEdited) != NULL) {
			if (!snapshots.empty()) {
				_startMsgsEditor->addMessages(snapshots);
				startMsgApplied();
				_scene->displayMessage("Snapshot successfully captured and applied to box start",INDICATION_LEVEL);
			}
			else {
				_scene->displayMessage("No snapshot taken for selection",INDICATION_LEVEL);
			}
		}
		else {
			_scene->displayMessage("No box selected during snapshot assignment",INDICATION_LEVEL);
		}
	}
	else {
		_scene->displayMessage("No selection found during snapshot",INDICATION_LEVEL);
	}
}

void AttributesEditor::assignSnapshotEnd()
{
	QList<QTreeWidgetItem*> selectedItems = _networkTree->selectedItems();
	if (!selectedItems.empty()) {
		QList<QTreeWidgetItem*>::iterator it;
		vector<string> snapshots;
		vector<string>::iterator it2;

		for (it = selectedItems.begin(); it != selectedItems.end() ; ++it) {
			QString address = getAbsoluteAddress(*it);
			if (!address.isEmpty()) {
				vector<string> snapshot = Maquette::getInstance()->requestNetworkSnapShot(address.toStdString());
				snapshots.insert(snapshots.end(),snapshot.begin(),snapshot.end());
			}
		}

		if (Maquette::getInstance()->getBox(_boxEdited) != NULL) {
			if (!snapshots.empty()) {
				_endMsgsEditor->addMessages(snapshots);
				endMsgApplied();
				_scene->displayMessage("Snapshot successfully captured and applied to box end",INDICATION_LEVEL);
			}
			else {
				_scene->displayMessage("No snapshot taken for selection",INDICATION_LEVEL);
			}
		}
		else {
			_scene->displayMessage("No box selected during snapshot assignment",INDICATION_LEVEL);
		}
	}
	else {
		_scene->displayMessage("No selection found during snapshot",INDICATION_LEVEL);
	}
}

