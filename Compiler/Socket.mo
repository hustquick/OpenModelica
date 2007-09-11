/*
 * This file is part of OpenModelica.
 * 
 * Copyright (c) 1998-2007, Linköpings universitet, Department of
 * Computer and Information Science, PELAB
 * 
 * All rights reserved.
 * 
 * (The new BSD license, see also
 * http://www.opensource.org/licenses/bsd-license.php)
 * 
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 
 *  Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * 
 *  Neither the name of Linköpings universitet nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
 
package Socket 
" file:        Socket.mo
  package:     Socket
  description: Modelica socket communication module
 
  RCS: $Id$
 
  This is the socket connection module of the compiler
  Used in interactive mode if omc is started with +d=interactive
  Implemented in ./runtime/soecketimpl.c
  Not implemented in Win32 builds use +d=interactiveCorba instead."

public function waitforconnect
  input Integer inInteger;
  output Integer outInteger;

  external "C" ;
end waitforconnect;

public function handlerequest
  input Integer inInteger;
  output String outString;

  external "C" ;
end handlerequest;

public function sendreply
  input Integer inInteger;
  input String inString;

  external "C" ;
end sendreply;

public function close
  input Integer inInteger;

  external "C" ;
end close;

public function cleanup

  external "C" ;
end cleanup;
end Socket;

