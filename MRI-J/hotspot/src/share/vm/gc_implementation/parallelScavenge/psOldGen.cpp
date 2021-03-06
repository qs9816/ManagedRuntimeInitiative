/*
 * Copyright 2001-2007 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *  
 */
// This file is a derivative work resulting from (and including) modifications
// made by Azul Systems, Inc.  The date of such changes is 2010.
// Copyright 2010 Azul Systems, Inc.  All Rights Reserved.
//
// Please contact Azul Systems, Inc., 1600 Plymouth Street, Mountain View, 
// CA 94043 USA, or visit www.azulsystems.com if you need additional information 
// or have any questions.


#include "cardTableModRefBS.hpp"
#include "collectedHeap.hpp"
#include "gcLocker.hpp"
#include "java.hpp"
#include "ostream.hpp"
#include "parallelScavengeHeap.hpp"
#include "psGenerationCounters.hpp"
#include "psMarkSweepDecorator.hpp"
#include "psOldGen.hpp"
#include "psParallelCompact.hpp"
#include "spaceCounters.hpp"
#include "tickProfiler.hpp"
#include "universe.hpp"

#include "atomic_os_pd.inline.hpp"
#include "gcLocker.inline.hpp"
#include "mutex.inline.hpp"
#include "oop.inline.hpp"
#include "thread_os.inline.hpp"

inline const char* PSOldGen::select_name() {
  return UseParallelOldGC ? "ParOldGen" : "PSOldGen";
}

PSOldGen::PSOldGen(ReservedSpace rs, size_t alignment,
		   size_t initial_size, size_t min_size, size_t max_size,
                   const char* perf_data_name, int level):
  _name(select_name()), _init_gen_size(initial_size), _min_gen_size(min_size),
  _max_gen_size(max_size)
{
  initialize(rs, alignment, perf_data_name, level);
}

PSOldGen::PSOldGen(size_t initial_size,
                   size_t min_size, size_t max_size,
                   const char* perf_data_name, int level):
  _name(select_name()), _init_gen_size(initial_size), _min_gen_size(min_size),
  _max_gen_size(max_size)
{}

void PSOldGen::initialize(ReservedSpace rs, size_t alignment,
			  const char* perf_data_name, int level) {
  initialize_virtual_space(rs, alignment);
  initialize_work(perf_data_name, level);
  // The old gen can grow to gen_size_limit().  _reserve reflects only
  // the current maximum that can be committed.
  assert(_reserved.byte_size() <= gen_size_limit(), "Consistency check");
}

void PSOldGen::initialize_virtual_space(ReservedSpace rs, size_t alignment, os::MemoryAccount account) {

  _virtual_space = new PSVirtualSpace(rs, alignment, account);
  if (!_virtual_space->expand_by(_init_gen_size)) {
    vm_exit_during_initialization("Could not reserve enough space for "
                                  "object heap");
  }
}

void PSOldGen::initialize_work(const char* perf_data_name, int level) {
  //
  // Basic memory initialization
  //

  MemRegion limit_reserved((HeapWord*)virtual_space()->low_boundary(),
    heap_word_size(_max_gen_size));
  assert(limit_reserved.byte_size() == _max_gen_size, 
    "word vs bytes confusion");
  //
  // Object start array stuff
  //

  // Ughh...need to come up with a better way to pick reserved space starts
char*reserved_space_start=NULL;
if(strcmp(perf_data_name,"old")==0){
    reserved_space_start = (char*) __OBJECT_START_ARRAY_OLD_START_ADDR__;
}else if(strcmp(perf_data_name,"perm")==0){
    reserved_space_start = (char*) __OBJECT_START_ARRAY_PERM_START_ADDR__;
  } else {
    ShouldNotReachHere();
  }
  start_array()->initialize(limit_reserved, reserved_space_start);

  _reserved = MemRegion((HeapWord*)virtual_space()->low_boundary(),
                        (HeapWord*)virtual_space()->high_boundary());

  //
  // Card table stuff
  //

  MemRegion cmr((HeapWord*)virtual_space()->low(),
		(HeapWord*)virtual_space()->high());
  Universe::heap()->barrier_set()->resize_covered_region(cmr);

  CardTableModRefBS* _ct = (CardTableModRefBS*)Universe::heap()->barrier_set();
  assert (_ct->kind() == BarrierSet::CardTableModRef, "Sanity");

  // Verify that the start and end of this generation is the start of a card.
  // If this wasn't true, a single card could span more than one generation,
  // which would cause problems when we commit/uncommit memory, and when we
  // clear and dirty cards.
  guarantee(_ct->is_card_aligned(_reserved.start()), "generation must be card aligned");
  if (_reserved.end() != Universe::heap()->reserved_region().end()) {
    // Don't check at the very end of the heap as we'll assert that we're probing off
    // the end if we try.
    guarantee(_ct->is_card_aligned(_reserved.end()), "generation must be card aligned");
  }

  //
  // ObjectSpace stuff
  //

  _object_space = new MutableSpace();
  
  if (_object_space == NULL)
    vm_exit_during_initialization("Could not allocate an old gen space");

  object_space()->initialize(cmr, true);

  _object_mark_sweep = new PSMarkSweepDecorator(_object_space, start_array(), MarkSweepDeadRatio);

  if (_object_mark_sweep == NULL)
    vm_exit_during_initialization("Could not complete allocation of old generation");

  // Update the start_array
  start_array()->set_covered_region(cmr);

  // Generation Counters, generation 'level', 1 subspace
  _gen_counters = new PSGenerationCounters(perf_data_name, level, 1,
					   virtual_space());
  _space_counters = new SpaceCounters(perf_data_name, 0,
                                      virtual_space()->reserved_size(),
                                      _object_space, _gen_counters);
}

// Assume that the generation has been allocated if its
// reserved size is not 0.
bool  PSOldGen::is_allocated() {
  return virtual_space()->reserved_size() != 0;
}

void PSOldGen::precompact() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  // Reset start array first.
  debug_only(if (!UseParallelOldGC || !VerifyParallelOldWithMarkSweep) {)
  start_array()->reset();
  debug_only(})

  object_mark_sweep()->precompact();

  // Now compact the young gen
  heap->young_gen()->precompact();
}

void PSOldGen::adjust_pointers() {
  object_mark_sweep()->adjust_pointers();
}

void PSOldGen::compact() {
  object_mark_sweep()->compact(ZapUnusedHeapArea);
}

void PSOldGen::move_and_update(ParCompactionManager* cm) {
  PSParallelCompact::move_and_update(cm, PSParallelCompact::old_space_id);
}

size_t PSOldGen::contiguous_available() const {
  return object_space()->free_in_bytes() + virtual_space()->uncommitted_size();
}

// Allocation. We report all successful allocations to the size policy
// Note that the perm gen does not use this method, and should not!
HeapWord* PSOldGen::allocate(size_t word_size, bool is_tlab) {
  assert_locked_or_safepoint(Heap_lock);
  HeapWord* res = allocate_noexpand(word_size, is_tlab);

  if (res == NULL) {
    res = expand_and_allocate(word_size, is_tlab);
  }

  // Allocations in the old generation need to be reported
  if (res != NULL) {
    ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
    heap->size_policy()->tenured_allocation(word_size);
  }

  return res;
}

HeapWord* PSOldGen::expand_and_allocate(size_t word_size, bool is_tlab) {
  assert(!is_tlab, "TLAB's are not supported in PSOldGen");
  expand(word_size*HeapWordSize);
  if (GCExpandToAllocateDelayMillis > 0) {
    os::sleep(Thread::current(), GCExpandToAllocateDelayMillis, false);
  }
  return allocate_noexpand(word_size, is_tlab);
}

HeapWord* PSOldGen::expand_and_cas_allocate(size_t word_size) {
  expand(word_size*HeapWordSize);
  if (GCExpandToAllocateDelayMillis > 0) {
    os::sleep(Thread::current(), GCExpandToAllocateDelayMillis, false);
  }
  return cas_allocate_noexpand(word_size);
}

void PSOldGen::expand(size_t bytes) {
  MutexLocker x(ExpandHeap_lock);
  const size_t alignment = virtual_space()->alignment();
  size_t aligned_bytes  = align_size_up(bytes, alignment);
  size_t aligned_expand_bytes = align_size_up(MinHeapDeltaBytes, alignment);

  bool success = false;
  if (aligned_expand_bytes > aligned_bytes) {
    success = expand_by(aligned_expand_bytes);
  }
  if (!success) {
    success = expand_by(aligned_bytes);
  }
  if (!success) {
    success = expand_to_reserved();
  }

  if (GC_locker::is_active()) {
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("Garbage collection disabled, expanded heap instead");
    }
  }
}

bool PSOldGen::expand_by(size_t bytes) {
  assert_lock_strong(ExpandHeap_lock);
  assert_locked_or_safepoint(Heap_lock);
  bool result = virtual_space()->expand_by(bytes);
  if (result) {
    post_resize();
    if (UsePerfData) {
      _space_counters->update_capacity();
      _gen_counters->update_all();
    }
  }

  if (result && Verbose && PrintGC) {
    size_t new_mem_size = virtual_space()->committed_size();
    size_t old_mem_size = new_mem_size - bytes;
    gclog_or_tty->print_cr("Expanding %s from " SIZE_FORMAT "K by " 
                                       SIZE_FORMAT "K to " 
                                       SIZE_FORMAT "K",
                    name(), old_mem_size/K, bytes/K, new_mem_size/K);
  }

  return result;
}

bool PSOldGen::expand_to_reserved() {
  assert_lock_strong(ExpandHeap_lock);
  assert_locked_or_safepoint(Heap_lock);

  bool result = true;
  const size_t remaining_bytes = virtual_space()->uncommitted_size();
  if (remaining_bytes > 0) {
    result = expand_by(remaining_bytes);
    DEBUG_ONLY(if (!result) warning("grow to reserve failed"));
  }
  return result;
}

void PSOldGen::shrink(size_t bytes) {
  assert_lock_strong(ExpandHeap_lock);
  assert_locked_or_safepoint(Heap_lock);

  size_t size = align_size_down(bytes, virtual_space()->alignment());
  if (size > 0) {
    assert_lock_strong(ExpandHeap_lock);
    virtual_space()->shrink_by(bytes);
    post_resize();

    if (Verbose && PrintGC) {
      size_t new_mem_size = virtual_space()->committed_size();
      size_t old_mem_size = new_mem_size + bytes;
      gclog_or_tty->print_cr("Shrinking %s from " SIZE_FORMAT "K by " 
                                         SIZE_FORMAT "K to " 
                                         SIZE_FORMAT "K",
                      name(), old_mem_size/K, bytes/K, new_mem_size/K);
    }
  }
}

void PSOldGen::resize(size_t desired_free_space) {
  const size_t alignment = virtual_space()->alignment();
  const size_t size_before = virtual_space()->committed_size();
  size_t new_size = used_in_bytes() + desired_free_space;
  if (new_size < used_in_bytes()) {
    // Overflowed the addition.
    new_size = gen_size_limit();
  }
  // Adjust according to our min and max
  new_size = MAX2(MIN2(new_size, gen_size_limit()), min_gen_size());

  assert(gen_size_limit() >= reserved().byte_size(), "max new size problem?");
  new_size = align_size_up(new_size, alignment);

  const size_t current_size = capacity_in_bytes();

  if (PrintAdaptiveSizePolicy && Verbose) {
    gclog_or_tty->print_cr("AdaptiveSizePolicy::old generation size: "
      "desired free: " SIZE_FORMAT " used: " SIZE_FORMAT
      " new size: " SIZE_FORMAT " current size " SIZE_FORMAT
      " gen limits: " SIZE_FORMAT " / " SIZE_FORMAT,
      desired_free_space, used_in_bytes(), new_size, current_size,
      gen_size_limit(), min_gen_size());
  }

  if (new_size == current_size) {
    // No change requested
    return;
  }
  if (new_size > current_size) {
    size_t change_bytes = new_size - current_size;
    expand(change_bytes);
  } else {
    size_t change_bytes = current_size - new_size;
    // shrink doesn't grab this lock, expand does. Is that right?
    MutexLocker x(ExpandHeap_lock);
    shrink(change_bytes);
  }

  if (PrintAdaptiveSizePolicy) {
    ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
    assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
    gclog_or_tty->print_cr("AdaptiveSizePolicy::old generation size: "
                  "collection: %d "
                  "(" SIZE_FORMAT ") -> (" SIZE_FORMAT ") ",
                  heap->total_collections(),
                  size_before, virtual_space()->committed_size());
  }
}

// NOTE! We need to be careful about resizing. During a GC, multiple
// allocators may be active during heap expansion. If we allow the
// heap resizing to become visible before we have correctly resized
// all heap related data structures, we may cause program failures.
void PSOldGen::post_resize() {
  // First construct a memregion representing the new size
  MemRegion new_memregion((HeapWord*)virtual_space()->low(), 
    (HeapWord*)virtual_space()->high());
  size_t new_word_size = new_memregion.word_size();

  start_array()->set_covered_region(new_memregion);
  Universe::heap()->barrier_set()->resize_covered_region(new_memregion);

  // Did we expand?
  HeapWord* const virtual_space_high = (HeapWord*) virtual_space()->high();
  if (object_space()->end() < virtual_space_high) {
    // We need to mangle the newly expanded area. The memregion spans
    // end -> new_end, we assume that top -> end is already mangled.
    // This cannot be safely tested for, as allocation may be taking
    // place.
    MemRegion mangle_region(object_space()->end(), virtual_space_high);
    object_space()->mangle_region(mangle_region); 
  }

  // ALWAYS do this last!!
  object_space()->set_end(virtual_space_high);

  assert(new_word_size == heap_word_size(object_space()->capacity_in_bytes()),
    "Sanity");
}

size_t PSOldGen::gen_size_limit() { 
  return _max_gen_size;
}

void PSOldGen::reset_after_change() {
  ShouldNotReachHere();
  return;
}

size_t PSOldGen::available_for_expansion() {
  ShouldNotReachHere();
  return 0;
}

size_t PSOldGen::available_for_contraction() {
  ShouldNotReachHere();
  return 0;
}

void PSOldGen::print() const { print_on(tty);}
void PSOldGen::print_on(outputStream* st) const {
  st->print(" %-15s", name());
  if (PrintGCDetails && Verbose) {
    st->print(" total " SIZE_FORMAT ", used " SIZE_FORMAT, 
                capacity_in_bytes(), used_in_bytes());
  } else {
    st->print(" total " SIZE_FORMAT "K, used " SIZE_FORMAT "K", 
                capacity_in_bytes()/K, used_in_bytes()/K);
  }
st->print_cr(" ["PTR_FORMAT", "PTR_FORMAT", "PTR_FORMAT")",
		virtual_space()->low_boundary(),
		virtual_space()->high(),
		virtual_space()->high_boundary());

  st->print("  object"); object_space()->print_on(st);
}

void PSOldGen::print_used_change(size_t prev_used) const {
  gclog_or_tty->print(" [%s:", name());
  gclog_or_tty->print(" "  SIZE_FORMAT "K"
                      "->" SIZE_FORMAT "K"
                      "("  SIZE_FORMAT "K)",
                      prev_used / K, used_in_bytes() / K,
                      capacity_in_bytes() / K);
  gclog_or_tty->print("]");
}

void PSOldGen::update_counters() {
  if (UsePerfData) {
    _space_counters->update_all();
    _gen_counters->update_all();
  }
}

#ifndef PRODUCT

void PSOldGen::space_invariants() {
  assert(object_space()->end() == (HeapWord*) virtual_space()->high(), 
    "Space invariant");
  assert(object_space()->bottom() == (HeapWord*) virtual_space()->low(), 
    "Space invariant");
  assert(virtual_space()->low_boundary() <= virtual_space()->low(), 
    "Space invariant");
  assert(virtual_space()->high_boundary() >= virtual_space()->high(), 
    "Space invariant");
  assert(virtual_space()->low_boundary() == (char*) _reserved.start(), 
    "Space invariant");
  assert(virtual_space()->high_boundary() == (char*) _reserved.end(), 
    "Space invariant");
  assert(virtual_space()->committed_size() <= virtual_space()->reserved_size(),
    "Space invariant");
}
#endif

void PSOldGen::verify(bool allow_dirty) {
  object_space()->verify(allow_dirty);
}
class VerifyObjectStartArrayClosure : public ObjectClosure {
  PSOldGen* _gen;
  ObjectStartArray* _start_array;

 public:
  VerifyObjectStartArrayClosure(PSOldGen* gen, ObjectStartArray* start_array) :
    _gen(gen), _start_array(start_array) { }

  virtual void do_object(oop obj) {
    if( obj->size()==1 ) return; // ignore size==1-word Objects
    HeapWord* test_addr = (HeapWord*)obj + 1;
    guarantee(_start_array->object_start(test_addr) == (HeapWord*)obj, "ObjectStartArray cannot find start of object");
    guarantee(_start_array->is_block_allocated((HeapWord*)obj), "ObjectStartArray missing block allocation");
  }
};

void PSOldGen::verify_object_start_array() {
  VerifyObjectStartArrayClosure check( this, &_start_array );
  object_iterate(&check);
}
