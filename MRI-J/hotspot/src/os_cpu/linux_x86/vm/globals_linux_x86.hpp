/*
 * Copyright 2000-2005 Sun Microsystems, Inc.  All Rights Reserved.
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
#ifndef GLOBALS_OS_PD_HPP
#define GLOBALS_OS_PD_HPP


//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//

define_pd_global(uintx,JVMInvokeMethodSlack,12288);

define_pd_global(uintx,JavaThreadMaxPriority,20);
define_pd_global(uintx,SignalThreadPriority,21);
define_pd_global(uintx,LowMemoryDetectorPriority,21);
define_pd_global(uintx,CompilerThreadPriority,21);
define_pd_global(uintx,SurrogateLockerPriority,22);
define_pd_global(uintx,CheckpointBoostPriority,23);
define_pd_global(uintx,GCThreadPriority,25);
define_pd_global(uintx,WatcherThreadPriority,28);
define_pd_global(uintx,TransportPriority,29);
define_pd_global(uintx,VMThreadPriority,30);
#endif // GLOBALS_OS_PD_HPP
