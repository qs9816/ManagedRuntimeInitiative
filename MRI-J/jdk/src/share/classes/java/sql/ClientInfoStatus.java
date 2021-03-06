/*
 * Copyright 2006 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
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
 */

package java.sql;

import java.util.*;

/**
 * Enumeration for status of the reason that a property could not be set
 * via a call to <code>Connection.setClientInfo</code>
 * @since 1.6
 */

public enum ClientInfoStatus {

    /**
     * The client info property could not be set for some unknown reason
     * @since 1.6
     */
    REASON_UNKNOWN,

    /**
     * The client info property name specified was not a recognized property
     * name.
     * @since 1.6
     */
    REASON_UNKNOWN_PROPERTY,

    /**
     * The value specified for the client info property was not valid.
     * @since 1.6
     */
    REASON_VALUE_INVALID,

    /**
     * The value specified for the client info property was too large.
     * @since 1.6
     */
    REASON_VALUE_TRUNCATED
}
