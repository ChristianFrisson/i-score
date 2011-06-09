/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2009-09-08 21:10:29 +0200 (Tue, 08 Sep 2009) $ by $Author: schulte $
 *     $Revision: 9692 $
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

namespace Gecode { namespace Gist {

  template<class Cursor>
  NodeVisitor<Cursor>::NodeVisitor(Cursor& c0) : c(c0) {}

  template<class Cursor>
  void
  NodeVisitor<Cursor>::setCursor(Cursor& c0) { c = c0; }

  template<class Cursor>
  Cursor&
  NodeVisitor<Cursor>::getCursor(void) { return c; }

  template<class Cursor>
  void
  PostorderNodeVisitor<Cursor>::moveToLeaf(void) {
    while (c.mayMoveDownwards()) {
        c.moveDownwards();
    }
  }

  template<class Cursor>
  PostorderNodeVisitor<Cursor>::PostorderNodeVisitor(Cursor& c0)
    : NodeVisitor<Cursor>(c0) {
    moveToLeaf();
  }

  template<class Cursor>
  bool
  PostorderNodeVisitor<Cursor>::next(void) {
    c.processCurrentNode();
    if (c.mayMoveSidewards()) {
        c.moveSidewards();
        moveToLeaf();
    } else if (c.mayMoveUpwards()) {
        c.moveUpwards();
    } else {
        return false;
    }
    return true;
  }

  template<class Cursor>
  bool
  PreorderNodeVisitor<Cursor>::backtrack(void) {
    while (! c.mayMoveSidewards() && c.mayMoveUpwards()) {
      c.moveUpwards();
    }
    if (! c.mayMoveUpwards()) {
      return false;
    } else {
      c.moveSidewards();
    }
    return true;
  }

  template<class Cursor>
  PreorderNodeVisitor<Cursor>::PreorderNodeVisitor(Cursor& c0)
    : NodeVisitor<Cursor>(c0) {}

  template<class Cursor>
  bool
  PreorderNodeVisitor<Cursor>::next(void) {
    c.processCurrentNode();
    if (c.mayMoveDownwards()) {
      c.moveDownwards();
    } else if (c.mayMoveSidewards()) {
      c.moveSidewards();
    } else {
      return backtrack();
    }
    return true;
  }
}}

// STATISTICS: gist-any