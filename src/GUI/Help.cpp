/*
 * Copyright: LaBRI / SCRIME
 *
 * Authors: Luc Vercellin (08/03/2010)
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
#include "Help.hpp"

#include <QString>
#include <QFile>
#include <QLabel>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <string>
using std::string;

Help::Help(QWidget *parent)
  : QDialog(parent)
{
  //setWindowModality(Qt::ApplicationModal);

  setWindowTitle("Help");
  _layout = new QVBoxLayout();
  _tabs = new QTabWidget(this);
  _editorTabs = new QTabWidget(this);

  QString fileString;
  QFile file(":/documentation/file.htm");
  if (file.open(QIODevice::ReadOnly)) {
      fileString = QString(file.readAll());
    }
  else {
      fileString = tr("No Help Found");
    }

  _fileLabel = new QLabel(fileString);

  QString toolbarString;
  QFile toolbar(":/documentation/toolbar.htm");
  if (toolbar.open(QIODevice::ReadOnly)) {
      toolbarString = QString(toolbar.readAll());
    }
  else {
      toolbarString = tr("No Help Found");
    }
  _toolBarLabel = new QLabel(toolbarString);

  QString contextString;
  QFile context(":/documentation/contextual.htm");
  if (context.open(QIODevice::ReadOnly)) {
      contextString = QString(context.readAll());
    }
  else {
      contextString = tr("No Help Found");
    }
  _contextLabel = new QLabel(contextString);

  QString interactionString;
  QFile interaction(":/documentation/interaction.htm");
  if (interaction.open(QIODevice::ReadOnly)) {
      interactionString = QString(interaction.readAll());
    }
  else {
      interactionString = tr("No Help Found");
    }
  _interactionLabel = new QLabel(interactionString);

  QString editorGeneralString;
  QFile editorGeneral(":/documentation/editorGeneral.htm");
  if (editorGeneral.open(QIODevice::ReadOnly)) {
      editorGeneralString = QString(editorGeneral.readAll());
    }
  else {
      editorGeneralString = tr("No Help Found");
    }
  _editorGeneralLabel = new QLabel(editorGeneralString);

  QString editorProfilesString;
  QFile editorProfiles(":/documentation/editorProfiles.htm");
  if (editorProfiles.open(QIODevice::ReadOnly)) {
      editorProfilesString = QString(editorProfiles.readAll());
    }
  else {
      editorProfilesString = tr("No Help Found");
    }
  _editorProfilesLabel = new QLabel(editorProfilesString);

  QString editorMessagesString;
  QFile editorMessages(":/documentation/editorMessages.htm");
  if (editorMessages.open(QIODevice::ReadOnly)) {
      editorMessagesString = QString(editorMessages.readAll());
    }
  else {
      editorMessagesString = tr("No Help Found");
    }
  _editorMessagesLabel = new QLabel(editorMessagesString);

  QString editorSnapshotString;
  QFile editorSnapshot(":/documentation/editorSnapshot.htm");
  if (editorSnapshot.open(QIODevice::ReadOnly)) {
      editorSnapshotString = QString(editorSnapshot.readAll());
    }
  else {
      editorSnapshotString = tr("No Help Found");
    }
  _editorSnapshotLabel = new QLabel(editorSnapshotString);

  QString editorCurvesString;
  QFile editorCurves(":/documentation/editorCurves.htm");
  if (editorCurves.open(QIODevice::ReadOnly)) {
      editorCurvesString = QString(editorCurves.readAll());
    }
  else {
      editorCurvesString = tr("No Help Found");
    }
  _editorCurvesLabel = new QLabel(editorCurvesString);

  QScrollArea *fileScrollArea = new QScrollArea;
  fileScrollArea->setWidget(_fileLabel);

  //_tabs->addTab(fileScrollArea,tr("File"));

  QScrollArea *toolBarScrollArea = new QScrollArea;
  toolBarScrollArea->setWidget(_toolBarLabel);
  _tabs->addTab(toolBarScrollArea, tr("Toolbar"));

  QScrollArea *contextScrollArea = new QScrollArea;
  contextScrollArea->setWidget(_contextLabel);

  //_tabs->addTab(contextScrollArea,"Contextual Menu");
  QScrollArea *interactionScrollArea = new QScrollArea;
  interactionScrollArea->setWidget(_interactionLabel);
  _tabs->addTab(interactionScrollArea, tr("Interaction"));

  QScrollArea *editorGeneralScrollArea = new QScrollArea;
  editorGeneralScrollArea->setWidget(_editorGeneralLabel);
  _editorTabs->addTab(editorGeneralScrollArea, tr("General"));

  QScrollArea *editorProfilesScrollArea = new QScrollArea;
  editorProfilesScrollArea->setWidget(_editorProfilesLabel);
  _editorTabs->addTab(editorProfilesScrollArea, tr("Profiles"));

  QScrollArea *editorMessagesScrollArea = new QScrollArea;
  editorMessagesScrollArea->setWidget(_editorMessagesLabel);
  _editorTabs->addTab(editorMessagesScrollArea, tr("Messages"));

  QScrollArea *editorSnapshotScrollArea = new QScrollArea;
  editorSnapshotScrollArea->setWidget(_editorSnapshotLabel);
  _editorTabs->addTab(editorSnapshotScrollArea, tr("Snapshot"));

  QScrollArea *editorCurvesScrollArea = new QScrollArea;
  editorCurvesScrollArea->setWidget(_editorCurvesLabel);
  _editorTabs->addTab(editorCurvesScrollArea, tr("Curves"));

  _tabs->addTab(_editorTabs, tr("Attributes Editor"));

  _layout->addWidget(_tabs);

  setLayout(_layout);
  adjustSize();
}
