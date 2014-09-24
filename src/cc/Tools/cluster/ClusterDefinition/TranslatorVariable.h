/* -*- c++ -*-
 * Copyright (C) 2007-2014 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 3 of the
 * License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/// @file
/// Declarations for TranslatorVariable.
/// This file contains type declarations for TranslatorVariable, a class for
/// translating variable definition statements.

#ifndef Tools_cluster_TranslatorVariable_h
#define Tools_cluster_TranslatorVariable_h

#include "TranslationContext.h"
#include "Translator.h"

#include <string>

namespace Hypertable { namespace ClusterDefinition {

  using namespace std;

  /// @addtogroup ClusterDefinition
  /// @{

  class TranslatorVariable : public Translator {
  public:
    TranslatorVariable(const string &text) : m_text(text) {};
    const string translate(TranslationContext &context) override;
  private:
    string m_text;
  };

  /// @}

}}

#endif // Tools_cluster_TranslatorVariable_h
