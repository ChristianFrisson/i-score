#ifndef DEVICEEDIT_HPP
#define DEVICEEDIT_HPP

#include <QDialog>
#include <QMessageBox>
#include <QLabel>
#include <QGridLayout>
#include <map>
#include <string>
#include "Maquette.hpp"
#include <QSpinBox>
#include <QPushButton>
#include <QHostAddress>

class MaquetteScene;

/*!
 * \class DeviceEdit
 *
 * \brief Dialog for device edition or device creation.
 */

class DeviceEdit : public QDialog
{
  Q_OBJECT

  public:
    DeviceEdit(QWidget *parent);

  public slots:
    void edit(QString name);
    void edit();
    void setChanged();
    void setProtocolChanged();
    void setDeviceNameChanged();
    void setNetworkPortChanged();
    void setLocalHostChanged();
    void updateNetworkConfiguration();


  signals:
    void deviceNameChanged(QString,QString);
    void deviceProtocolChanged(QString);

  private:
    void init();

    bool _changed;
    bool _nameChanged;
    bool _protocolChanged;
    bool _localHostChanged;
    bool _networkPortChanged;

    QString defaultName = "MinuitDevice1";
    QString defaultLocalHost = "192.168.0.1";
    unsigned int defaultPort = 9998;
    int defaultProtocolIndex = 1;


    QString _currentDevice;

    QGridLayout *_layout;
    QLabel *_deviceNameLabel;
    QLineEdit *_nameEdit;

    QLabel *_devicesLabel;       //!< QLabel for devices.
    QLabel *_protocolsLabel;       //!< QLabel for protocols.
    QLabel *_portOutputLabel;          //!< QLabel for output ports.
    QLabel *_portInputLabel;          //!< QLabel for intput ports.
    QLabel *_localHostLabel;            //!< QLabel for network adress.

    QComboBox *_protocolsComboBox; //!< QComboBox handling various plug-ins available.
    QSpinBox *_portOutputBox;          //!< QSpinBox managing output port value.
//    QSpinBox *_portInputBox;          //!< QSpinBox managing input port value.
    QLineEdit *_localHostBox;           //!< QLineEdit

    QPushButton *_okButton;      //!< Button used to confirme.
    QPushButton *_cancelButton;  //!< Button used to cancel.
};
#endif // DEVICEEDIT_HPP
