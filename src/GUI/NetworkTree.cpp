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

#include "NetworkTree.hpp"
#include "Maquette.hpp"
#include <QList>
#include <map>
#include <exception>

NetworkTree::NetworkTree(QWidget *parent) : QTreeWidget(parent)
{    
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *,int)),this,SLOT(itemCollapsed()));
}
/*
void NetworkTree::load() {
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

	addTopLevelItems(itemsList);
}
*/

void NetworkTree::setSelectedItems(QList<QTreeWidgetItem*> selectedItems){

    resetSelectedItems();
    QList<QTreeWidgetItem *>::iterator it;
    QTreeWidgetItem *curItem;
    for (it =  selectedItems.begin() ; it !=  selectedItems.end() ; ++it){
        curItem = *it;
        curItem->setSelected(true);
    }
}

void NetworkTree::resetSelectedItems(){

    //QList<QTreeWidgetItem*> selection = assignedItems();
    QList<QTreeWidgetItem*> selection = selectedItems();
    QList<QTreeWidgetItem *>::iterator it;
    QTreeWidgetItem *curItem;

    for (it =  selection.begin() ; it !=  selection.end() ; ++it){
        curItem = *it;
        curItem->setSelected(false);
    }
}

QList<QTreeWidgetItem*> NetworkTree::getSelectedItems(){

    QList<QTreeWidgetItem*> items  = selectedItems(), allSelectedItems, *children = new QList<QTreeWidgetItem*>();
    QList<QTreeWidgetItem*>::iterator it;
    allSelectedItems << items;

    for(it=items.begin() ; it!=items.end() ; ++it){
        QTreeWidgetItem *curItem = *it;
        treeRecursiveSelection(curItem,children);
        if (!children->empty())
            allSelectedItems << *children;
    }
    return allSelectedItems;
}

void NetworkTree::treeRecursiveSelection(QTreeWidgetItem *curItem, QList<QTreeWidgetItem*> *itemsList){

    int i;
    QTreeWidgetItem *child;
    if (!curItem->isDisabled()) {
        int childrenCount = curItem->childCount();
        for (i=0; i < childrenCount ; i++) {
            child=curItem->child(i);
            if (child->type()==NodeNamespaceType){
                itemsList->push_back(child);
                treeRecursiveSelection(child,itemsList);
            }
            if (child->type()==LeaveType){
                itemsList->push_back(child);
            }
        }
    }

}

void NetworkTree::treeRecursiveExploration(QTreeWidgetItem *curItem){

    if (!curItem->isDisabled()) {

        vector<string> nodes,leaves,attributes,attributesValues;
        QString address = getAbsoluteAddress(curItem);
        Maquette::getInstance()->requestNetworkNamespace(address.toStdString(), nodes, leaves, attributes, attributesValues);
        vector<string>::iterator it;
        vector<string>::iterator it2;

        for (it = leaves.begin() ; it != leaves.end() ; ++it) {
            QStringList list;
            list << QString::fromStdString(*it);
            QTreeWidgetItem *childItem = new QTreeWidgetItem(list,LeaveType);
            childItem->setBackground(0,QBrush(Qt::darkGreen));
            curItem->addChild(childItem);
            list.clear();
            treeRecursiveExploration(childItem);
        }
        for (it = attributes.begin(),it2 = attributesValues.begin() ; it != attributes.end(), it2 != attributesValues.end() ; ++it , ++it2) {
            QStringList list;
            list << QString::fromStdString(*it + " : " + *it2);
            QTreeWidgetItem *childItem = new QTreeWidgetItem(list,AttributeType);
            childItem->setBackground(0,QBrush(Qt::green));
            curItem->addChild(childItem);
            list.clear();
        }
        for (it = nodes.begin() ; it != nodes.end() ; ++it) {
            QStringList list;
            list << QString::fromStdString(*it);
            QTreeWidgetItem *childItem = new QTreeWidgetItem(list,NodeNamespaceType);
            childItem->setBackground(0,QBrush(Qt::darkCyan));
            curItem->addChild(childItem);
            list.clear();
            treeRecursiveExploration(childItem);
        }
    }
}

void NetworkTree::load() {
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

            try{
                treeRecursiveExploration(curItem);
            }catch (const std::exception & e){
                std::cerr << *nameIt << " : " << e.what();
            }
        }
        itemsList << curItem;
    }
    addTopLevelItems(itemsList);
}

QString
NetworkTree::getAbsoluteAddress(QTreeWidgetItem *item) const {
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

vector<string> NetworkTree::snapshot() {
	QList<QTreeWidgetItem*> selection = selectedItems();  
	vector<string> snapshots;
	if (!selection.empty()) {
		QList<QTreeWidgetItem*>::iterator it;
		vector<string>::iterator it2;

		for (it = selection.begin(); it != selection.end() ; ++it) {
			QString address = getAbsoluteAddress(*it);
			if (!address.isEmpty()) {
				vector<string> snapshot = Maquette::getInstance()->requestNetworkSnapShot(address.toStdString());
				snapshots.insert(snapshots.end(),snapshot.begin(),snapshot.end());
			}
		}
	}

	return snapshots;
}

void
NetworkTree::itemCollapsed() {
	QTreeWidgetItem *selectedItem = currentItem();
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
		}
	}
}

