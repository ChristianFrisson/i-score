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
#ifndef CHOOSE_TEMPORAL_RELATION
#define CHOOSE_TEMPORAL_RELATION

#include <QDialog>

class QComboBox;
class QGridLayout;
class QPushButton;
class QGroupBox;
class QRadioButton;
class QSpinBox;
class QLabel;

class MainWindow;
class MaquetteScene;

class ConstrainedTemporalEntity;

class ChooseTemporalRelationPanel : public QDialog
{
  Q_OBJECT

  public:
    ChooseTemporalRelationPanel(MainWindow *parent);
    ChooseTemporalRelationPanel(unsigned int ID1, unsigned int ID2, MaquetteScene *scene, QWidget *parent);

  private slots:
    /**
     *  Create a relation between objects stored
     */
    void createRelation();

    /**
     * Buttons toggled slots
     */
    void allenRadioToggled();
    void intervalRadioToggled();

  private:
    /// Logical representation of the main window
    MainWindow *_mainWindow;

    /// Logical representation of the scene
    MaquetteScene *_scene;

    /// Entity concerned in a relation
    //ConstrainedTemporalEntity *_entity1;
    unsigned int _ent1ID;

    //ConstrainedTemporalEntity *_entity2;
    unsigned int _ent2ID;

    /// Layout, labels, etc ...
    QGridLayout *_layout;

    QGridLayout *_relationLayout;
    QLabel *_name1;
    QGroupBox *_relationTypeBox;
    QLabel *_name2;
    QRadioButton *_allenRadio;
    QRadioButton *_intervalRadio;

    QComboBox *_allenComboBox;

    QGroupBox *_intervalBox;
    QSpinBox *_valueBox;
    QSpinBox *_toleranceBox;

    QPushButton *_okButton;
    QPushButton *_cancelButton;
};
#endif
