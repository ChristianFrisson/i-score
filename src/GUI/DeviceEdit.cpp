/*
 * Copyright: LaBRI / SCRIME
 *
 * Authors: Nicolas Hincker (08/02/2013)
 *
 * This software is a computer program whose purpose is to provide
 * notation/composition combining synthesized as well as recorded
 * sounds, providing answers to the problem of notation and, drawing,
 * from its very design, on benefits from state of the art research
 * in musicology and sound/music computing.
 *
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */

#include "DeviceEdit.hpp"

DeviceEdit::DeviceEdit(QWidget *parent)
  : QDialog(parent)
{
  setModal(true);
  init();
}

void
DeviceEdit::init()
{
  _changed = false;
  _nameChanged = false;

  _layout = new QGridLayout(this);
  setLayout(_layout);

  _deviceNameLabel = new QLabel(tr("DeviceName"));
  _pluginsLabel = new QLabel(tr("Plugins"));
  _portOutputLabel = new QLabel(tr("Port (output)"));
  _portInputLabel = new QLabel(tr("Port (input)"));
  _IPLabel = new QLabel(tr("IP"));

  _portOutputBox = new QSpinBox;
  _portOutputBox->setRange(0, 10000);
  _portInputBox = new QSpinBox;
  _portInputBox->setRange(0, 10000);
  _portInputBox->setValue(7002);
  _portInputBox->setEnabled(false);

  _IPBox = new QLineEdit;
  _nameEdit = new QLineEdit;

  _pluginsComboBox = new QComboBox;
  _pluginsComboBox->setEnabled(false);

  _layout->addWidget(_deviceNameLabel, 0, 0, 1, 1);
  _layout->addWidget(_nameEdit, 0, 1, 1, 1);
  _layout->addWidget(_pluginsLabel, 1, 0, 1, 1);
  _layout->addWidget(_pluginsComboBox, 1, 1, 1, 1);
  _layout->addWidget(_portOutputLabel, 2, 0, 1, 1);
  _layout->addWidget(_portOutputBox, 2, 1, 1, 1);
  _layout->addWidget(_portInputLabel, 3, 0, 1, 1);
  _layout->addWidget(_portInputBox, 3, 1, 1, 1);
  _layout->addWidget(_IPLabel, 4, 0, 1, 1);
  _layout->addWidget(_IPBox, 4, 1, 1, 1);

  _okButton = new QPushButton(tr("OK"), this);
  _layout->addWidget(_okButton, 4, 2, 1, 1);
  _cancelButton = new QPushButton(tr("Cancel"), this);
  _layout->addWidget(_cancelButton, 4, 3, 1, 1);

  connect(_nameEdit, SIGNAL(editingFinished()), this, SLOT(deviceNameChanged()));
  connect(_pluginsComboBox, SIGNAL(activated(int)), this, SLOT(setPluginChanged()));
  connect(_portOutputBox, SIGNAL(valueChanged(int)), this, SLOT(setChanged()));
  connect(_IPBox, SIGNAL(textChanged(const QString &)), this, SLOT(setChanged()));

  connect(_okButton, SIGNAL(clicked()), this, SLOT(updateNetworkConfiguration()));
  connect(_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void
DeviceEdit::edit(QString name)
{
  std::map<string, MyDevice> devices = Maquette::getInstance()->getNetworkDevices();
  std::vector<std::string> plugins = Maquette::getInstance()->getPlugins();
  std::map<string, MyDevice>::iterator it;

  if ((it = devices.find(name.toStdString())) != devices.end()) {
      _currentDevice = it->second;
    }
  else {
      QMessageBox::warning(this, "", tr("Device not found"));
      return;
    }

  _portOutputBox->setValue(_currentDevice.networkPort);
  _IPBox->setText(QString::fromStdString(_currentDevice.networkHost));

  _nameEdit->setText(QString::fromStdString(_currentDevice.name));
  _nameEdit->selectAll();

  /***************************   Plugins *****************************/
  for (unsigned int i = 0; i < plugins.size(); i++) {
      if (_pluginsComboBox->findText(QString::fromStdString(plugins[i])) == -1) {
          _pluginsComboBox->addItem(QString::fromStdString(plugins[i]));
        }
    }
  if (_pluginsComboBox->findText(QString::fromStdString(_currentDevice.plugin)) == -1) {
      _pluginsComboBox->addItem(QString::fromStdString(_currentDevice.plugin));
    }

  int found = -1;
  if ((found = _pluginsComboBox->findText(QString::fromStdString(_currentDevice.plugin))) != -1) {
      _pluginsComboBox->setCurrentIndex(found);
    }
  else {
      QMessageBox::warning(this, "", QString::fromStdString(_currentDevice.plugin) + tr(" plugin not found : default selected"));
      _pluginsComboBox->setCurrentIndex(0);
    }

  /*******************************************************************/

  exec();
}

void
DeviceEdit::setPluginChanged()
{
  _pluginChanged = true;
  setChanged();
}

void
DeviceEdit::setChanged()
{
  _changed = true;
}

void
DeviceEdit::updateNetworkConfiguration()
{
  if (_changed) {
      QHostAddress hostAddress(_IPBox->text());
      if (!hostAddress.isNull()) {
          Maquette::getInstance()->changeNetworkDevice(_nameEdit->text().toStdString(), _pluginsComboBox->currentText().toStdString(),
                                                       _IPBox->text().toStdString(), _portOutputBox->text().toStdString());
          if (_pluginChanged) {
              emit(devicePluginChanged(_pluginsComboBox->currentText()));
            }
          if (_nameChanged) {
              emit(deviceNameChanged(_nameEdit->text(), _pluginsComboBox->currentText()));
            }

          accept();
        }
      else {
          QMessageBox::warning(this, "", tr("Unvalid IP address"));
        }
    }
  else {
      accept();
    }
  _changed = false;
  _nameChanged = false;
}

void
DeviceEdit::deviceNameChanged()
{
  _nameChanged = true;
  setChanged();
}
