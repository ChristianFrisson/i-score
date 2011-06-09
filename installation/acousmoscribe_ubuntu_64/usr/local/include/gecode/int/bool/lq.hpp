/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date: 2009-01-08 17:25:53 +0100 (Thu, 08 Jan 2009) $ by $Author: tack $
 *     $Revision: 8043 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

namespace Gecode { namespace Int { namespace Bool {

  /*
   * Less or equal propagator
   *
   */

  template<class BV>
  forceinline
  Lq<BV>::Lq(Space& home, BV b0, BV b1)
    : BoolBinary<BV,BV>(home,b0,b1) {}

  template<class BV>
  forceinline
  Lq<BV>::Lq(Space& home, bool share, Lq<BV>& p)
    : BoolBinary<BV,BV>(home,share,p) {}

  template<class BV>
  Actor*
  Lq<BV>::copy(Space& home, bool share) {
    return new (home) Lq<BV>(home,share,*this);
  }

  template<class BV>
  inline ExecStatus
  Lq<BV>::post(Space& home, BV b0, BV b1) {
    if (b0.zero()) {
      return ES_OK;
    } else if (b0.one()) {
      GECODE_ME_CHECK(b1.one(home));
    } else if (b1.zero()) {
      GECODE_ME_CHECK(b0.zero(home));
    } else if (b1.one()) {
      return ES_OK;
    } else {
      (void) new (home) Lq<BV>(home,b0,b1);
    }
    return ES_OK;
  }

  template<class BV>
  ExecStatus
  Lq<BV>::propagate(Space& home, const ModEventDelta&) {
#define GECODE_INT_STATUS(S0,S1) \
  ((BV::S0<<(1*BV::BITS))|(BV::S1<<(0*BV::BITS)))
    switch ((x0.status()<<(1*BV::BITS)) | (x1.status()<<(0*BV::BITS))) {
    case GECODE_INT_STATUS(NONE,NONE):
      GECODE_NEVER;
    case GECODE_INT_STATUS(NONE,ZERO):
      GECODE_ME_CHECK(x0.zero_none(home)); break;
    case GECODE_INT_STATUS(NONE,ONE):
      x0.cancel(home,*this,PC_BOOL_VAL); break;
    case GECODE_INT_STATUS(ZERO,NONE):
      x1.cancel(home,*this,PC_BOOL_VAL); break;
    case GECODE_INT_STATUS(ZERO,ZERO):
      break;
    case GECODE_INT_STATUS(ZERO,ONE):
      break;
    case GECODE_INT_STATUS(ONE,NONE):
      GECODE_ME_CHECK(x1.one_none(home)); break;
    case GECODE_INT_STATUS(ONE,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(ONE,ONE):
      break;
    default:
      GECODE_NEVER;
    }
    return ES_SUBSUMED(*this,sizeof(*this));
#undef GECODE_INT_STATUS
  }

  /*
   * Less posting
   *
   */

  template<class BV>
  forceinline ExecStatus
  Le<BV>::post(Space& home, BV b0, BV b1) {
    GECODE_ME_CHECK(b0.zero(home));
    GECODE_ME_CHECK(b1.one(home));
    return ES_OK;
  }

}}}

// STATISTICS: int-prop
