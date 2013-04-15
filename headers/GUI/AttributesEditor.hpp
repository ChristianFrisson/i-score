/*
 * Copyright: LaBRI / SCRIME
 *
 * Authors: Luc Vercellin and Bruno Valeze (08/03/2010)
 *
 * luc.vercellin@labri.fr
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
#ifndef ATTRIBUTES_EDITOR
#define ATTRIBUTES_EDITOR

/*!
 * \file AttributesEditor.h
 * \author Luc Vercellin
 * \date 30 september 2009
 */

#include "EnumPalette.hpp"
#include <QDockWidget>

class QComboBox;
class QButtonGroup;
class QRadioButton;
class QGridLayout;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QCheckBox;
class QTabWidget;
class QScrollArea;
class QDoubleSpinBox;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class CurvesWidget;
class Interpolation;

class PreviewArea;
class Palette;
class NetworkMessagesEditor;
class AbstractBox;
class MaquetteScene;
class NetworkTree;
class TreeMap;
class NetworkMessages;

/*!
 * \class AttributesEditor
 *
 * \brief Class for palette toolbar, derived from Qt's QDockWidget.
 * See http://doc.trolltech.com/qdockwidget.html for more details.
 */
class AttributesEditor : public QDockWidget
{
  Q_OBJECT

  public:
    AttributesEditor(QWidget* parent);

    virtual
    ~AttributesEditor();

    void init();

    void setScene(MaquetteScene * scene);

    /*!
     * \brief Gets the palette used by the editor.
     *
     * \return the palette used by the editor
     **/
    Palette getPalette() const;

    /*!
     * \brief Selects an abstract box to be edited.
     *
     * \param abBox : the abstract box
     */
    void setAttributes(AbstractBox *abBox);

    /*!
     * \brief Gets current box edited in the editor.
     *
     * \return the box edited in the editor
     **/
    unsigned int currentBox();

  public slots:
    void addToExpandedItemsList(QTreeWidgetItem *item);
    void removeFromExpandedItemsList(QTreeWidgetItem *item);

    /*!
     * \brief Called to specify that no box is currently edited.
     */
    void noBoxEdited();

    /*!
     * \brief Updates values of widgets according to the editor current attributes
     **/
    void updateWidgets(bool boxModified);

    /*!
     * \brief Assigns the snapshot to the start of the box.
     */
    void snapshotStartAssignment();

    /*!
     * \brief Assigns the snapshot to the end of the box.
     */
    void snapshotEndAssignment();
    inline NetworkTree *
    networkTree(){ return _networkTree; }
    virtual void clear();

  protected:
    /*!
     * \brief Fills widgets with names in selected language.
     *
     * \param language : the language to used for filling widgets' names
     **/

    // TODO : use QLocale instead
    void nameWidgets();

    /*!
     * \brief Allocates widgets.
     **/
    void createWidgets();

    /*!
     * \brief Inserts widgets to main layout.
     **/
    void addWidgetsToLayout();

    /*!
     * \brief Associates slots with QWidgets' signals.
     **/
    void connectSlots();
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

  private slots:
    /*!
     * \brief Selecs a new color for PreviewArea and palette.
     **/
    void changeColor();
    void startMessageChanged(QTreeWidgetItem *item);
    void endMessageChanged(QTreeWidgetItem *item);
    void startMessageRemoved(const std::string &address);
    void endMessageRemoved(const std::string &address);
    void curveActivationChanged(QTreeWidgetItem *item, bool activated);
    void curveRedundancyChanged(QTreeWidgetItem *item, bool activated);
    void curveSampleRateChanged(QTreeWidgetItem *item, int value);
    void deployMessageChanged(QTreeWidgetItem *item, QString address);
    void deployDeviceChanged(QString oldName, QString newName);

    /*!
     * \brief Updates box start message
     */
    void startMessagesChanged(bool forceUpdate = false);

    /*!
     * \brief Updates box end message
     */
    void endMessagesChanged(bool forceUpdate = false);
    /*!
     * \brief Called when the start of the box is changed.
     */
    void startChanged();

    /*!
     * \brief Called when the length of the box is changed.
     */
    void lengthChanged();

    /*!
     * \brief Called when the name of the box is changed.
     */
    void nameChanged();

  private:
    QWidget * _centralWidget;   //!< Central widget.
    QGridLayout * _centralLayout; //!< Central layout

    QHBoxLayout *_boxSettingsLayout; //!< Layout handling box settings (name, color, assignation...).
    NetworkTree *_networkTree; //!< NetworkTree (inspector).

    Palette *_palette;          //!< Palette storing attributes
    PreviewArea *_generalPreviewArea;  //!< Editor's preview area

    // TODO
    QDoubleSpinBox * _boxStartValue;
    QDoubleSpinBox * _boxLengthValue;    

    // boxEditLayout items
    QLineEdit * _boxName; //!< Color button.
    QPixmap *_colorButtonPixmap; //!< Color button.
    QPushButton *_generalColorButton;  //!< Color button.
    QPushButton *_snapshotAssignStart; //!< Start assignation button.
    QPushButton *_snapshotAssignEnd; //!< End assignation button.

    unsigned int _boxEdited;    //!< ID of box being edited
    MaquetteScene * _scene; //!< The maquetteScene related with.
};
#endif
