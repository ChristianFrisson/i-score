/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2009-05-14 10:44:50 +0200 (Thu, 14 May 2009) $ by $Author: tack $
 *     $Revision: 9107 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef GECODE_GIST_QTGIST_HH
#define GECODE_GIST_QTGIST_HH

#include <gecode/gist/treecanvas.hh>
#include <gecode/gist/nodestats.hh>

namespace Gecode {  namespace Gist {

  /**
   * \brief %Gecode Interactive %Search Tool
   *
   * This class provides an interactive search tree viewer and explorer as
   * a Qt widget. You can embedd or inherit from this widget to use %Gist
   * in your own project.
   *
   * \ingroup TaskGist
   */
  class Gist : public QWidget {
    Q_OBJECT
  private:
    /// The canvas implementation
    TreeCanvas* canvas;
    /// The time slider
    QSlider* timeBar;
    /// Context menu
    QMenu* contextMenu;
    /// Action used when no solution inspector is registered
    QAction* nullSolutionInspector;
    /// Menu of solution inspectors
    QMenu* solutionInspectorMenu;
    /// Action used when no double click inspector is registered
    QAction* nullDoubleClickInspector;
    /// Menu of double click inspectors
    QMenu* doubleClickInspectorMenu;
    /// Information about individual nodes
    NodeStatInspector* nodeStatInspector;
  public:
    /// Inspect current node
    QAction* inspect;
    /// Stop search
    QAction* stop;
    /// Reset %Gist
    QAction* reset;
    /// Navigate to parent node
    QAction* navUp;
    /// Navigate to leftmost child node
    QAction* navDown;
    /// Navigate to left sibling
    QAction* navLeft;
    /// Navigate to right sibling
    QAction* navRight;
    /// Navigate to root node
    QAction* navRoot;
    /// Navigate to next solution (to the left)
    QAction* navNextSol;
    /// Navigate to previous solution (to the right)
    QAction* navPrevSol;
    /// Search next solution in current subtree
    QAction* searchNext;
    /// Search all solutions in current subtree
    QAction* searchAll;
    /// Toggle whether current node is hidden
    QAction* toggleHidden;
    /// Hide failed subtrees under current node
    QAction* hideFailed;
    /// Unhide all hidden subtrees under current node
    QAction* unhideAll;
    /// Zoom tree to fit window
    QAction* zoomToFit;
    /// Center on current node
    QAction* center;
    /// Export PDF of current subtree
    QAction* exportPDF;
    /// Export PDF of whole tree
    QAction* exportWholeTreePDF;
    /// Print tree
    QAction* print;

    /// Set path from current node to the root
    QAction* setPath;
    /// Inspect all nodes on selected path
    QAction* inspectPath;
    /// Open node statistics inspector
    QAction* showNodeStats;

    /// Group of all actions for solution inspectors
    QActionGroup* solutionInspectorGroup;
    /// Group of all actions for double click inspectors
    QActionGroup* doubleClickInspectorGroup;
  public:
    /// Constructor
    Gist(Space* root, bool bab, QWidget* parent, const Options& opt);
    /// Destructor
    ~Gist(void);

    /// Add double click inspector \a i0
    void addDoubleClickInspector(Inspector* i0);
    /// Add solution inspector \a i0
    void addSolutionInspector(Inspector* i0);

    /// Set preference whether to automatically hide failed subtrees
    void setAutoHideFailed(bool b);
    /// Set preference whether to automatically zoom to fit
    void setAutoZoom(bool b);
    /// Return preference whether to automatically hide failed subtrees
    bool getAutoHideFailed(void);
    /// Return preference whether to automatically zoom to fit
    bool getAutoZoom(void);
    /// Set preference whether to show copies in the tree
    void setShowCopies(bool b);
    /// Return preference whether to show copies in the tree
    bool getShowCopies(void);

    /// Set refresh rate
    void setRefresh(int i);
    /// Return preference whether to use smooth scrolling and zooming
    bool getSmoothScrollAndZoom(void);
    /// Set preference whether to use smooth scrolling and zooming
    void setSmoothScrollAndZoom(bool b);

    /// Set recomputation parameters \a c_d and \a a_d
    void setRecompDistances(int c_d, int a_d);
    /// Return recomputation distance
    int getCd(void);
    /// Return adaptive recomputation distance
    int getAd(void);

    /// Stop search and wait until finished
    bool finish(void);

    /// Handle resize event
    void resizeEvent(QResizeEvent*);

  Q_SIGNALS:
    /// Signals that the tree has changed
    void statusChanged(const Statistics&, bool);

    /// Signals that a solution has been found
    void solution(const Space*);

    /// Signals that %Gist is ready to be closed
    void finished(void);

  private Q_SLOTS:
    /// Displays the context menu for a node
    void on_canvas_contextMenu(QContextMenuEvent*);
    /// Reacts on status changes
    void on_canvas_statusChanged(VisualNode*, const Statistics&, bool);
    /// Reacts on double click inspector selection
    void selectDoubleClickInspector(QAction*);
    /// Reacts on solution inspector selection
    void selectSolutionInspector(QAction*);
    /// Shows node status information
    void showStats(void);
  protected:
    /// Add inspector \a i0
    void addInspector(Inspector* i, bool solutionInspector);
  };

}}

#endif

// STATISTICS: gist-any