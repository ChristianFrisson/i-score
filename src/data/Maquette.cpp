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
#include <unistd.h>
#include <stdlib.h>

#include "Maquette.hpp"

#include <QFile>
#include "MainWindow.hpp"
#include "MaquetteView.hpp"
#include <QDomDocument>
#include "SoundBox.hpp"
#include "ControlBox.hpp"
#include "ParentBox.hpp"
#include "Palette.hpp"
#include "Engines.hpp"
#include "AbstractRelation.hpp"
#include "AbstractSoundBox.hpp"
#include "Relation.hpp"
#include "TriggerPoint.hpp"
#include <algorithm>
#include <QTextStream>
#include "AttributesEditor.hpp"
#include "NetworkTree.hpp"

#include <stdio.h>
#include <assert.h>
#include <QCoreApplication>

using std::vector;
using std::map;
using std::string;
using std::stringstream;
using std::list;
using std::pair;

typedef map<unsigned int,BasicBox*> BoxesMap;
typedef map<unsigned int,Relation*> RelationsMap;
typedef map<unsigned int,TriggerPoint*> TrgPntMap;

using namespace SndBoxProp;

void
Maquette::init() {
/*	char buf[256];
	getwd(buf);
	string pluginsDir;
	pluginsDir.append(buf);
	pluginsDir.append("/plugins");*/
	string pluginsDir = "/usr/local/lib/IScore";

	_engines = new Engines(SCENARIO_SIZE,pluginsDir);

    _engines->getLoadedNetworkPlugins(_plugins,_listeningPorts);

    //pour maintenir le fonctionnement pendant le developpement : l'appli n'est pas auto portée.
    if(_plugins.empty()){
        string pluginsDir =  (QCoreApplication::applicationDirPath()+"/../plugins/i-score").toStdString();
        _engines = new Engines(SCENARIO_SIZE,pluginsDir);
        _engines->getLoadedNetworkPlugins(_plugins,_listeningPorts);
        if(_plugins.empty()){
            string error;
            error.append(tr("No network plugins found in ").toStdString());
            error.append(pluginsDir);
            _scene->displayMessage(error,ERROR_LEVEL);
        }
        else{
            vector<string>::iterator it;
            vector<unsigned int>::iterator it2;
            for (it = _plugins.begin(), it2 = _listeningPorts.begin();
                (it != _plugins.end()) && (it2 != _listeningPorts.end()) ;
                it++,it2++) {
                stringstream deviceName;
                deviceName << *it << "Device";
                MyDevice device(deviceName.str(),*it,*it2,NETWORK_LOCALHOST);
                _devices[device.name] = device;
            }

            map<string,MyDevice>::iterator deviceIt;
            deviceIt = _devices.find("MinuitDevice");
            if (deviceIt != _devices.end()) {
                MyDevice minuitDevice = deviceIt->second;
                minuitDevice.name = "MinuitDevice1";
                minuitDevice.networkPort = 9998;
                minuitDevice.networkHost = "127.0.0.1";
                _devices[minuitDevice.name] = minuitDevice;
                stringstream port;
                port << minuitDevice.networkPort;
                _engines->addNetworkDevice(minuitDevice.name,minuitDevice.plugin,minuitDevice.networkHost,port.str());
            }

            deviceIt = _devices.find("OSCDevice");
            if (deviceIt != _devices.end()) {
                MyDevice oscDevice = deviceIt->second;
                oscDevice.name = "OSCDevice";
                oscDevice.networkPort = OSC_NETWORK_PORT;
                oscDevice.networkHost = "127.0.0.1";
                _devices[oscDevice.name] = oscDevice;
                stringstream port;
                port << oscDevice.networkPort;
                _engines->addNetworkDevice(oscDevice.name,oscDevice.plugin,oscDevice.networkHost,port.str());
            }
            _devices.erase("MinuitDevice");
        }
    }
    else{
        vector<string>::iterator it;
        vector<unsigned int>::iterator it2;
        for (it = _plugins.begin(), it2 = _listeningPorts.begin();
            (it != _plugins.end()) && (it2 != _listeningPorts.end()) ;
            it++,it2++) {
            stringstream deviceName;
            deviceName << *it << "Device";
            MyDevice device(deviceName.str(),*it,*it2,NETWORK_LOCALHOST);
            _devices[device.name] = device;
        }

        map<string,MyDevice>::iterator deviceIt;
        deviceIt = _devices.find("MinuitDevice");
        if (deviceIt != _devices.end()) {
            MyDevice minuitDevice = deviceIt->second;
            minuitDevice.name = "MinuitDevice1";
            minuitDevice.networkPort = 9998;
            minuitDevice.networkHost = "127.0.0.1";
            _devices[minuitDevice.name] = minuitDevice;
            stringstream port;
            port << minuitDevice.networkPort;
            _engines->addNetworkDevice(minuitDevice.name,minuitDevice.plugin,minuitDevice.networkHost,port.str());
        }

        deviceIt = _devices.find("OSCDevice");
        if (deviceIt != _devices.end()) {
            MyDevice oscDevice = deviceIt->second;
            oscDevice.name = "OSCDevice";
            oscDevice.networkPort = OSC_NETWORK_PORT;
            oscDevice.networkHost = "127.0.0.1";
            _devices[oscDevice.name] = oscDevice;
            stringstream port;
            port << oscDevice.networkPort;
            _engines->addNetworkDevice(oscDevice.name,oscDevice.plugin,oscDevice.networkHost,port.str());            
        }
        _devices.erase("MinuitDevice");
    }

	_engines->addCrossingCtrlPointCallback(&crossTransitionCallback);
    _engines->addCrossingTrgPointCallback(&crossTriggerPointCallback);
    _engines->addExecutionFinishedCallback(&executionFinishedCallback);
    _engines->addEnginesNetworkUpdateCallback(&enginesNetworkUpdateCallback);
}

Maquette::Maquette() {
	//init();
}

Maquette::~Maquette()
{
	_boxes.clear();
	_parentBoxes.clear();
	delete _engines;
}

void
Maquette::setScene(MaquetteScene *scene) {
	_scene = scene;
}

vector<string>
Maquette::getPlugins(){
    return _plugins;
}

vector<unsigned int>
Maquette::getRelationsIDs(unsigned int boxID)
{
	vector<unsigned int> boxRelations;
	RelationsMap::iterator rel;
	unsigned int relID1 = NO_ID;
	unsigned int relID2 = NO_ID;
	for (rel = _relations.begin() ; rel != _relations.end() ; rel++) {
		if (rel->first != NO_ID) {
			relID1 = _engines->getRelationFirstBoxId(rel->first);
			relID2 = _engines->getRelationSecondBoxId(rel->first);
			if (boxID == relID1 || boxID == relID2) {
				boxRelations.push_back(rel->first);
			}
		}
	}

	return boxRelations;
}

Relation*
Maquette::getRelation(unsigned int ID) {
	RelationsMap::iterator it = _relations.find(ID);
	if (it != _relations.end()) {
		return it->second;
	}

	return NULL;
}

BasicBox*
Maquette::getBox(unsigned int ID)
{
	BoxesMap::iterator it = _boxes.find(ID);
	if (it != _boxes.end()) {
		return it->second;
	}

	return NULL;
}

map<unsigned int,ParentBox*>
Maquette::parentBoxes()
{
	return _parentBoxes;
}

unsigned int
Maquette::sequentialID()
{
	return _boxes.size();
}

unsigned int
Maquette::addSoundBox(unsigned int ID, const QPointF & corner1, const QPointF & corner2, const string & name,
		const Palette &pal, unsigned int mother)
{
	if (ID != NO_ID) {
		vector<string> firstMsgs;
		vector<string> lastMsgs;
		_engines->getCtrlPointMessagesToSend(ID, BEGIN_CONTROL_POINT_INDEX, firstMsgs);
		_engines->getCtrlPointMessagesToSend(ID, END_CONTROL_POINT_INDEX, lastMsgs);

		SoundBox *newBox = new SoundBox(corner1, corner2, _scene);

		newBox->setName(QString::fromStdString(name));

		_boxes[ID] = newBox;
		newBox->setID(ID);
		newBox->setPalette(pal);
		if (mother != NO_ID && mother != ROOT_BOX_ID) {
			BoxesMap::iterator it;
			if ((it = _boxes.find(mother)) != _boxes.end()) {
				if (it->second->type() == PARENT_BOX_TYPE) {
					newBox->setMother(mother);
				  static_cast<ParentBox*>(it->second)->addChild(ID);
				}
			}
		}

		newBox->setFirstMessagesToSend(firstMsgs);
		newBox->setLastMessagesToSend(lastMsgs);
		}

	return ID;
}

unsigned int
Maquette::addSoundBox(const QPointF & corner1, const QPointF & corner2, const string & name, unsigned int mother)
{
	QPointF firstCorner(std::min(corner1.x(),corner2.x()),std::min(corner1.y(),corner2.y()));
	QPointF secondCorner(std::max(corner1.x(),corner2.x()),std::max(corner1.y(),corner2.y()));

	SoundBox *newBox = new SoundBox(firstCorner, secondCorner, _scene);

	ParentBox *motherBox = NULL;
	int motherID = mother;
	if (_boxes.find(mother) == _boxes.end()) {
		motherID = ROOT_BOX_ID;
	}
	else {
		BoxesMap::iterator it;
		if ((it = _boxes.find(mother)) != _boxes.end()) {
			if (it->second->type() == PARENT_BOX_TYPE) {
				motherBox = static_cast<ParentBox*>(it->second);
				firstCorner.rx() -= motherBox->beginPos();
				secondCorner.rx() -= motherBox->beginPos();
			}
		}
	}
	unsigned int newBoxID = _engines->addBox(firstCorner.x() * MaquetteScene::MS_PER_PIXEL,
			(secondCorner.x()-firstCorner.x()) * MaquetteScene::MS_PER_PIXEL , motherID);

	if (newBoxID != NO_ID) {
		newBox->setName(QString::fromStdString(name));

		_boxes[newBoxID] = newBox;
		newBox->setID(newBoxID);
		if (motherBox != NULL) {
			newBox->setMother(motherID);
			motherBox->addChild(newBoxID);
		}
		_engines->setCtrlPointMessagesToSend(newBoxID,BEGIN_CONTROL_POINT_INDEX,newBox->firstMessagesToSend());
		_engines->setCtrlPointMessagesToSend(newBoxID,END_CONTROL_POINT_INDEX,newBox->lastMessagesToSend());
	}

	return newBoxID;
}

unsigned int
Maquette::addSoundBox(const AbstractSoundBox &abstract)
{
	return addSoundBox(abstract.topLeft(), QPointF(abstract.topLeft().x() + abstract.width() ,
			abstract.topLeft().y() + abstract.height()),
			abstract.name(),abstract.mother());
}

unsigned int
Maquette::addControlBox(unsigned int ID, const QPointF & corner1, const QPointF & corner2, const string & name, unsigned int mother)
{
	vector<string> firstMsgs;
	vector<string> lastMsgs;
	_engines->getCtrlPointMessagesToSend(ID, BEGIN_CONTROL_POINT_INDEX, firstMsgs);
	_engines->getCtrlPointMessagesToSend(ID, END_CONTROL_POINT_INDEX, lastMsgs);

	ControlBox *newBox = new ControlBox(corner1, corner2, _scene);

	if (ID != NO_ID) {
		newBox->setName(QString::fromStdString(name));

		_boxes[ID] = newBox;
		newBox->setID(ID);
		if (mother != NO_ID && mother != ROOT_BOX_ID) {
			BoxesMap::iterator it;
			if ((it = _boxes.find(mother)) != _boxes.end()) {
				if (it->second->type() == PARENT_BOX_TYPE) {
					newBox->setMother(mother);
				  static_cast<ParentBox*>(it->second)->addChild(ID);
				}
			}
		}

		newBox->setFirstMessagesToSend(firstMsgs);
		newBox->setLastMessagesToSend(lastMsgs);
	}

	return ID;
}

unsigned int
Maquette::addControlBox(const QPointF & corner1, const QPointF & corner2, const string & name, unsigned int mother)
{
	QPointF firstCorner(std::min(corner1.x(),corner2.x()),std::min(corner1.y(),corner2.y()));
	QPointF secondCorner(std::max(corner1.x(),corner2.x()),std::max(corner1.y(),corner2.y()));

	ControlBox *newBox = new ControlBox(firstCorner, secondCorner, _scene);

	ParentBox *motherBox = NULL;
	int motherID = mother;
	if (_boxes.find(mother) == _boxes.end()) {
		motherID = ROOT_BOX_ID;
	}
	else {
		BoxesMap::iterator it;
		if ((it = _boxes.find(mother)) != _boxes.end()) {
			if (it->second->type() == PARENT_BOX_TYPE) {
				motherBox = static_cast<ParentBox*>(it->second);
				firstCorner.rx() -= motherBox->beginPos();
				secondCorner.rx() -= motherBox->beginPos();
			}
		}
	}
	unsigned int newBoxID = _engines->addBox(firstCorner.x() * MaquetteScene::MS_PER_PIXEL,
			(secondCorner.x()-firstCorner.x()) * MaquetteScene::MS_PER_PIXEL , motherID);

	if (newBoxID != NO_ID) {
		newBox->setName(QString::fromStdString(name));

		_boxes[newBoxID] = newBox;
		newBox->setID(newBoxID);
		if (motherBox != NULL) {
			newBox->setMother(motherID);
			motherBox->addChild(newBoxID);
		}
		_engines->setCtrlPointMessagesToSend(newBoxID,BEGIN_CONTROL_POINT_INDEX,newBox->firstMessagesToSend());
		_engines->setCtrlPointMessagesToSend(newBoxID,END_CONTROL_POINT_INDEX,newBox->lastMessagesToSend());
	}

	return newBoxID;
}

unsigned int
Maquette::addControlBox(const AbstractControlBox &abstract)
{
	return addControlBox(abstract.topLeft(), QPointF(abstract.topLeft().x() + abstract.width() ,
			abstract.topLeft().y() + abstract.height()),abstract.name(), abstract.mother());
}

unsigned int
Maquette::addParentBox(unsigned int ID, const QPointF & corner1, const QPointF & corner2, const string & name, unsigned int mother)
{
	vector<string> firstMsgs;
	vector<string> lastMsgs;
	_engines->getCtrlPointMessagesToSend(ID, BEGIN_CONTROL_POINT_INDEX, firstMsgs);
	_engines->getCtrlPointMessagesToSend(ID, END_CONTROL_POINT_INDEX, lastMsgs);

    ParentBox *newBox = new ParentBox(corner1, corner2, _scene);

	if (ID != NO_ID) {
		newBox->setName(QString::fromStdString(name));

		_boxes[ID] = newBox;
		_parentBoxes[ID] = newBox;
		newBox->setID(ID);
		if (mother != NO_ID && mother != ROOT_BOX_ID) {
			BoxesMap::iterator it;
			if ((it = _boxes.find(mother)) != _boxes.end()) {
				if (it->second->type() == PARENT_BOX_TYPE) {
					newBox->setMother(mother);
				  static_cast<ParentBox*>(it->second)->addChild(ID);
				}
				else {
					newBox->setMother(ROOT_BOX_ID);
				}
			}
		}

		newBox->setFirstMessagesToSend(firstMsgs);
		newBox->setLastMessagesToSend(lastMsgs);
	}

	return ID;
}

unsigned int
Maquette::addParentBox(unsigned int ID, const unsigned int date, const unsigned int topLeftY, const unsigned int sizeY, const unsigned int duration, const string & name, unsigned int mother, QColor color)
{
    QPointF corner1(date/MaquetteScene::MS_PER_PIXEL,topLeftY);
    QPointF corner2((date+duration)/MaquetteScene::MS_PER_PIXEL,topLeftY+sizeY);

    vector<string> firstMsgs;
    vector<string> lastMsgs;
    _engines->getCtrlPointMessagesToSend(ID, BEGIN_CONTROL_POINT_INDEX, firstMsgs);
    _engines->getCtrlPointMessagesToSend(ID, END_CONTROL_POINT_INDEX, lastMsgs);

    ParentBox *newBox = new ParentBox(corner1, corner2, _scene);

    if (ID != NO_ID) {
        newBox->setName(QString::fromStdString(name));
        newBox->setColor(color);
        _boxes[ID] = newBox;
        _parentBoxes[ID] = newBox;
        newBox->setID(ID);
        if (mother != NO_ID && mother != ROOT_BOX_ID) {
            BoxesMap::iterator it;
            if ((it = _boxes.find(mother)) != _boxes.end()) {
                if (it->second->type() == PARENT_BOX_TYPE) {
                    newBox->setMother(mother);
                  static_cast<ParentBox*>(it->second)->addChild(ID);
                }
                else {
                    newBox->setMother(ROOT_BOX_ID);
                }
            }
        }

        newBox->setFirstMessagesToSend(firstMsgs);
        newBox->setLastMessagesToSend(lastMsgs);
    }

    return ID;

}

unsigned int
Maquette::addParentBox(const QPointF & corner1, const QPointF & corner2, const string & name, unsigned int mother)
{
    QPointF firstCorner(std::min(corner1.x(),corner2.x()),std::min(corner1.y(),corner2.y()));
        QPointF secondCorner(std::max(corner1.x(),corner2.x()),std::max(corner1.y(),corner2.y()));

        ParentBox *newBox = new ParentBox(firstCorner, secondCorner, _scene);

        ParentBox *motherBox = NULL;
        int motherID = mother;
        if (_boxes.find(mother) == _boxes.end()) {
            motherID = ROOT_BOX_ID;
        }
        else {
            BoxesMap::iterator it;
            if ((it = _boxes.find(mother)) != _boxes.end()) {
                if (it->second->type() == PARENT_BOX_TYPE) {
                    motherBox = static_cast<ParentBox*>(it->second);
                    firstCorner.rx() -= motherBox->beginPos();
                    secondCorner.rx() -= motherBox->beginPos();
                }
            }
        }
        unsigned int newBoxID = _engines->addBox(firstCorner.x() * MaquetteScene::MS_PER_PIXEL,
                (secondCorner.x()-firstCorner.x()) * MaquetteScene::MS_PER_PIXEL , motherID);

        if (newBoxID != NO_ID) {
            newBox->setName(QString::fromStdString(name));

            _boxes[newBoxID] = newBox;
            _parentBoxes[newBoxID] = newBox;
            newBox->setID(newBoxID);
            if (motherBox != NULL) {
                newBox->setMother(motherID);
                motherBox->addChild(newBoxID);
            }
            _engines->setCtrlPointMessagesToSend(newBoxID,BEGIN_CONTROL_POINT_INDEX,newBox->firstMessagesToSend());
            _engines->setCtrlPointMessagesToSend(newBoxID,END_CONTROL_POINT_INDEX,newBox->lastMessagesToSend());
            }

        return newBoxID;
}

unsigned int
Maquette::addParentBox(const AbstractParentBox &abstract)
{
	return addParentBox(abstract.topLeft(), QPointF(abstract.topLeft().x() + abstract.width() ,
			abstract.topLeft().y() + abstract.height()), abstract.name(), abstract.mother());
}

map<string,MyDevice>
Maquette::getNetworkDevices() {
	return _devices;
}

int
Maquette::requestNetworkNamespace(const string &address, vector<string>& nodes, vector<string>& leaves,
					 vector<string>& attributes, vector<string>& attributesValue) {
	return _engines->requestNetworkNamespace(address,nodes, leaves, attributes, attributesValue);
}

void
Maquette::changeNetworkDevice(const string &deviceName, const string &pluginName, const string &IP, const string &port) {
    if (_devices.find(deviceName) != _devices.end())
        removeNetworkDevice(deviceName);

    addNetworkDevice(deviceName,pluginName,IP,port);

    std::istringstream iss(port);
    unsigned int portInt;
    iss >> portInt;

//    MyDevice newDevice(deviceName,pluginName,portInt,IP);
//    _devices[deviceName] = newDevice;

//    _engines->addNetworkDevice(deviceName,pluginName,IP,port);
    _currentDevice = deviceName;
}

std::string
Maquette::getNetworkDevice(){
    return _currentDevice;
}

void
Maquette::removeNetworkDevice(string deviceName){
    _devices.erase(deviceName);
    _engines->removeNetworkDevice(deviceName);
}

void
Maquette::getNetworkDeviceNames(vector<string> &deviceName, vector<bool> &namespaceRequestable) {
	_engines->getNetworkDevicesName(deviceName,namespaceRequestable);
}

vector<string> Maquette::requestNetworkSnapShot(const string &address) {
	return _engines->requestNetworkSnapShot(address);
}

bool
Maquette::updateMessagesToSend(unsigned int boxID) {
	if (boxID != NO_ID) {
		_engines->setCtrlPointMessagesToSend(boxID,BEGIN_CONTROL_POINT_INDEX,static_cast<SoundBox*>(_boxes[boxID])->firstMessagesToSend());
		_engines->setCtrlPointMessagesToSend(boxID,END_CONTROL_POINT_INDEX,static_cast<SoundBox*>(_boxes[boxID])->lastMessagesToSend());
		return true;
	}
	return false;
}

vector<string>
Maquette::firstMessagesToSend(unsigned int boxID)
{
	vector<string> messages;
	if (boxID != NO_ID && (getBox(boxID) != NULL)) {
		_engines->getCtrlPointMessagesToSend(boxID,BEGIN_CONTROL_POINT_INDEX,messages);
	}
	return messages;
}

vector<string>
Maquette::lastMessagesToSend(unsigned int boxID)
{
	vector<string> messages;
	if (boxID != NO_ID && (getBox(boxID) != NULL)) {
		_engines->getCtrlPointMessagesToSend(boxID,END_CONTROL_POINT_INDEX,messages);
	}
	return messages;
}


void
Maquette::updateCurves(unsigned int boxID, const vector<string> &startMsgs, const vector<string> &endMsgs)
{    
    //QMap<address,value>
    QMap<string,string> startMessages;
    QMap<string,string> endMessages;
    vector<string> curvesAddresses = getCurvesAddresses(boxID);

    vector<string>::const_iterator it;
    string currentMsg;
    string currentAddress;
    string currentValue;

    /************  init start messages (QMap)  ************/
    for (it = startMsgs.begin() ; it != startMsgs.end() ; ++it) {
        size_t blankPos;
        currentMsg = *it;
        if ((blankPos = it->find_first_of(" ")) != string::npos) {
            currentAddress = currentMsg.substr(0,blankPos);
            if(blankPos != currentMsg.size())
                currentValue = currentMsg.substr(blankPos+1,currentMsg.size());
        }
        startMessages.insert(currentAddress,currentValue);
    }

    /************  init end messages (QMap)  ************/
    for (it = endMsgs.begin() ; it != endMsgs.end() ; ++it) {
        size_t blankPos;
        currentMsg = *it;
        if ((blankPos = it->find_first_of(" ")) != string::npos) {
            currentAddress = currentMsg.substr(0,blankPos);
            if(blankPos != currentMsg.size())
                currentValue = currentMsg.substr(blankPos+1,currentMsg.size());
        }
        endMessages.insert(currentAddress,currentValue);
    }


    /************    Pour le cas open file   ************/
    for (it = curvesAddresses.begin() ; it != curvesAddresses.end() ; ++it) {
        currentMsg = *it;
        getBox(boxID)->addCurveAddress(currentMsg);
        if(startMessages.contains(currentMsg)){
            startMessages.remove(currentMsg);
        }
        if(endMessages.contains(currentMsg)){
            endMessages.remove(currentMsg);
        }
    }


    /************  addCurve if endAddress contains startAddress && endValue != startValue ************/
    QList<string> startAddresses = startMessages.keys();
    QList<string>::iterator startAddressIt;

    for(startAddressIt = startAddresses.begin() ; startAddressIt != startAddresses.end() ; ++startAddressIt) {
        string address = *startAddressIt;
        if (endMessages.contains(address)) {
            if (std::find(curvesAddresses.begin(),curvesAddresses.end(),address) == curvesAddresses.end() && startMessages.value(address)!=endMessages.value(address)) {
                _engines->addCurve(boxID,address);
                getBox(boxID)->addCurve(address);
            }
        }
        else {
            if (std::find(curvesAddresses.begin(),curvesAddresses.end(),address) != curvesAddresses.end()) {
                _engines->removeCurve(boxID,address);
            }
        }
    }

    /************  addCurve if startAddress contains endAddress && endValue != startValue ************/
    QList<string> endAddresses = endMessages.keys();
    QList<string>::iterator endAddressIt;

    for(endAddressIt = endAddresses.begin() ; endAddressIt != endAddresses.end() ; ++endAddressIt) {
        string address = *endAddressIt;
        if (startMessages.contains(address)) {
            if (std::find(curvesAddresses.begin(),curvesAddresses.end(),address) == curvesAddresses.end() && startMessages.value(address)!=endMessages.value(address)) {
                _engines->addCurve(boxID,address);
                getBox(boxID)->addCurve(address);
            }
        }
        else {
            if (std::find(curvesAddresses.begin(),curvesAddresses.end(),address) != curvesAddresses.end()) {
                _engines->removeCurve(boxID,address);
            }
        }
    }
}

bool
Maquette::setFirstMessagesToSend(unsigned int boxID, const vector<string> &firstMsgs) {
    if (boxID != NO_ID && (getBox(boxID) != NULL)) {

       //PRINT
//           std::cout<<"---- Maquette::setFirstMsgsToSend ----"<<std::endl;
//           for(int i=0 ; i<firstMsgs.size(); i++ )
//               std::cout<< firstMsgs[i]<<std::endl;
//           std::cout<<std::endl<<std::endl;
       //PRINT
		_engines->setCtrlPointMessagesToSend(boxID,BEGIN_CONTROL_POINT_INDEX,firstMsgs);
		_boxes[boxID]->setFirstMessagesToSend(firstMsgs);

		vector<string> lastMsgs;
		_engines->getCtrlPointMessagesToSend(boxID,END_CONTROL_POINT_INDEX,lastMsgs);
		updateCurves(boxID,firstMsgs,lastMsgs);

		return true;
	}
	return false;
}

bool
Maquette::setStartMessagesToSend(unsigned int boxID, NetworkMessages *messages){

    vector<string> firstMsgs = messages->computeMessages();

    if (boxID != NO_ID && (getBox(boxID) != NULL)) {
        _engines->setCtrlPointMessagesToSend(boxID,BEGIN_CONTROL_POINT_INDEX,firstMsgs);
        _boxes[boxID]->setStartMessages(messages);

        vector<string> lastMsgs;
        _engines->getCtrlPointMessagesToSend(boxID,END_CONTROL_POINT_INDEX,lastMsgs);
        updateCurves(boxID,firstMsgs,lastMsgs);

        return true;
    }
    return false;
}

NetworkMessages *
Maquette::startMessages(unsigned int boxID){
    if (boxID != NO_ID && (getBox(boxID) != NULL))
        return _boxes[boxID]->startMessages();
    else{
        std::cerr<<"Maquette::startMessage : wrong boxID"<<std::endl;
        return NULL;
    }
}

bool
Maquette::setSelectedItemsToSend(unsigned int boxID,  QMap<QTreeWidgetItem*,Data> itemsSelected){
    if (boxID != NO_ID && (getBox(boxID) != NULL)) {

        _boxes[boxID]->setSelectedItemsToSend(itemsSelected);

        return true;
    }
    return false;
}

bool
Maquette::setExpandedItemsList(unsigned int boxID,  QList<QTreeWidgetItem*> itemsExpanded){
    if (boxID != NO_ID && (getBox(boxID) != NULL)) {

        _boxes[boxID]->setExpandedItemsList(itemsExpanded);

        return true;
    }
    return false;
}

bool
Maquette::addToExpandedItemsList(unsigned int boxID,  QTreeWidgetItem* item){
    if (boxID != NO_ID && (getBox(boxID) != NULL)) {

        _boxes[boxID]->addToExpandedItemsList(item);

        return true;
    }
    return false;
}

bool
Maquette::removeFromExpandedItemsList(unsigned int boxID,  QTreeWidgetItem* item){
    if (boxID != NO_ID && (getBox(boxID) != NULL)) {

        _boxes[boxID]->removeFromExpandedItemsList(item);

        return true;
    }
    return false;
}

bool
Maquette::setStartMessages(unsigned int boxID,  NetworkMessages* nm){
    if (boxID != NO_ID && (getBox(boxID) != NULL)) {

        _boxes[boxID]->setStartMessages(nm);
        return true;
    }
    return false;
}

bool
Maquette::setEndMessages(unsigned int boxID,  NetworkMessages* nm){
    if (boxID != NO_ID && (getBox(boxID) != NULL)) {

        _boxes[boxID]->setEndMessages(nm);
        return true;
    }
    return false;
}

NetworkMessages *
Maquette::endMessages(unsigned int boxID){
    if (boxID != NO_ID && (getBox(boxID) != NULL))
        return _boxes[boxID]->endMessages();
    else{
        std::cerr<<"Maquette::startMessage : wrong boxID"<<std::endl;
        return NULL;
    }
}

bool
Maquette::setLastMessagesToSend(unsigned int boxID, const vector<string> &lastMsgs) {
	if (boxID != NO_ID && (getBox(boxID) != NULL)) {
		_engines->setCtrlPointMessagesToSend(boxID,END_CONTROL_POINT_INDEX,lastMsgs);
		_boxes[boxID]->setLastMessagesToSend(lastMsgs);

		vector<string> firstMsgs;
		_engines->getCtrlPointMessagesToSend(boxID,BEGIN_CONTROL_POINT_INDEX,firstMsgs);
		updateCurves(boxID,firstMsgs,lastMsgs);

		return true;
	}
	return false;
}

bool
Maquette::setEndMessagesToSend(unsigned int boxID, NetworkMessages *messages) {

    vector<string> lastMsgs = messages->computeMessages();

    if (boxID != NO_ID && (getBox(boxID) != NULL)) {
        _engines->setCtrlPointMessagesToSend(boxID,END_CONTROL_POINT_INDEX,lastMsgs);
        _boxes[boxID]->setEndMessages(messages);

        vector<string> firstMsgs;
        _engines->getCtrlPointMessagesToSend(boxID,BEGIN_CONTROL_POINT_INDEX,firstMsgs);
        updateCurves(boxID,firstMsgs,lastMsgs);

        return true;
    }
    return false;
}

bool
Maquette::sendMessage(const string &message) {
	if (!message.empty()) {
		_engines->sendNetworkMessage(message);
		return true;
	}
	return false;
}

void
Maquette::clear()
{
	BoxesMap::iterator it;
	for (it = _boxes.begin() ; it != _boxes.end() ; it++) {
		removeBox(it->first);
	}
	RelationsMap::iterator it2;
	for (it2 = _relations.begin() ; it2 != _relations.end() ; it2++) {
		removeRelation(it2->first);
	}
	_relations.clear();
}

vector<unsigned int>
Maquette::removeBox(unsigned int boxID)
{
	vector<unsigned int> removedRelations;
	if (boxID != NO_ID) {
		RelationsMap::iterator it;
		unsigned int relID = NO_ID;
		for (it = _relations.begin(); it != _relations.end(); it++) {
			relID = it->first;
			if (relID != NO_ID) {
				if (_engines->getRelationFirstBoxId(relID) == boxID
						|| _engines->getRelationSecondBoxId(relID) == boxID) {
					removedRelations.push_back(relID);
				}
			}
		}

		_engines->removeBox(boxID);

		BoxesMap::iterator it2 = _boxes.find(boxID);
		if (it2 != _boxes.end()) {
			_boxes.erase(it2);
		}
		map<unsigned int,ParentBox*>::iterator it3 = _parentBoxes.find(boxID);
		if (it3 != _parentBoxes.end()) {
			_parentBoxes.erase(it3);
		}
	}

	return removedRelations;
}

void
Maquette::updateRelations() {
	//   RelationsMap::iterator it;
	//   for (it = _relations.begin() ; it != _relations.end() ; it++) {
		//     it->second->setPos(it->second->getCenter());
		//   }
}

bool Maquette::updateBox(unsigned int boxID, const Coords &coord) {
	bool moveAccepted;
	vector<unsigned int> moved;
	vector<unsigned int>::iterator it;
	if (boxID != NO_ID && boxID != ROOT_BOX_ID) {
		BasicBox *box = _boxes[boxID];
		if (moveAccepted = _engines->performBoxEditing(boxID, coord.topLeftX * MaquetteScene::MS_PER_PIXEL,
				coord.topLeftX * MaquetteScene::MS_PER_PIXEL +
                coord.sizeX * MaquetteScene::MS_PER_PIXEL, moved)) {
			box->setRelativeTopLeft(QPoint(coord.topLeftX,coord.topLeftY));
			box->setSize(QPoint(coord.sizeX,coord.sizeY));
			box->setPos(box->getCenter());
			box->update();
		}
        else {
			box->setRelativeTopLeft(QPoint(_engines->getBoxBeginTime(boxID)/MaquetteScene::MS_PER_PIXEL,
					box->getTopLeft().y()));
			box->setSize(QPoint((_engines->getBoxEndTime(boxID)/MaquetteScene::MS_PER_PIXEL -
					_engines->getBoxBeginTime(boxID)/MaquetteScene::MS_PER_PIXEL),
					box->getSize().y()));
			box->setPos(box->getCenter());
			box->update();
#ifdef DEBUG
			std::cerr << "Maquette::updateBox : Move refused by Engines" << std::endl;
#endif
		}
	}


	if (moveAccepted) {
		for (it = moved.begin() ; it != moved.end() ; it++) {
#ifdef DEBUG
			std::cerr << "Maquette::updateBoxes : box moved : " << *it << std::endl;
#endif
            if ((_boxes[*it]->relativeBeginPos() != _engines->getBoxBeginTime(*it)/MaquetteScene::MS_PER_PIXEL ||
                 (_engines->getBoxEndTime(*it)/MaquetteScene::MS_PER_PIXEL - _engines->getBoxBeginTime(*it)/MaquetteScene::MS_PER_PIXEL) != _boxes[*it]->width())&& _engines->getBoxBeginTime(*it)) {


				_boxes[*it]->setRelativeTopLeft(QPoint(_engines->getBoxBeginTime(*it)/MaquetteScene::MS_PER_PIXEL,
						_boxes[*it]->getTopLeft().y()));
				_boxes[*it]->setSize(QPoint((_engines->getBoxEndTime(*it)/MaquetteScene::MS_PER_PIXEL -
						_engines->getBoxBeginTime(*it)/MaquetteScene::MS_PER_PIXEL),
						_boxes[*it]->getSize().y()));
				_boxes[*it]->setPos(_boxes[*it]->getCenter());
				_boxes[*it]->update();
			}
		}
	}

	return moveAccepted;
}

bool
Maquette::updateBoxes(const map<unsigned int,Coords> &boxes) {

	bool moveAccepted;
	vector<unsigned int> moved;
	map<unsigned int,Coords >::const_iterator it;
	vector<unsigned int>::iterator it2;
	for (it = boxes.begin() ; it!= boxes.end() ; it++) {
		if (it->first != NO_ID && it->first != ROOT_BOX_ID) {
			BasicBox *curBox = _boxes[it->first];
			if (moveAccepted = _engines->performBoxEditing(it->first, it->second.topLeftX * MaquetteScene::MS_PER_PIXEL,
					it->second.topLeftX * MaquetteScene::MS_PER_PIXEL +
					it->second.sizeX * MaquetteScene::MS_PER_PIXEL, moved)) {
				curBox->setRelativeTopLeft(QPoint(it->second.topLeftX,it->second.topLeftY));
				curBox->setSize(QPoint(it->second.sizeX,it->second.sizeY));
				curBox->setPos(_boxes[it->first]->getCenter());
				curBox->update();
			}
			else {
				curBox->setRelativeTopLeft(QPoint(_engines->getBoxBeginTime(it->first)/MaquetteScene::MS_PER_PIXEL,
						curBox->getTopLeft().y()));
				curBox->setSize(QPoint((_engines->getBoxEndTime(it->first)/MaquetteScene::MS_PER_PIXEL -
						_engines->getBoxBeginTime(it->first)/MaquetteScene::MS_PER_PIXEL),
						curBox->getSize().y()));
				curBox->setPos(curBox->getCenter());
				curBox->update();
#ifdef DEBUG
				std::cerr << "Maquette::updateBoxes : Move refused by Engines" << std::endl;
#endif
			}
		}
	}

	if (moveAccepted) {
		for (it2 = moved.begin() ; it2 != moved.end() ; it2++) {
#ifdef DEBUG
			std::cerr << "Maquette::updateBoxes : box moved : " << *it2 << std::endl;
#endif
			if (_boxes[*it2]->relativeBeginPos() != _engines->getBoxBeginTime(*it2)/MaquetteScene::MS_PER_PIXEL ||
					(_engines->getBoxEndTime(*it2)/MaquetteScene::MS_PER_PIXEL - _engines->getBoxBeginTime(*it2)/MaquetteScene::MS_PER_PIXEL) != _boxes[*it2]->width()) {
				_boxes[*it2]->setRelativeTopLeft(QPoint(_engines->getBoxBeginTime(*it2)/MaquetteScene::MS_PER_PIXEL,
						_boxes[*it2]->getTopLeft().y()));
				_boxes[*it2]->setSize(QPoint((_engines->getBoxEndTime(*it2)/MaquetteScene::MS_PER_PIXEL -
						_engines->getBoxBeginTime(*it2)/MaquetteScene::MS_PER_PIXEL),
						_boxes[*it2]->getSize().y()));
				_boxes[*it2]->setPos(_boxes[*it2]->getCenter());
				_boxes[*it2]->update();
			}
		}
	}
    return moveAccepted;
}

void
Maquette::simulateTriggeringMessage(const string &message)
{
    _engines->simulateNetworkMessageReception(message);
}

int
Maquette::addTriggerPoint(const AbstractTriggerPoint &abstract) {
	if (abstract.ID() == NO_ID) {
		return addTriggerPoint(abstract.boxID(),abstract.boxExtremity(),abstract.message());
	}
	else {
		TriggerPoint* newTP = new TriggerPoint(abstract.boxID(),abstract.boxExtremity(),abstract.message(),abstract.ID(),_scene);
		newTP->setID(abstract.ID());
		_scene->addItem(newTP);

		_triggerPoints[abstract.ID()] = newTP;
        _boxes[abstract.boxID()]->addTriggerPoint(abstract.boxExtremity(),newTP);
		return (int)abstract.ID();
    }
	return RETURN_ERROR;
}

int
Maquette::addTriggerPoint(unsigned int boxID, BoxExtremity extremity, const string &message) {
	if (boxID == NO_ID) {
		return ARGS_ERROR;
	}

	unsigned int triggerID = _engines->addTriggerPoint(_boxes[boxID]->mother());

	unsigned int controlPointID = NO_ID;
	if (extremity == BOX_START) {
		controlPointID = BEGIN_CONTROL_POINT_INDEX;
	}
	else if (extremity == BOX_END) {
		controlPointID = END_CONTROL_POINT_INDEX;
	}
	if (controlPointID == NO_ID) {
		return RETURN_ERROR;
	}
	if (!_engines->assignCtrlPointToTriggerPoint(triggerID,boxID,controlPointID)) {
		_scene->displayMessage(tr("Trigger point already linked to a control point.").toStdString(),INDICATION_LEVEL);
		return NO_MODIFICATION;
	}
	else {
		_scene->displayMessage(tr("Trigger point succesfully added").toStdString(),INDICATION_LEVEL);
		_engines->setTriggerPointMessage(triggerID,message);
		TriggerPoint * newTP = new TriggerPoint(boxID,extremity,message,triggerID,_scene);
		_scene->addItem(newTP);
        _triggerPoints[triggerID] = newTP;
		_boxes[boxID]->addTriggerPoint(extremity,_triggerPoints[triggerID]);

		return triggerID;
    }

	return RETURN_ERROR;
}

void
Maquette::removeTriggerPoint(unsigned int ID)
{
	TrgPntMap::iterator it;
	if ((it = _triggerPoints.find(ID)) != _triggerPoints.end()) {
        _engines->removeTriggerPoint(ID);
		delete it->second;
		_triggerPoints.erase(it);

	}
}

TriggerPoint *
Maquette::getTriggerPoint(unsigned int ID)
{
	TrgPntMap::iterator it;
	if ((it = _triggerPoints.find(ID)) != _triggerPoints.end()) {
		return it->second;
	}
	return NULL;
}

bool
Maquette::setTriggerPointMessage(unsigned int trgID, const string &message) {
	bool ret = false;
	TrgPntMap::iterator it;
	if ((it = _triggerPoints.find(trgID)) != _triggerPoints.end()) {
		_engines->setTriggerPointMessage(trgID,message);
		ret = true;
	}
	return ret;
}

void Maquette::addCurve(unsigned int boxID, const string &address)
{
	_engines->addCurve(boxID,address);
}

void Maquette::removeCurve(unsigned int boxID, const string &address)
{
	_engines->removeCurve(boxID,address);
}

void Maquette::clearCurves(unsigned int boxID)
{
	_engines->clearCurves(boxID);
}

vector<string> Maquette::getCurvesAddresses(unsigned int boxID)
{
	return _engines->getCurvesAddress(boxID);
}

void Maquette::setCurveRedundancy(unsigned int boxID, const string &address, bool redundancy) {
	_engines->setCurveRedundancy(boxID,address,redundancy);
}

bool
Maquette::getCurveRedundancy(unsigned int boxID, const std::string &address){
    return _engines->getCurveRedundancy(boxID,address);
}

void Maquette::setCurveSampleRate(unsigned int boxID, const string &address, int sampleRate) {
    _engines->setCurveSampleRate(boxID,address,sampleRate);
}

unsigned int
Maquette::getCurveSampleRate(unsigned int boxID, const std::string &address){
    return _engines->getCurveSampleRate(boxID,address);
}

void Maquette::setCurveMuteState(unsigned int boxID, const string &address, bool muteState) {
	_engines->setCurveMuteState(boxID,address,muteState);
}

bool Maquette::getCurveMuteState(unsigned int boxID, const string &address) {
    return _engines->getCurveMuteState(boxID,address);
}

bool Maquette::setCurveSections(unsigned int boxID, const string &address, unsigned int argPosition,
			const vector<float> &xPercents, const vector<float> &yValues, const vector<short> &sectionType, const vector<float> &coeff)
{
    return _engines->setCurveSections(boxID,address,argPosition,xPercents,yValues,sectionType,coeff);
}

bool Maquette::getCurveAttributes(unsigned int boxID, const std::string &address, unsigned int argPosition,
	unsigned int &sampleRate, bool &redundancy, bool &interpolate, vector<float>& values, vector<string> &argTypes,
	vector<float> &xPercents, vector<float> &yValues, vector<short> &sectionType, vector<float> &coeff) {

	if (_engines->getCurveValues(boxID,address,argPosition,values)) {
        if (_engines->getCurveSections(boxID,address,argPosition,xPercents,yValues,sectionType,coeff)) {
			sampleRate = _engines->getCurveSampleRate(boxID,address);
			redundancy = _engines->getCurveRedundancy(boxID,address);
			interpolate = !_engines->getCurveMuteState(boxID,address);
			_engines->getCurveArgTypes(address,argTypes);
			return true;
		}
	}

	return false;
}

void
Maquette::updateBoxesFromEngines(const vector<unsigned int> &movedBoxes)
{
	vector<unsigned int>::const_iterator it;
	if (!movedBoxes.empty()) {
        for (it = movedBoxes.begin() ; it != movedBoxes.end() ; it++) {
            if ((_boxes[*it]->relativeBeginPos() != _engines->getBoxBeginTime(*it)/MaquetteScene::MS_PER_PIXEL ||
                    (_engines->getBoxEndTime(*it)/MaquetteScene::MS_PER_PIXEL - _engines->getBoxBeginTime(*it)/MaquetteScene::MS_PER_PIXEL) != _boxes[*it]->width())) {
				_boxes[*it]->setRelativeTopLeft(QPoint(_engines->getBoxBeginTime(*it)/MaquetteScene::MS_PER_PIXEL,
						_boxes[*it]->getTopLeft().y()));
				_boxes[*it]->setSize(QPoint((_engines->getBoxEndTime(*it)/MaquetteScene::MS_PER_PIXEL -
						_engines->getBoxBeginTime(*it)/MaquetteScene::MS_PER_PIXEL),
						_boxes[*it]->getSize().y()));
				_boxes[*it]->setPos(_boxes[*it]->getCenter());
				_boxes[*it]->update();
			}
		}
	}
}

void
Maquette::updateBoxesFromEngines()
{
	BoxesMap::iterator it;
	for (it = _boxes.begin() ; it != _boxes.end() ; ++it) {
		it->second->setRelativeTopLeft(QPoint(_engines->getBoxBeginTime(it->first)/MaquetteScene::MS_PER_PIXEL,
				it->second->getTopLeft().y()));
		it->second->setSize(QPoint((_engines->getBoxEndTime(it->first)/MaquetteScene::MS_PER_PIXEL -
				_engines->getBoxBeginTime(it->first)/MaquetteScene::MS_PER_PIXEL),
				it->second->getSize().y()));
		it->second->setPos(it->second->getCenter());
        it->second->centerWidget();
		it->second->update();        
	}
}


int
Maquette::addRelation(unsigned int ID1, BoxExtremity firstExtremum, unsigned int ID2,
		BoxExtremity secondExtremum, int antPostType) {

	if (ID1 == NO_ID || ID2 == NO_ID) {
		return ARGS_ERROR;
	}
	vector<unsigned int> movedBoxes;
	unsigned int controlPointID1 = NO_ID;
	unsigned int controlPointID2 = NO_ID;
	if (firstExtremum == BOX_START) {
		controlPointID1 = BEGIN_CONTROL_POINT_INDEX;
	}
	else if (firstExtremum == BOX_END) {
		controlPointID1 = END_CONTROL_POINT_INDEX;
	}
	if (secondExtremum == BOX_START) {
		controlPointID2 = BEGIN_CONTROL_POINT_INDEX;
	}
	else if (secondExtremum == BOX_END) {
		controlPointID2 = END_CONTROL_POINT_INDEX;
	}

	unsigned int relationID = NO_ID;
	if (_engines->isTemporalRelationExisting(ID1,controlPointID1,ID2,controlPointID2)) {
		return NO_MODIFICATION;
	}

	relationID = _engines->addTemporalRelation(ID1,controlPointID1,ID2,controlPointID2,
			TemporalRelationType(antPostType),movedBoxes);

	if (!_engines->isTemporalRelationExisting(ID1,controlPointID1,ID2,controlPointID2)) {
		return RETURN_ERROR;
	}

	if (relationID != NO_ID) {
		Relation* newRel = new Relation(ID1,firstExtremum,ID2,secondExtremum,_scene);
		_relations[relationID] = newRel;

		_boxes[ID1]->addRelation(firstExtremum,newRel);
        _boxes[ID2]->addRelation(secondExtremum,newRel);
		_scene->addItem(newRel);
		updateBoxesFromEngines(movedBoxes);
        _scene->boxesMoved(movedBoxes);
		return (int)relationID;
	}

	return RETURN_ERROR;
}

int
Maquette::addRelation(const AbstractRelation &abstract) {
    if (abstract.ID() == NO_ID) {
		return addRelation(abstract.firstBox(),abstract.firstExtremity(),abstract.secondBox(),abstract.secondExtremity(),ANTPOST_ANTERIORITY);
	}
	else {
        Relation* newRel = new Relation(abstract.firstBox(),abstract.firstExtremity(),abstract.secondBox(),abstract.secondExtremity(),_scene);
		newRel->setID(abstract.ID());
        newRel->changeBounds(abstract.minBound(),abstract.maxBound());

		_relations[abstract.ID()] = newRel;
		_scene->addItem(newRel);
		_boxes[abstract.firstBox()]->addRelation(abstract.firstExtremity(),newRel);
        _boxes[abstract.secondBox()]->addRelation(abstract.secondExtremity(),newRel);
		newRel->updateCoordinates();        
		return (int)abstract.ID();
	}
}

bool
Maquette::addInterval(unsigned int /*ID1*/, unsigned int /*ID2*/, int /*value*/, int /*tolerance*/)
{
	return false;
}

void
Maquette::removeRelation(unsigned int relationID)
{
	RelationsMap::iterator it;
	if ((it = _relations.find(relationID)) != _relations.end()) {
		_engines->removeTemporalRelation(relationID);
		_relations.erase(it);
	}
}

bool
Maquette::areRelated(unsigned int ID1, unsigned int  ID2)
{
	RelationsMap::iterator rel;
	unsigned int relID1 = NO_ID;
	unsigned int relID2 = NO_ID;
	for (rel = _relations.begin() ; rel != _relations.end() ; rel++) {
		relID1 = _engines->getRelationFirstBoxId(rel->first);
		relID2 = _engines->getRelationSecondBoxId(rel->first);
		if ((relID1 == ID1 && relID2 == ID2) || (relID1 == ID2 && relID2 == ID1)) {
			return true;
		}
	}
	return false;
}

void
Maquette::changeRelationBounds(unsigned int relID, const float &minBound, const float &maxBound)
{
	vector<unsigned int> movedBoxes;
	int minBoundMS = NO_BOUND;
	if (minBound != NO_BOUND) {
        minBoundMS = minBound * (MaquetteScene::MS_PER_PIXEL*_scene->zoom());
	}
	int maxBoundMS = NO_BOUND;
	if (maxBound != NO_BOUND) {
        maxBoundMS = maxBound * (MaquetteScene::MS_PER_PIXEL*_scene->zoom());
	}
    _engines->changeTemporalRelationBounds(relID,minBoundMS,maxBoundMS,movedBoxes);
    updateBoxesFromEngines(movedBoxes);
}

int
Maquette::duration()
{
	int duration = 0;

	for (BoxesMap::iterator it = _boxes.begin() ; it != _boxes.end() ; it++) {
		if ((it->second->beginPos() + it->second->width()) * MaquetteScene::MS_PER_PIXEL > duration ) {
			duration = (it->second->beginPos() + it->second->width()) * MaquetteScene::MS_PER_PIXEL;
		}
	}

	return duration;
}

unsigned int
Maquette::getCurrentTime() const {
	return _engines->getCurrentExecutionTime();
}

float
Maquette::getProgression(unsigned int boxID)
{
	return (float)_engines->getProcessProgression(boxID);
}

void
Maquette::setGotoValue(int gotoValue) {
    _scene->view()->setGotoValue(gotoValue);
    _engines->setGotoValue(gotoValue);
}

void
Maquette::generateTriggerQueue(){
    _scene->triggersQueueList().clear();
    TrgPntMap::iterator it1;
    for(it1=_triggerPoints.begin() ; it1!=_triggerPoints.end() ; ++it1){
        _scene->addToTriggerQueue(it1->second);
    }
}

void
Maquette::initSceneState(){
    //Pour palier au bug du moteur (qui envoyait tous les messages début et fin de toutes les boîtes < Goto)

    double gotoValue = (double)_engines->getGotoValue();

    unsigned int boxID;
    QMap<QString,QPair<QString,unsigned int> > msgs, boxMsgs;
    QList<QString> boxAddresses;
    BasicBox *currentBox;
    std::vector<string> curvesList;

    //Pour toutes les boîtes avant le goto, on récupère leur état final (on simule leur exécution)
    for (BoxesMap::iterator it = _boxes.begin() ; it != _boxes.end() ; it++) {
        boxID = it->first;
        currentBox = (*it).second;

        //réinit : On démute toutes les boîtes, elles ont potentiellement pu être mutées à la fin de l'algo
        _engines->setCtrlPointMutingState(boxID,1,false);
        _engines->setCtrlPointMutingState(boxID,2,false);

        if(currentBox->date() < gotoValue && (currentBox->date()+currentBox->duration()) <= gotoValue ){
            boxMsgs = currentBox->getFinalState();

        }
        else if(gotoValue > currentBox->date() && gotoValue < (currentBox->date()+currentBox->duration())){
            //goto au milieu d'une boîte : On envoie la valeur du début de boîte            
            boxMsgs = currentBox->getStartState();            
            curvesList = _engines->getCurvesAddress(boxID);

            //On supprime les messages si ils sont déjà associés à une courbe (le moteur les envoie automatiquement)
            for (unsigned int i=0 ; i<curvesList.size() ; i++){
                //sauf si la courbe a été désactivée manuellement
                if(!getCurveMuteState(boxID,curvesList[i])){
                    msgs.remove(QString::fromStdString(curvesList[i]));
                }
            }
            std::cout<<std::endl;
        }
        else if(gotoValue == currentBox->date()){            
            boxMsgs = currentBox->getStartState();            
        }


        boxAddresses = boxMsgs.keys();

        //Pour le cas où le même paramètre est modifié par plusieurs boîtes (avant le goto), on ne garde que la dernière modif.
        for(QList<QString>::iterator it2 = boxAddresses.begin() ; it2!=boxAddresses.end() ; it2++){
            if(msgs.contains(*it2)){                
                if(msgs.value(*it2).second < boxMsgs.value(*it2).second && boxMsgs.value(*it2).second <= gotoValue){
                    msgs.insert(*it2,boxMsgs.value(*it2));                    
                }
            }
            //sinon on ajoute dans la liste de messages
            else
                if(boxMsgs.value(*it2).second <= gotoValue){
                    msgs.insert(*it2,boxMsgs.value(*it2));
                }
        }

        //On mute tous les messages avant le goto (Bug du moteur, qui envoyait des valeurs non désirées)
        //    Start messages
        if(currentBox->date() < gotoValue){
            _engines->setCtrlPointMutingState(boxID,1,true);
        }
        //    End messages
        if(currentBox->date()+currentBox->duration()<gotoValue){            
            _engines->setCtrlPointMutingState(boxID,2,true);
        }
    }

    //traduction en QMap<QString,QString>, on supprime le champs date des messages
    QList<QString> addresses = msgs.keys();

    QString message;
    for(QList<QString>::iterator it = addresses.begin() ; it != addresses.end() ; it++){         
         message = *it+" "+msgs.value(*it).first;
         sendMessage(message.toStdString());
    }
}

void
Maquette::pause(){
    _engines->pause(true);
}

void
Maquette::startPlaying(){

    _engines->pause(false);
    double gotoValue = (double)_engines->getGotoValue();
    initSceneState();
    generateTriggerQueue();
    int nbTrg = _scene->triggersQueueList().size();

    try{
        for(int i=0 ; i<nbTrg ; i++){
            if( gotoValue >= _scene->triggersQueueList().first()->date() ){
                _scene->triggersQueueList().removeFirst();
            }
            else
                break;
        }
    }
    catch (const std::exception & e){
       std::cerr << e.what();
   }

    for (BoxesMap::iterator it = _boxes.begin() ; it != _boxes.end() ; it++) {
        it->second->lock();
        if (it->second->type() == SOUND_BOX_TYPE) {
            if (it->second->date() >= _engines->getGotoValue()) {
                sendMessage(static_cast<SoundBox*>(it->second)->getPalette().toString());
            }
        }
    }
    _engines->play();
}

void
Maquette::stopPlayingGotoStart()
{
    for (BoxesMap::iterator it = _boxes.begin() ; it != _boxes.end() ; it++) {
        it->second->unlock();
    }

    _engines->stop();

    BoxesMap::iterator it;
    for (it = _boxes.begin() ; it != _boxes.end() ; it++) {
        int type = it->second->type();
        if (type == SOUND_BOX_TYPE || type == CONTROL_BOX_TYPE || type == PARENT_BOX_TYPE) {
            static_cast<BasicBox*>(it->second)->setCrossedExtremity(BOX_END);
        }
    }
    _scene->triggersQueueList().clear();

    setGotoValue(0);
}

void
Maquette::stopPlaying(){
    for (BoxesMap::iterator it = _boxes.begin() ; it != _boxes.end() ; it++) {
        it->second->unlock();
    }
    _engines->stop();

    BoxesMap::iterator it;
    for (it = _boxes.begin() ; it != _boxes.end() ; it++) {
        int type = it->second->type();
        if (type == SOUND_BOX_TYPE || type == CONTROL_BOX_TYPE || type == PARENT_BOX_TYPE) {
            static_cast<BasicBox*>(it->second)->setCrossedExtremity(BOX_END);
        }
    }
    _scene->triggersQueueList().clear();
}

void
Maquette::stopPlayingWithGoto()
{
    unsigned int gotoValue = _scene->getCurrentTime();
    for (BoxesMap::iterator it = _boxes.begin() ; it != _boxes.end() ; it++) {
        it->second->unlock();
    }
    _engines->stop();

    BoxesMap::iterator it;
    for (it = _boxes.begin() ; it != _boxes.end() ; it++) {
        int type = it->second->type();
        if (type == SOUND_BOX_TYPE || type == CONTROL_BOX_TYPE || type == PARENT_BOX_TYPE) {
            static_cast<BasicBox*>(it->second)->setCrossedExtremity(BOX_END);
        }
    }
    _scene->triggersQueueList().clear();
    setGotoValue(gotoValue);
}

void
Maquette::saveBox(unsigned int boxID)
{
    // TODO : handle others boxes further attributes during save
    QString boxType = "unknown";
    if (_boxes[boxID]->type() == SOUND_BOX_TYPE) {
        boxType = QString("sound");
    }
    else if (_boxes[boxID]->type() == CONTROL_BOX_TYPE) {
        boxType = QString("control");
    }
    else if (_boxes[boxID]->type() == PARENT_BOX_TYPE) {
        boxType = QString("parent");
    }
    else {
        boxType = QString("unknown");
    }

    /* Provided for convenience */
    BasicBox *box = _boxes[boxID];
    unsigned int date = box->date();
    unsigned int duration = box->duration();
    unsigned int topLeftY = box->getTopLeft().y();
    unsigned int sizeY = box->getSize().y();
    QColor color = box->color();
    QString name = box->name();
    int mother = box->mother();

    // root node
    QDomElement root = _doc->documentElement();
    QDomElement boxNode = _doc->createElement("box");
    root.firstChild().appendChild(boxNode);

    boxNode.setAttribute("type",boxType);
    boxNode.setAttribute("ID",boxID);
    boxNode.setAttribute("name",name);
    boxNode.setAttribute("mother",mother);

    QDomElement positionNode = _doc->createElement("position");

    QDomElement dateNode = _doc->createElement("date");
    dateNode.setAttribute("begin",date);
    dateNode.setAttribute("duration",duration);

    QDomElement topLeftNode = _doc->createElement("top-left");
    topLeftNode.setAttribute("y",topLeftY);

    QDomElement sizeNode = _doc->createElement("size");
    sizeNode.setAttribute("y",sizeY);

    positionNode.appendChild(dateNode);
    positionNode.appendChild(topLeftNode);
    positionNode.appendChild(sizeNode);

    boxNode.appendChild(positionNode);

    QDomElement colorNode = _doc->createElement("color");
    colorNode.setAttribute("red",color.red());
    colorNode.setAttribute("green",color.green());
    colorNode.setAttribute("blue",color.blue());

    boxNode.appendChild(colorNode);

    colorNode.setAttribute("red",color.red());
    colorNode.setAttribute("green",color.green());
    colorNode.setAttribute("blue",color.blue());

    if (boxType == "sound") {
        Palette pal = static_cast<SoundBox*>(box)->getPalette();
        QString soundFile = static_cast<SoundBox*>(box)->soundSelected();
        QDomElement attributesNode = _doc->createElement("attributes");
        QDomElement dynamicNode = _doc->createElement("dynamic");
        dynamicNode.setAttribute("shape",pal.shape());
        attributesNode.appendChild(dynamicNode);
        QDomElement rythmNode = _doc->createElement("rythm");
        rythmNode.setAttribute("speed",pal.speed());
        rythmNode.setAttribute("speed-variation",pal.speedVariation());
        rythmNode.setAttribute("grain",pal.grain());
        attributesNode.appendChild(rythmNode);
        QDomElement melodicNode = _doc->createElement("melodic");
        melodicNode.setAttribute("pitch-start",pal.pitchStart());
        melodicNode.setAttribute("pitch-random",pal.pitchRandom());
        melodicNode.setAttribute("pitch-vibrato",pal.pitchVibrato());
        melodicNode.setAttribute("pitch-end",pal.pitchEnd());
        melodicNode.setAttribute("pitch-amplitude",pal.pitchAmplitude());
        melodicNode.setAttribute("pitch-grade",pal.pitchGrade());
        attributesNode.appendChild(melodicNode);
        QDomElement harmonicNode = _doc->createElement("harmonic");
        harmonicNode.setAttribute("harmony",pal.harmo());
        harmonicNode.setAttribute("harmony-variation",pal.harmoVariation());
        attributesNode.appendChild(harmonicNode);
        QDomElement impulsiveNode = _doc->createElement("impulsive");
        impulsiveNode.setAttribute("state",pal.impulsive());
        attributesNode.appendChild(impulsiveNode);
        QDomElement soundNode = _doc->createElement("sound");
        soundNode.setAttribute("mode",pal.playingMode());
        soundNode.setAttribute("file",soundFile);
        attributesNode.appendChild(soundNode);
        boxNode.appendChild(attributesNode);
    }

    //return true;
}

std::string
Maquette::getNetworkHost(){
    return _devices.at(_currentDevice).networkHost;
}

void
Maquette::save(const string &fileName) {
    _engines->store(fileName+".simone");    

    QFile file(QString::fromStdString(fileName));

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        _scene->displayMessage(((QString("Cannot write file %1:\n%2.")).arg(QString::fromStdString(fileName)).
                arg(file.errorString())).toStdString(),WARNING_LEVEL);
        //return false;
    }

    QDomImplementation impl = QDomDocument().implementation();

    QString publicId = "i-score 2012";
    QString systemId = "http://scrime.labri.fr";
    QString typeId = "i-scoreSave";
    _doc = new QDomDocument(impl.createDocumentType(typeId,publicId,systemId));

    QDomElement root = _doc->createElement("GRAPHICS");
    root.setAttribute("zoom",_scene->zoom());
    root.setAttribute("centerX",_scene->view()->getCenterCoordinates().x());
    root.setAttribute("centerY",_scene->view()->getCenterCoordinates().y());
    _doc->appendChild(root);

    QDomElement boxesNode = _doc->createElement("boxes");
    root.appendChild(boxesNode);

    for(BoxesMap::iterator it = _boxes.begin() ; it != _boxes.end() ; ++it ) {
        saveBox(it->first);
    }

    //****************************  Devices ****************************
    QDomElement devicesNode = _doc->createElement("Devices");
    QDomElement deviceNode;

    std::map<std::string,MyDevice>::iterator it;
    string deviceName;
    unsigned int networkPort;
    string networkHost;
    string plugin;
    MyDevice curDevice;

    for( it = _devices.begin(); it!=_devices.end(); it++){
        curDevice = it->second;

        deviceName = curDevice.name;
        networkPort = curDevice.networkPort;
        networkHost = curDevice.networkHost;
        plugin = curDevice.plugin;

        deviceNode = _doc->createElement("Device");
        deviceNode.setAttribute("IP",QString::fromStdString(networkHost));
        deviceNode.setAttribute("port",networkPort);
        deviceNode.setAttribute("plugin",QString::fromStdString(plugin));
        deviceNode.setAttribute("name",QString::fromStdString(deviceName));

        devicesNode.appendChild(deviceNode);
    }

    root.appendChild(devicesNode);

    //OSC Messages
    QList<QString> OSCMessages = _scene->editor()->networkTree()->getOSCMessages();
    QDomElement OSCMessagesNode = _doc->createElement("OSCMessages");
    QDomElement OSCMessageNode;

    for(QList<QString>::iterator it=OSCMessages.begin() ; it!=OSCMessages.end() ; it++){
        OSCMessageNode = _doc->createElement("OSC");
        OSCMessageNode.setAttribute("message",*it);
        OSCMessagesNode.appendChild(OSCMessageNode);
    }
    root.appendChild(OSCMessagesNode);

    //***************************************************************


    QTextStream ts(&file);
    ts << _doc->toString();
    file.close();

    delete _doc;
}

void
Maquette::loadOLD(const string &fileName){
    _engines->load(fileName + ".simone");
    _engines->addCrossingCtrlPointCallback(&crossTransitionCallback);
    _engines->addExecutionFinishedCallback(&executionFinishedCallback);

    QFile enginesFile(QString::fromStdString(fileName + ".simone"));
    QFile file(QString::fromStdString(fileName));

    _doc = new QDomDocument;

    if (enginesFile.open(QFile::ReadOnly)) {
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            _scene->displayMessage((tr("Cannot read file %1:\n%2.")
                    .arg(QString::fromStdString(fileName))
                    .arg(file.errorString())).toStdString(),
                    WARNING_LEVEL);
            return;
        }
        else if (!_doc->setContent(&file)) {
            _scene->displayMessage((tr("Cannot import xml document from %1:\n%2.")
                    .arg(QString::fromStdString(fileName))
                    .arg(file.errorString())).toStdString(),
                    WARNING_LEVEL);
            file.close();
            return;
        }
    }
    else {
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            _scene->displayMessage((tr("Cannot read neither file %1 or %2 :\n%3.")
                    .arg(QString::fromStdString(fileName))
                    .arg(QString::fromStdString(fileName+".simone"))
                    .arg(file.errorString())).toStdString(),
                    WARNING_LEVEL);
        }
        else {
            _scene->displayMessage((tr("Cannot read file %1 :\n%2.")
                    .arg(QString::fromStdString(fileName+".simone"))
                    .arg(file.errorString())).toStdString(),
                    WARNING_LEVEL);
        }
    }

    QDomElement root = _doc->documentElement();
    if (root.tagName() != "GRAPHICS") {
        _scene->displayMessage((tr("Unvailable xml document %1").
                arg(QString::fromStdString(fileName))).toStdString(),
                WARNING_LEVEL);
        file.close();
        return;
    }

    _scene->clear();

    QPointF topLeft,size,bottomRight;    
    QString name,boxType,relType;
    QColor color(1.,1.,1.);
    Palette pal;
    int boxID,motherID;
    QMap<int,unsigned int> hashMap;
    QDomNode mainNode = root.firstChild(); // Boxes
    while (!mainNode.isNull()) {
        QDomNode node1 = mainNode.firstChild(); // Box
        while (!node1.isNull()) {
            QDomElement elt1 = node1.toElement();
            if (!elt1.isNull()) {
                if (elt1.tagName() == "box") {
                    boxID = elt1.attribute("ID",QString("%1").arg(NO_ID)).toInt();
                    boxType = elt1.attribute("type","unknown");
                    name = elt1.attribute("name","unknown");
                    motherID = elt1.attribute("mother",QString("%1").arg(ROOT_BOX_ID)).toInt();
                }
                QDomNode node2 = node1.firstChild();
                while (!node2.isNull()) {
                    QDomElement elt2 = node2.toElement();
                    if (!elt2.isNull()) {
                        if (elt2.tagName() == "color") {
                            color = QColor(elt2.attribute("red","1").toFloat() * 255,elt2.attribute("green","1").toFloat() * 255,
                                    elt2.attribute("blue","1").toFloat() * 255);
                            pal.setColor(color);
                        }
                        QDomNode node3 = node2.firstChild();
                        while (!node3.isNull()) {
                            QDomElement elt3 = node3.toElement();
                            if (elt2.tagName() == "position") {
                                QPointF tmp(elt3.attribute("x","0").toInt(),elt3.attribute("y","0").toInt());
                                if (elt3.tagName() == "top-left") {
                                    topLeft = tmp;
                                }
                                else if (elt3.tagName() == "size") {
                                    size = tmp;
                                }
                            }
                            else if (elt2.tagName() == "attributes") {
                                if (elt3.tagName() == "dynamic")
                                    pal.setShape(Shape(elt3.attribute("shape","0").toInt()));
                                else if (elt3.tagName() == "rythm") {
                                    pal.setSpeed(Speed(elt3.attribute("speed","0").toInt()));
                                    pal.setSpeedVariation(SpeedVariation(elt3.attribute("speed-variation","0").toInt()));
                                    pal.setGrain(Grain(elt3.attribute("grain","0").toInt()));
                                }
                                else if (elt3.tagName() == "melodic") {
                                    pal.setPitchStart(Pitch(elt3.attribute("pitch-start","0").toInt()));
                                    pal.setPitchRandom(elt3.attribute("pitch-random","0").toInt() == 1);
                                    pal.setPitchVibrato(elt3.attribute("pitch-vibrato","0").toInt() == 1);
                                    pal.setPitchEnd(Pitch(elt3.attribute("pitch-end","0").toInt()));
                                    pal.setPitchAmplitude(PitchVariation(elt3.attribute("pitch-amplitude","0").toInt()));
                                    pal.setPitchGrade(PitchVariation(elt3.attribute("pitch-grade","0").toInt()));
                                }
                                else if (elt3.tagName() == "harmonic") {
                                    pal.setHarmo(Harmo(elt3.attribute("harmony","0").toInt()));
                                    pal.setHarmoVariation(HarmoVariation(elt3.attribute("harmony-variation","0").toInt()));
                                }
                                else if (elt3.tagName() == "impulsive") {
                                    pal.setImpulsive(elt3.attribute("state","0").toInt() == 1);
                                }
                                else if (elt3.tagName() == "sound") {
                                    pal.setPlayingMode((PlayingMode)(elt3.attribute("mode","0").toInt()));
                                    QString file = elt3.attribute("file","");
                                    if (pal.playingMode() == FileMode) {
                                        if (QFile(file).exists()) {
                                            pal.setSoundFile(file);
                                        }
                                        else {
                                            pal.setSoundFile("");
                                            _scene->displayMessage((QString("Unvalid sound directory \"%1\" in box \"%2\"").
                                                    arg(file).arg(name)).toStdString(),WARNING_LEVEL);
                                        }
                                    }
                                }
                                // TODO : handle other boxes' attributes
                            }
                            node3 = node3.nextSibling();
                        }
                    }
                    node2 = node2.nextSibling();
                }
            }
            if (!elt1.isNull()) {
                if (elt1.tagName() == "box") {
                    if (boxType != "unknown") {
                        bottomRight = topLeft + size;
                        unsigned int ID = NO_ID;
                        if (boxType == "sound") {
                            pal.setColor(color);
                            ID = addSoundBox((unsigned int)boxID,topLeft,bottomRight,name.toStdString(),pal,motherID);
                            _scene->addSoundBox(ID);
                        }
                        else if (boxType == "control") {
                            ID = addControlBox((unsigned int)boxID,topLeft,bottomRight,name.toStdString(),motherID);
                            _scene->addControlBox(ID);
                            _scene->getBox(ID)->updateWidgets();
                            //OPEN FILE
                        }
                        else if (boxType == "parent") {
                            ID = addParentBox((unsigned int)boxID,topLeft,bottomRight,name.toStdString(),motherID);
                            _scene->addParentBox(ID);
                        }
                        else {
                            _scene->displayMessage((QString("Unavailable box type through loading %1")
                                    .arg(QString::fromStdString(fileName))).toStdString(),
                                    WARNING_LEVEL);
                        }
                    }
                }
            }
            node1 = node1.nextSibling(); // next Box
        }
        mainNode = mainNode.nextSibling();
    }

    vector<unsigned int> boxesID;
    _engines->getBoxesId(boxesID);
    vector<unsigned int>::iterator it;

    /************************ TRIGGER ************************/
    vector<unsigned int> triggersID;
    _engines->getTriggersPointId(triggersID);

    for (it = triggersID.begin() ; it != triggersID.end() ; it++){
        AbstractTriggerPoint abstractTrgPnt;
        abstractTrgPnt.setID(*it);
        unsigned int tpBoxID = _engines->getTriggerPointRelatedBoxId(*it);
        if (tpBoxID != NO_ID) {
            BasicBox *tpBox = getBox(tpBoxID);
            if (tpBox != NULL) {
                abstractTrgPnt.setBoxID(tpBoxID);
                switch(_engines->getTriggerPointRelatedCtrlPointIndex(*it)) {
                case BEGIN_CONTROL_POINT_INDEX :
                    abstractTrgPnt.setBoxExtremity(BOX_START);
                    break;
                case END_CONTROL_POINT_INDEX :
                    abstractTrgPnt.setBoxExtremity(BOX_END);
                    break;
                default :
                    std::cerr << "Maquette::load : unrecognized box extremity for Trigger Point " << *it << std::endl;
                    abstractTrgPnt.setBoxExtremity(BOX_START);
                    break;
                }
                std::string tpMsg = _engines->getTriggerPointMessage(*it);
                if (tpMsg != "") {
                    abstractTrgPnt.setMessage(tpMsg);
                }
                else {
#ifdef DEBUG
                    std::cerr << "Maquette::load : empty message found for Trigger Point " << *it << std::endl;
#endif
                }
                addTriggerPoint(abstractTrgPnt);
            }
            else {
#ifdef DEBUG
                std::cerr << "Maquette::load : NULL box found for trigger point " << *it << std::endl;
#endif
            }
        }
        else {
#ifdef DEBUG
            std::cerr << "Maquette::load : box with NO_ID found for trigger point " << *it << std::endl;
#endif
        }
    }

    /************************ RELATIONS ************************/
    vector<unsigned int> relationsID;
    _engines->getRelationsId(relationsID);


    for (it = relationsID.begin() ; it != relationsID.end() ; it++) {
        AbstractRelation abstractRel;
        unsigned int firstBoxID = _engines->getRelationFirstBoxId(*it);
        unsigned int secondBoxID = _engines->getRelationSecondBoxId(*it);
        if (firstBoxID != NO_ID && firstBoxID != ROOT_BOX_ID && secondBoxID != NO_ID
                && secondBoxID != ROOT_BOX_ID && firstBoxID != secondBoxID) {
            abstractRel.setFirstBox(firstBoxID);
            abstractRel.setSecondBox(secondBoxID);
            switch (_engines->getRelationFirstCtrlPointIndex(*it)) {
            case BEGIN_CONTROL_POINT_INDEX :
                abstractRel.setFirstExtremity(BOX_START);
                break;
            case END_CONTROL_POINT_INDEX :
                abstractRel.setFirstExtremity(BOX_END);
                break;
            }
            switch (_engines->getRelationSecondCtrlPointIndex(*it)) {
            case BEGIN_CONTROL_POINT_INDEX :
                abstractRel.setSecondExtremity(BOX_START);
                break;
            case END_CONTROL_POINT_INDEX :
                abstractRel.setSecondExtremity(BOX_END);
                break;
            }
            int minBoundMS = _engines->getRelationMinBound(*it);
            float minBoundPXL = NO_BOUND;
            if (minBoundMS != NO_BOUND) {
                minBoundPXL = (float)minBoundMS/MaquetteScene::MS_PER_PIXEL;
            }
            int maxBoundMS = _engines->getRelationMaxBound(*it);
            float maxBoundPXL = NO_BOUND;
            if (maxBoundMS != NO_BOUND) {
                maxBoundPXL = (float)maxBoundMS/MaquetteScene::MS_PER_PIXEL;
            }
            abstractRel.setMinBound(minBoundPXL);
            abstractRel.setMaxBound(maxBoundPXL);
            abstractRel.setID(*it);
            addRelation(abstractRel);
        }
    }


    delete _doc;
}

void
Maquette::load(const string &fileName){
    _engines->load(fileName + ".simone");
    _engines->addCrossingCtrlPointCallback(&crossTransitionCallback);
    _engines->addExecutionFinishedCallback(&executionFinishedCallback);

    QFile enginesFile(QString::fromStdString(fileName + ".simone"));
    QFile file(QString::fromStdString(fileName));

    _doc = new QDomDocument;

    if (enginesFile.open(QFile::ReadOnly)) {
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            _scene->displayMessage((tr("Cannot read file %1:\n%2.")
                    .arg(QString::fromStdString(fileName))
                    .arg(file.errorString())).toStdString(),
                    WARNING_LEVEL);
            return;
        }
        else if (!_doc->setContent(&file)) {
            _scene->displayMessage((tr("Cannot import xml document from %1:\n%2.")
                    .arg(QString::fromStdString(fileName))
                    .arg(file.errorString())).toStdString(),
                    WARNING_LEVEL);
            file.close();
            return;
        }
    }
    else {
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            _scene->displayMessage((tr("Cannot read neither file %1 or %2 :\n%3.")
                    .arg(QString::fromStdString(fileName))
                    .arg(QString::fromStdString(fileName+".simone"))
                    .arg(file.errorString())).toStdString(),
                    WARNING_LEVEL);
        }
        else {
            _scene->displayMessage((tr("Cannot read file %1 :\n%2.")
                    .arg(QString::fromStdString(fileName+".simone"))
                    .arg(file.errorString())).toStdString(),
                    WARNING_LEVEL);
        }
    }

    if(_doc->doctype().nodeName()!="i-scoreSave"){
        loadOLD(fileName);
        return;
    }

    QDomElement root = _doc->documentElement();

    if (root.tagName() != "GRAPHICS") {
        _scene->displayMessage((tr("Unvailable xml document %1").
                arg(QString::fromStdString(fileName))).toStdString(),
                WARNING_LEVEL);
        file.close();
        return;
    }

    _scene->clear();

    QPointF topLeft,size,bottomRight;
    unsigned int begin,duration,topLeftY,sizeY;
    QString name,boxType;
    QColor color(1.,1.,1.);
    Palette pal;
    int boxID,motherID;
    float zoom;
    QPointF centerCoordinates;

    zoom = root.attribute("zoom","1").toFloat();
    centerCoordinates = QPointF(root.attribute("centerX","0.").toFloat(),root.attribute("centerY","0.").toFloat());

    _scene->view()->setZoom(zoom);
    _scene->view()->centerOn(centerCoordinates);

    QDomNode mainNode = root.firstChild(); // Boxes
    while (!mainNode.isNull()) {
        QDomNode node1 = mainNode.firstChild(); // Box
        while (!node1.isNull()) {
            QDomElement elt1 = node1.toElement();
            if (!elt1.isNull()) {
                if (elt1.tagName() == "box") {
                    boxID = elt1.attribute("ID",QString("%1").arg(NO_ID)).toInt();
                    boxType = elt1.attribute("type","unknown");
                    name = elt1.attribute("name","unknown");
                    motherID = elt1.attribute("mother",QString("%1").arg(ROOT_BOX_ID)).toInt();
                }
                QDomNode node2 = node1.firstChild();
                while (!node2.isNull()) {
                    QDomElement elt2 = node2.toElement();
                    if (!elt2.isNull()) {
                        if (elt2.tagName() == "color") {
                            color = QColor(elt2.attribute("red","1").toInt(),elt2.attribute("green","1").toInt(),
                                    elt2.attribute("blue","1").toInt());
                            pal.setColor(color);
                        }
                        QDomNode node3 = node2.firstChild();
                        while (!node3.isNull()) {
                            QDomElement elt3 = node3.toElement();
                            if (elt2.tagName() == "position") {
                                if (elt3.tagName() == "date") {
                                    begin = elt3.attribute("begin","0").toInt();
                                    duration = elt3.attribute("duration","0").toInt();
                                }
                                else if (elt3.tagName() == "top-left") {
                                    topLeftY = sizeY = elt3.attribute("y","0").toInt();
                                }
                                else if (elt3.tagName() == "size") {
                                    sizeY = elt3.attribute("y","0").toInt();
                                }
                            }
                            else if (elt2.tagName() == "attributes") {
                                if (elt3.tagName() == "dynamic")
                                    pal.setShape(Shape(elt3.attribute("shape","0").toInt()));
                                else if (elt3.tagName() == "rythm") {
                                    pal.setSpeed(Speed(elt3.attribute("speed","0").toInt()));
                                    pal.setSpeedVariation(SpeedVariation(elt3.attribute("speed-variation","0").toInt()));
                                    pal.setGrain(Grain(elt3.attribute("grain","0").toInt()));
                                }
                                else if (elt3.tagName() == "melodic") {
                                    pal.setPitchStart(Pitch(elt3.attribute("pitch-start","0").toInt()));
                                    pal.setPitchRandom(elt3.attribute("pitch-random","0").toInt() == 1);
                                    pal.setPitchVibrato(elt3.attribute("pitch-vibrato","0").toInt() == 1);
                                    pal.setPitchEnd(Pitch(elt3.attribute("pitch-end","0").toInt()));
                                    pal.setPitchAmplitude(PitchVariation(elt3.attribute("pitch-amplitude","0").toInt()));
                                    pal.setPitchGrade(PitchVariation(elt3.attribute("pitch-grade","0").toInt()));
                                }
                                else if (elt3.tagName() == "harmonic") {
                                    pal.setHarmo(Harmo(elt3.attribute("harmony","0").toInt()));
                                    pal.setHarmoVariation(HarmoVariation(elt3.attribute("harmony-variation","0").toInt()));
                                }
                                else if (elt3.tagName() == "impulsive") {
                                    pal.setImpulsive(elt3.attribute("state","0").toInt() == 1);
                                }
                                else if (elt3.tagName() == "sound") {
                                    pal.setPlayingMode((PlayingMode)(elt3.attribute("mode","0").toInt()));
                                    QString file = elt3.attribute("file","");
                                    if (pal.playingMode() == FileMode) {
                                        if (QFile(file).exists()) {
                                            pal.setSoundFile(file);
                                        }
                                        else {
                                            pal.setSoundFile("");
                                            _scene->displayMessage((QString("Unvalid sound directory \"%1\" in box \"%2\"").
                                                    arg(file).arg(name)).toStdString(),WARNING_LEVEL);
                                        }
                                    }
                                }
                                // TODO : handle other boxes' attributes
                            }
                            node3 = node3.nextSibling();
                        }
                    }
                    node2 = node2.nextSibling();
                }
            }
            if (!elt1.isNull()) {
                if (elt1.tagName() == "box") {
                    if (boxType != "unknown") {
                        bottomRight = topLeft + size;
                        unsigned int ID = NO_ID;
                        if (boxType == "sound") {
                            pal.setColor(color);
                            ID = addSoundBox((unsigned int)boxID,topLeft,bottomRight,name.toStdString(),pal,motherID);
                            _scene->addSoundBox(ID);
                        }
                        else if (boxType == "control") {
                            ID = addControlBox((unsigned int)boxID,topLeft,bottomRight,name.toStdString(),motherID);
                            _scene->addControlBox(ID);
                            _scene->getBox(ID)->updateWidgets();
                            //OPEN FILE
                        }
                        else if (boxType == "parent") {
//                            ID = addParentBox((unsigned int)boxID,topLeft,bottomRight,name.toStdString(),motherID);
                            ID = addParentBox((unsigned int)boxID,(unsigned int)begin,(unsigned int)topLeftY,(unsigned int)sizeY,(unsigned int)duration,name.toStdString(),motherID,color);
                            _scene->addParentBox(ID);
                        }
                        else {
                            _scene->displayMessage((QString("Unavailable box type through loading %1")
                                    .arg(QString::fromStdString(fileName))).toStdString(),
                                    WARNING_LEVEL);
                        }
                    }
                }
            }
            node1 = node1.nextSibling(); // next Box
        }
        mainNode = mainNode.nextSibling();
    }

    vector<unsigned int> boxesID;
    _engines->getBoxesId(boxesID);
    vector<unsigned int>::iterator it;

    /************************ TRIGGER ************************/
    vector<unsigned int> triggersID;
    _engines->getTriggersPointId(triggersID);

    for (it = triggersID.begin() ; it != triggersID.end() ; it++){
        AbstractTriggerPoint abstractTrgPnt;
        abstractTrgPnt.setID(*it);
        unsigned int tpBoxID = _engines->getTriggerPointRelatedBoxId(*it);
        if (tpBoxID != NO_ID) {
            BasicBox *tpBox = getBox(tpBoxID);
            if (tpBox != NULL) {
                abstractTrgPnt.setBoxID(tpBoxID);
                switch(_engines->getTriggerPointRelatedCtrlPointIndex(*it)) {
                case BEGIN_CONTROL_POINT_INDEX :
                    abstractTrgPnt.setBoxExtremity(BOX_START);
                    break;
                case END_CONTROL_POINT_INDEX :
                    abstractTrgPnt.setBoxExtremity(BOX_END);
                    break;
                default :
                    std::cerr << "Maquette::load : unrecognized box extremity for Trigger Point " << *it << std::endl;
                    abstractTrgPnt.setBoxExtremity(BOX_START);
                    break;
                }
                std::string tpMsg = _engines->getTriggerPointMessage(*it);
                if (tpMsg != "") {
                    abstractTrgPnt.setMessage(tpMsg);
                }
                else {
#ifdef DEBUG
                    std::cerr << "Maquette::load : empty message found for Trigger Point " << *it << std::endl;
#endif
                }
                addTriggerPoint(abstractTrgPnt);
            }
            else {
#ifdef DEBUG
                std::cerr << "Maquette::load : NULL box found for trigger point " << *it << std::endl;
#endif
            }
        }
        else {
#ifdef DEBUG
            std::cerr << "Maquette::load : box with NO_ID found for trigger point " << *it << std::endl;
#endif
        }
    }

    /************************ RELATIONS ************************/
    vector<unsigned int> relationsID;
    _engines->getRelationsId(relationsID);


    for (it = relationsID.begin() ; it != relationsID.end() ; it++) {
        AbstractRelation abstractRel;
        unsigned int firstBoxID = _engines->getRelationFirstBoxId(*it);
        unsigned int secondBoxID = _engines->getRelationSecondBoxId(*it);
        if (firstBoxID != NO_ID && firstBoxID != ROOT_BOX_ID && secondBoxID != NO_ID
                && secondBoxID != ROOT_BOX_ID && firstBoxID != secondBoxID) {
            abstractRel.setFirstBox(firstBoxID);
            abstractRel.setSecondBox(secondBoxID);
            switch (_engines->getRelationFirstCtrlPointIndex(*it)) {
            case BEGIN_CONTROL_POINT_INDEX :
                abstractRel.setFirstExtremity(BOX_START);
                break;
            case END_CONTROL_POINT_INDEX :
                abstractRel.setFirstExtremity(BOX_END);
                break;
            }
            switch (_engines->getRelationSecondCtrlPointIndex(*it)) {
            case BEGIN_CONTROL_POINT_INDEX :
                abstractRel.setSecondExtremity(BOX_START);
                break;
            case END_CONTROL_POINT_INDEX :
                abstractRel.setSecondExtremity(BOX_END);
                break;
            }
            int minBoundMS = _engines->getRelationMinBound(*it);
            float minBoundPXL = NO_BOUND;
            if (minBoundMS != NO_BOUND) {
                minBoundPXL = (float)minBoundMS/(MaquetteScene::MS_PER_PIXEL*zoom);
            }
            int maxBoundMS = _engines->getRelationMaxBound(*it);
            float maxBoundPXL = NO_BOUND;
            if (maxBoundMS != NO_BOUND) {
                maxBoundPXL = (float)maxBoundMS/(MaquetteScene::MS_PER_PIXEL*zoom);
            }
            abstractRel.setMinBound(minBoundPXL);
            abstractRel.setMaxBound(maxBoundPXL);
            abstractRel.setID(*it);
            addRelation(abstractRel);
        }
    }


    /************************ Devices ************************/
    MyDevice OSCDevice;
    string OSCDevicePort;

    //clean
    vector<string> deviceNames;
    vector<bool> deviceRequestable;
    _engines->getNetworkDevicesName(deviceNames,deviceRequestable);
    for(unsigned int i=0; i<deviceNames.size(); i++)
        _engines->removeNetworkDevice(deviceNames[i]);
    _devices.clear();

    //read from xml
    if(root.childNodes().size()>=2){ //Devices
        QDomElement devices = root.childNodes().at(1).toElement();
        if (devices.tagName() != "Devices") {
            _scene->displayMessage((tr("Unvailable xml document %1 - Devices problem").
                    arg(QString::fromStdString(fileName))).toStdString(),
                    WARNING_LEVEL);
            file.close();
            return;
        }
        else{ //get device infos
            QString deviceName;
            QString ip;
            QString port;
            QString plugin;

            for(int i=0 ; i<devices.childNodes().size() ; i++){ //Device
                if(devices.childNodes().at(i).toElement().tagName()=="Device"){
                    deviceName = devices.childNodes().at(i).toElement().attribute("name");
                    ip = devices.childNodes().at(i).toElement().attribute("IP");
                    port = devices.childNodes().at(i).toElement().attribute("port");
                    plugin = devices.childNodes().at(i).toElement().attribute("plugin");

                    addNetworkDevice(deviceName.toStdString(),plugin.toStdString(),ip.toStdString(),port.toStdString());

                    //save OSC device (for OSCMessages)
                    if(plugin == "OSC"){
                        OSCDevice.name = deviceName.toStdString();
                        OSCDevice.networkHost = ip.toStdString();
                        OSCDevicePort = port.toStdString();
                        OSCDevice.plugin = plugin.toStdString();
                    }
                }
            }
        }
    }

    //reload networkTree
    _scene->editor()->networkTree()->load();


    /************************ OSC ************************/
    if(root.childNodes().size()>=2){
        QDomElement OSC = root.childNodes().at(2).toElement();

        if (OSC.tagName() != "OSCMessages") {
            _scene->displayMessage((tr("Unvailable xml document %1 - OSC Messages problem").
                    arg(QString::fromStdString(fileName))).toStdString(),
                    WARNING_LEVEL);
            file.close();
            return;
        }

        QList<QString> OSCMessagesList;
        for(int i=0 ; i<OSC.childNodes().size() ; i++){
            if(OSC.childNodes().at(i).toElement().tagName()=="OSC")
                OSCMessagesList<<OSC.childNodes().at(i).toElement().attribute("message");
        }

        _scene->setNetworDeviceConfig(OSCDevice.name,OSCDevice.plugin,OSCDevice.networkHost,OSCDevicePort);
        _scene->editor()->networkTree()->createItemsFromMessages(OSCMessagesList);
    }

    delete _doc;
}

void
Maquette::addNetworkDevice(string deviceName,string plugin,string ip,string port){
    std::istringstream iss(port);
    unsigned int portInt;
    iss >> portInt;

    MyDevice newDevice(deviceName,plugin,portInt,ip);
    _devices[deviceName]=newDevice;
    _engines->addNetworkDevice(deviceName,plugin,ip,port);
}

double
Maquette::accelerationFactor(){
    return _engines->getExecutionSpeedFactor();
}

void
Maquette::setAccelerationFactor(const float &factor) {
    _engines->setExecutionSpeedFactor(factor);
}

void
Maquette::crossedTransition(unsigned int boxID, unsigned int CPIndex)
{
	int type = getBox(boxID)->type();
	if (type == SOUND_BOX_TYPE || type == CONTROL_BOX_TYPE ||type == PARENT_BOX_TYPE) {
		if (CPIndex == BEGIN_CONTROL_POINT_INDEX) {
			static_cast<BasicBox*>(_boxes[boxID])->setCrossedExtremity(BOX_START);
		}
		else if (CPIndex == END_CONTROL_POINT_INDEX) {
			static_cast<BasicBox*>(_boxes[boxID])->setCrossedExtremity(BOX_END);
		}
		else {
			std::cerr << "Maquette::crossTransitionCallback : unrecognized control point index" << std::endl;
		}
	}
}

void
Maquette::crossedTriggerPoint(bool waiting, unsigned int trgID)
{
    TriggerPoint *trgPnt = getTriggerPoint(trgID);

     if (trgPnt != NULL) {
          if(waiting){
              trgPnt->setWaiting(waiting);
              if (_scene->triggersQueueList().isEmpty())
                  generateTriggerQueue();
                  _scene->setFocusItem(_scene->triggersQueueList().first(),Qt::OtherFocusReason);
              }

           else{
              trgPnt->setWaiting(waiting);
              _scene->triggersQueueList().removeAll(trgPnt);
              if(!_scene->triggersQueueList().isEmpty())
                 _scene->setFocusItem(_scene->triggersQueueList().first(),Qt::OtherFocusReason);
           }
      }
}

void
Maquette::executionFinished(){
	_scene->timeEndReached();
}

void
crossTransitionCallback(unsigned int boxID, unsigned int CPIndex, vector<unsigned int> processesToStop) {
	Maquette::getInstance()->crossedTransition(boxID,CPIndex);
	for (vector<unsigned int>::iterator it = processesToStop.begin() ; it != processesToStop.end() ; ++it) {
		Maquette::getInstance()->crossedTransition(*it,END_CONTROL_POINT_INDEX);
	}
}

void
enginesNetworkUpdateCallback(unsigned int boxID, string m1, string m2){
    Q_UNUSED(boxID);
    MaquetteScene *scene = Maquette::getInstance()->scene();

    if(scene != NULL){
        if(m1 == PLAY_ENGINES_MESSAGE){
            scene->play();
        }
        else if(m1 == STOP_ENGINES_MESSAGE){
            scene->stopWithGoto();
        }
        else if(m1 == STARTPOINT_ENGINES_MESSAGE){
            if(!m2.empty()){
                std::istringstream iss(m2);
                unsigned int value;
                iss >> value;
                scene->gotoChanged(value);
            }
        }
        else if(m1 == REWIND_ENGINES_MESSAGE){
            scene->stopGotoStart();
        }
        else if(m1 == SPEED_ENGINES_MESSAGE){
            if(!m2.empty()){
                std::istringstream iss(m2);
                double value;
                iss >> value;
                scene->speedChanged(value);
            }
        }
        scene->view()->emitPlayModeChanged();
    }
#ifdef DEBUG
    else
        std::cerr << "Maquette::enginesNetworkCallback : attribute _scene == NULL" << std::endl;
#endif
}

void
crossTriggerPointCallback(bool waiting, unsigned int trgID, unsigned int boxID, unsigned int CPIndex, string message) {
	Q_UNUSED(boxID);
	Q_UNUSED(CPIndex);
    Q_UNUSED(message);
    Maquette::getInstance()->crossedTriggerPoint(waiting,trgID);
}

void
executionFinishedCallback(){
	Maquette::getInstance()->executionFinished();
}

void
Maquette::setStartMessageToSend(unsigned int boxID,QTreeWidgetItem *item,QString address){
    _boxes[boxID]->setStartMessage(item,address);
}

void
Maquette::setEndMessageToSend(unsigned int boxID,QTreeWidgetItem *item,QString address){
    _boxes[boxID]->setEndMessage(item,address);
}
