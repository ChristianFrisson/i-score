/*
Copyright: LaBRI / SCRIME

Authors: Luc Vercellin (22/06/2010)

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
#ifndef RELATION_EDIT_HPP
#define RELATION_EDIT_HPP

/*!
 * \file RelationEdit.hpp
 *
 * \author Luc Vercellin
 */

#include <QDialog>
#include <map>
#include <string>
#include "Maquette.hpp"

class QComboBox;
class QGridLayout;
class QPushButton;
class QDoubleSpinBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class MainWindow;
class MaquetteScene;

/*!
 * \class RelationEdit
 *
 * \brief Dialog for relation properties edition.
 */

class RelationEdit : public QDialog
{
  Q_OBJECT

    public :
  RelationEdit(MainWindow *parent);
  RelationEdit(MaquetteScene *scene,unsigned int relID, QWidget *parent);

  private slots :

  /*!
   * \brief Raised when a value has changed.
   */
  void setChanged();
  /*!
   * \brief Raised when min bound check box state has changed.
   */
  void minBoundChanged();
  /*!
   * \brief Raised when max bound check box state has changed.
   */
  void maxBoundChanged();
  /*!
   * \brief Updates relation configuration if changed
   * values are valid.
   * Raised when OK button is pressed.
   */
  void updateRelationConfiguration();

 private :

  MaquetteScene *_scene; //!< Logical representation of the scene.

	unsigned int _relID; //!< Relation in edition.

  QGridLayout *_layout; //!< Internal layout.

  QLabel *_timeLabel; //!< QLabel for time.
  QLabel *_minBoundLabel; //!< QLabel for time min bound.
  QLabel *_maxBoundLabel; //!< QLabel for time max bound.
  QLabel *_timeUnits;

  QDoubleSpinBox *_timeBox; //!< QSpinBox managing time value.
  QDoubleSpinBox *_minBoundBox; //!< QSpinBox managing time min bound value.
  QDoubleSpinBox *_maxBoundBox; //!< QSpinBox managing time max bound value.

  QCheckBox *_minBoundCheckBox;
  QCheckBox *_maxBoundCheckBox;

  QPushButton *_okButton; //!< Button used to confirme.
  QPushButton *_cancelButton; //!< Button used to cancel.

  bool _changed; //!< Handling if a value was changed.
};

#endif
