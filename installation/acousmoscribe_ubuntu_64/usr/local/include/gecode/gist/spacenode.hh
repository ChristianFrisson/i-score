/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2009-09-29 12:49:42 +0200 (Tue, 29 Sep 2009) $ by $Author: tack $
 *     $Revision: 9766 $
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

#ifndef GECODE_GIST_SPACENODE_HH
#define GECODE_GIST_SPACENODE_HH

#include <gecode/gist/node.hh>
#include <gecode/kernel.hh>

namespace Gecode { namespace Gist {

  /** \brief Status of nodes in the search tree
   */
  enum NodeStatus {
    SOLVED,       ///< Node representing a solution
    FAILED,       ///< Node representing failure
    BRANCH,       ///< Node representing a branch
    UNDETERMINED, ///< Node that has not been explored yet
    SPECIAL,      ///< Node representing user controlled exploration
    STEP          ///< Node representing one propagation step
  };

  static const unsigned int FIRSTBIT = 25; //< First free bit in status word
  static const unsigned int STATUSMASK = 15<<20; //< Mask for accessing status
  static const unsigned int MAXDISTANCE = (1<<20)-1; //< Maximum representable distance
  static const unsigned int DISTANCEMASK = (1<<20)-1; //< Mask for accessing distance

  /// \brief Description for step nodes
  class StepDesc {
  public:
    int noOfSteps;
    bool debug;
    StepDesc(int steps);
    void toggleDebug(void);
  };

  /// \brief Description for special nodes
  class SpecialDesc {
  public:
    const std::string vn;
    const int v;
    const int rel;
    SpecialDesc(std::string varName, int rel0, int v0);
  };

  /// Statistics about the search tree
  class Statistics : public StatusStatistics {
  public:
    /// Number of solutions
    int solutions;
    /// Number of failures
    int failures;
    /// Number of choice nodes
    int choices;
    /// Number of open, undetermined nodes
    int undetermined;
    /// Maximum depth of the tree
    int maxDepth;

    /// Constructor
    Statistics(void)
      : solutions(0), failures(0), choices(0), undetermined(1), maxDepth(0) {}
  };

  class SpaceNode;

  /// \brief Static reference to the currently best space
  class BestNode {
  public:
    /// The currently best node found, or NULL
    SpaceNode* s;
    /// Constructor
    BestNode(SpaceNode* s0);
  };

  /// \brief A node of a search tree of Gecode spaces
  class SpaceNode : public Node {
  protected:
    /// A copy used for recomputation, or NULL
    Space* copy;
    /// Working space used for computing the status
    Space* workingSpace;
  private:
    /// Reference to best space when the node was created
    SpaceNode* ownBest;
    /** \brief Status of the node
     *
     * If the node has a workingSpace, the first 20 bits encode the distance
     * to the closest copy. The next 5 bits encode the NodeStatus, and the
     * remaining bits are used by the VisualNode class for further flags.
     */
    unsigned int nstatus;
  protected:
    union {
      /// Choice
      const Choice* branch;
      /// Special branching description
      const SpecialDesc* special;
      /// Step description
      StepDesc* step;
    } desc;

    /// Set distance from copy
    void setDistance(unsigned int d);
    
    /// Return distance from copy
    unsigned int getDistance(void) const;

    /// Set status flag
    void setFlag(int flag, bool value);

    /// Return status flag
    bool getFlag(int flag) const;

    /// Flags for SpaceNodes
    enum SpaceNodeFlags {
      HASOPENCHILDREN = FIRSTBIT,
      HASFAILEDCHILDREN,
      HASSOLVEDCHILDREN
    };
    /// Last bit used for SpaceNode flags
    static const int LASTBIT = HASSOLVEDCHILDREN;

  private:
    /// Set whether the node has children that are not fully explored
    void setHasOpenChildren(bool b);
    /// Set whether the subtree of this node is known to contain failure
    void setHasFailedChildren(bool b);
    /// Set whether the subtree of this node is known to contain solutions
    void setHasSolvedChildren(bool b);
    /// Set status to \a s
    void setStatus(NodeStatus s);

    /// Recompute workingSpace from a copy higher up, return distance to copy
    int recompute(BestNode* curBest, int c_d, int a_d);

    /// Book-keeping of open children
    void closeChild(bool hadFailures, bool hadSolutions);
  protected:
    /// Acquire working space, either from parent or by recomputation
    void acquireSpace(BestNode* curBest, int c_d, int a_d);
  public:
    /// Construct node
    SpaceNode(void);
    /// Construct root node from Space \a root and branch-and-bound object \a better
    SpaceNode(Space* root);
    /// Destructor
    ~SpaceNode(void);

    /// Return working space.  Receiver must delete the space.
    Space* getSpace(BestNode* curBest, int c_d, int a_d);

    /// Return working space (if present).
    const Space* getWorkingSpace(void) const;

    /// Clear working space and copy (if present and this is not the root).
    void purge(void);

    /// Return whether this node is the currently best solution
    bool isCurrentBest(BestNode* curBest);

    /** \brief Compute and return the number of children
      *
      * On a node whose status is already determined, this function
      * just returns the number of children.  On an undetermined node,
      * it first acquires a Space (possibly through recomputation), and
      * then asks for its status.  If the space is solved or failed, the
      * node's status will be set accordingly, and 0 will be returned.
      * Otherwise, the status is SS_BRANCH, and as many new children will
      * be created as the branch has alternatives, and the number returned.
      */
    int getNumberOfChildNodes(NodeAllocator& na,
                              BestNode* curBest,
                              Statistics& stats,
                              int c_d, int a_d);

    /// Return current status of the node
    NodeStatus getStatus(void) const;
    /// Return whether this node represents a propagation step
    bool isStepNode(void);
    /// Change the SpecialDesc to \a d
    void setSpecialDesc(const SpecialDesc* d);
    /// Change the StepDesc to \a d
    void setStepDesc(StepDesc* d);
    /// Return the StepDesc
    StepDesc* getStepDesc(void);

    /// Return whether this node still has open children
    bool isOpen(void);
    /// Return whether the subtree of this node has any failed children
    bool hasFailedChildren(void);
    /// Return whether the subtree of this node has any solved children
    bool hasSolvedChildren(void);
    /// Return whether the subtree of this node has any open children
    bool hasOpenChildren(void);
    /// Return number of open children
    int getNoOfOpenChildren(void);
    /// Set number of open children to \a n
    void setNoOfOpenChildren(int n);
    /// Return whether the node has a copy
    bool hasCopy(void);
    /// Return whether the node has a working space
    bool hasWorkingSpace(void);

    /// Return the parent
    SpaceNode* getParent(void);
    /// Return child \a i
    SpaceNode* getChild(int i);
    /// Return alternative number of this node
    int getAlternative(void);
  };

}}

#include <gecode/gist/spacenode.hpp>

#endif

// STATISTICS: gist-any