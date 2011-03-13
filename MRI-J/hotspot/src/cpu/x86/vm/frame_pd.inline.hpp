// Copyright 2010 Azul Systems, Inc.  All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is free software; you can redistribute it and/or modify it under 
// the terms of the GNU General Public License version 2 only, as published by 
// the Free Software Foundation. 
//
// This code is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License version 2 for  more
// details (a copy is included in the LICENSE file that accompanied this code).
//
// You should have received a copy of the GNU General Public License version 2 
// along with this work; if not, write to the Free Software Foundation,Inc., 
// 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
// 
// Please contact Azul Systems, Inc., 1600 Plymouth Street, Mountain View, 
// CA 94043 USA, or visit www.azulsystems.com if you need additional information 
// or have any questions.
#ifndef FRAME_PD_INLINE_HPP
#define FRAME_PD_INLINE_HPP

#include "interpreter_pd.hpp"
#include "stubRoutines.hpp"
#include "synchronizer.hpp"
#include "thread.hpp"

// Inline functions for x86 frames:

// Return unique id for this frame.  The id must have a value where we can distinguish
// identity and younger/older relationship.  NULL represents an invalid (incomparable)
// frame.  Nothing exciting for X86 and  IA64's must report r-stack not normal-stack.
inline intptr_t* frame::id(void) const { return sp(); /* Use RSP */ }

// --- IFrame
// Interpreter Control Frames.  Stored on the normal C stack.
struct IFrame {
  constantPoolCacheRef _cpc; // Constant Pool Cache Ref; for active frames RSP points here
  methodRef _mref;      // MethodRef being executed
  uint32_t _loc;        // #bytes to start of locals on the JEXSTK from Thread*
  uint32_t _stk;        // #bytes to top                 of JEXSTK from Thread*
  uint16_t _bci;        // Obvious BCI
  uint16_t _numlck;     // Number of simo locks (max 256)
  uint32_t _pad;        // Even number of 8-byte words
  uint64_t _pad1;       // Even number of 8-byte words; really ought to be a saved RBP or RBX
  address _retadr;      // X86 return address
};


// --- pd_sender
// Create frame for sender. 
inline frame frame::pd_sender() const { 
  assert0( !is_entry_frame() ); // do not ask for sender of an entry frame; instead the caller uses the C anchor
  int framesize_bytes = CodeCache::find_blob(_pc)->framesize_bytes();
  // do not assert much sanity here: the tick profiler calls here with weird pcs & stacks
  // just try not to crash
  intptr_t *ssp = (intptr_t*)((char*)_sp+framesize_bytes);
  return frame( ssp, (address)ssp[-1] );
}

inline constantPoolCacheRef* frame::interpreter_frame_cache_addr   () const { return &ifr()->_cpc;    }
inline methodRef*            frame::interpreter_frame_method_addr  () const { return &ifr()->_mref;   }
inline uint16_t *            frame::interpreter_frame_bci_addr     () const { return &ifr()->_bci;    }
inline int                   frame::interpreter_frame_monitor_count() const { return  ifr()->_numlck; }
inline intptr_t*frame::interpreter_frame_locals_base()const{
  return ((intptr_t*)(((intptr_t)_sp & ~(BytesPerThreadStack-1)) | ifr()->_loc));
}
inline intptr_t*             frame::interpreter_frame_tos_at( int idx ) const {
  return ((intptr_t*)(((intptr_t)_sp & ~(BytesPerThreadStack-1)) | ifr()->_stk))-idx-1;
}

// Entry frames (aka call stubs):
// ------------------------------
//
// Call stubs are used in the entry to the interpreter from native code. They
// copy arguments from a native array onto the expression stack. They also
// save and restore JT->_jexstk_top.

enum call_stub_layout {
  call_stub_saved_rty_offset   = 24+sizeof(struct IFrame), // incoming thread argument, saved return type
  call_stub_param_count_offset = 16+sizeof(struct IFrame), // incoming parameter count argument
  call_stub_saved_jcw_offset   = 16+sizeof(struct IFrame), // saved java call wrapper offset
  call_stub_frame_size         = 16+sizeof(struct IFrame),
  call_stub_return_address     =  8+sizeof(struct IFrame), // standard x86 return address
  call_stub_saved_rbp_offset   =  0+sizeof(struct IFrame), // saved RBP offset where GDB can find it
  call_stub_iframe             =  0,
};

// For an entry frame (ie. a frame generated by a call stub) return the address
// of the stack pointer for the previous frame
inline JavaCallWrapper* frame::entry_frame_call_wrapper() const {
  assert0(is_entry_frame());
  return *(JavaCallWrapper**)((char*)sp()+call_stub_saved_jcw_offset);
}

#endif // FRAME_PD_INLINE_HPP