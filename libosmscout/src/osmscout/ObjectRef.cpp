/*
  This source is part of the libosmscout library
  Copyright (C) 2009  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <osmscout/ObjectRef.h>

namespace osmscout {

  const char* ObjectOSMRef::GetTypeName() const
  {
    switch (type) {
    case osmRefNode:
      return "Node";
    case osmRefWay:
      return "Way";
    case osmRefRelation:
      return "Relation";
    default:
      return "none";
    }
  }

  const char* ObjectFileRef::GetTypeName() const
  {
    switch (type) {
    case refNode:
      return "Node";
    case refArea:
      return "Area";
    case refWay:
      return "Way";
    default:
      return "none";
    }
  }
}

