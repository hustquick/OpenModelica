/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Link�ping University,
 * Department of Computer and Information Science,
 * SE-58183 Link�ping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Link�ping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include <stdlib.h>

#define NR_REAL_ELEMENTS    1000000
#define NR_INTEGER_ELEMENTS 1000000
#define NR_STRING_ELEMENTS  10000
#define NR_BOOLEAN_ELEMENTS 10000
#define NR_SIZE_ELEMENTS    1000000
#define NR_INDEX_ELEMENTS   1000000
#define NR_CHAR_ELEMENTS    10000

typedef double      m_real;
typedef long        m_integer;
typedef const char* m_string;
typedef signed char m_boolean;
typedef m_integer   _index_t;

struct state_s {
  _index_t real_buffer_ptr;
  _index_t integer_buffer_ptr;
  _index_t string_buffer_ptr;
  _index_t boolean_buffer_ptr;
  _index_t size_buffer_ptr;
  _index_t index_buffer_ptr;
  _index_t char_buffer_ptr;
};

typedef struct state_s state;

state get_memory_state();
void restore_memory_state(state restore_state);
void clear_memory_state();

/*Help functions*/
void print_current_state();

/* Allocation functions */
m_real* real_alloc(int ix, int n);
m_integer* integer_alloc(int ix, int n);
m_string* string_alloc(int ix, int n);
m_boolean* boolean_alloc(int ix, int n);
_index_t* size_alloc(int ix, int n);
_index_t** index_alloc(int ix, int n);
char* char_alloc(int ix, int n);

void* push_memory_states(int maxThreads);
void pop_memory_states(void* new_states);

#endif