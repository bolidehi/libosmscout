/*
  This source is part of the libosmscout library
  Copyright (C) 2013  Tim Teulings

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

#include <osmscout/Area.h>

#include <limits>

#include <osmscout/system/Math.h>
#include <iostream>
namespace osmscout {

  bool AreaAttributes::SetTags(Progress& /*progress*/,
                               const TypeConfig& typeConfig,
                               std::vector<Tag>& tags)
  {
    uint32_t namePriority=0;
    uint32_t nameAltPriority=0;

    name.clear();
    nameAlt.clear();
    location.clear();
    address.clear();

    flags=0;

    this->tags.clear();

    flags|=hasAccess;

    std::vector<Tag>::iterator tag=tags.begin();
    while (tag!=tags.end()) {
      uint32_t ntPrio;
      bool     isNameTag=typeConfig.IsNameTag(tag->key,ntPrio);
      uint32_t natPrio;
      bool     isNameAltTag=typeConfig.IsNameAltTag(tag->key,natPrio);

      if (isNameTag &&
          (name.empty() || ntPrio>namePriority)) {
        name=tag->value;
        namePriority=ntPrio;

        /*
        size_t i=0;
        while (postfixes[i]!=NULL) {
          size_t pos=name.rfind(postfixes[i]);
          if (pos!=std::string::npos &&
              pos==name.length()-strlen(postfixes[i])) {
            name=name.substr(0,pos);
            break;
          }

          i++;
        }*/
      }

      if (isNameAltTag &&
          (nameAlt.empty() || natPrio>nameAltPriority)) {
        nameAlt=tag->value;
        nameAltPriority=natPrio;
      }

      if (isNameTag || isNameAltTag) {
        tag=tags.erase(tag);
      }
      else if (tag->key==typeConfig.tagStreet) {
        location=tag->value;
        tag=tags.erase(tag);
      }
      else if (tag->key==typeConfig.tagHouseNr) {
        address=tag->value;
        tag=tags.erase(tag);
      }
      else if (tag->key==typeConfig.tagAccess) {
        if (tag->value=="no" ||
            tag->value=="private" ||
            tag->value=="destination" ||
            tag->value=="delivery") {
          flags&=~hasAccess;
        }

        tag=tags.erase(tag);
      }
      else {
        ++tag;
      }
    }

    this->tags=tags;

    return true;
  }

  void AreaAttributes::GetFlags(uint8_t& flags) const
  {
    flags=0;

    if (!name.empty()) {
      flags|=hasName;
    }

    if (!nameAlt.empty()) {
      flags|=hasNameAlt;
    }

    if (!location.empty()) {
      flags|=hasLocation;
    }

    if (!address.empty()) {
      flags|=hasAddress;
    }

    if (!tags.empty()) {
      flags|=hasTags;
    }
  }

  bool AreaAttributes::Read(FileScanner& scanner)
  {
    uint8_t flags;

    scanner.Read(flags);

    if (scanner.HasError()) {
      return false;
    }

    return Read(scanner,
                flags);
  }

  bool AreaAttributes::Read(FileScanner& scanner,
                            uint8_t flags)
  {
    this->flags=flags;

    if (flags & hasName) {
      scanner.Read(name);
    }

    if (flags & hasNameAlt) {
      scanner.Read(nameAlt);
    }

    if (flags & hasLocation) {
      scanner.Read(location);
    }

    if (flags & hasAddress) {
      scanner.Read(address);
    }

    if (flags & hasTags) {
      uint32_t tagCount;

      scanner.ReadNumber(tagCount);
      if (scanner.HasError()) {
        return false;
      }

      tags.resize(tagCount);
      for (size_t i=0; i<tagCount; i++) {
        scanner.ReadNumber(tags[i].key);
        scanner.Read(tags[i].value);
      }
    }

    return !scanner.HasError();
  }

  bool AreaAttributes::Write(FileWriter& writer) const
  {
    uint8_t flags;

    GetFlags(flags);

    writer.Write(flags);

    return Write(writer,
                 flags);
  }

  bool AreaAttributes::Write(FileWriter& writer,
                             uint8_t flags) const
  {
    if (flags & hasName) {
      writer.Write(name);
    }

    if (flags & hasNameAlt) {
      writer.Write(nameAlt);
    }

    if (flags & hasLocation) {
      writer.Write(location);
    }

    if (flags & hasAddress) {
      writer.Write(address);
    }

    if (flags & hasTags) {
      writer.WriteNumber((uint32_t)tags.size());

      for (size_t i=0; i<tags.size(); i++) {
        writer.WriteNumber(tags[i].key);
        writer.Write(tags[i].value);
      }
    }

    return !writer.HasError();
  }

  bool AreaAttributes::operator==(const AreaAttributes& other) const
  {
    if (name!=other.name ||
        nameAlt!=other.nameAlt ||
        location!=other.location ||
        address!=other.address) {
      return false;
    }

    if (tags.empty() && other.tags.empty()) {
      return true;
    }

    if (tags.size()!=other.tags.size()) {
      return false;
    }

    for (size_t t=0; t<tags.size(); t++) {
      if (tags[t].key!=other.tags[t].key) {
        return false;
      }

      if (tags[t].value!=other.tags[t].value) {
        return false;
      }
    }

    return true;
  }

  bool AreaAttributes::operator!=(const AreaAttributes& other) const
  {
    return !this->operator==(other);
  }

  bool Area::Ring::GetCenter(double& lat, double& lon) const
  {
    double minLat=0.0;
    double minLon=0.0;
    double maxLat=0.0;
    double maxLon=0.0;

    bool start=true;

    for (size_t j=0; j<nodes.size(); j++) {
      if (start) {
        minLat=nodes[j].GetLat();
        minLon=nodes[j].GetLon();
        maxLat=nodes[j].GetLat();
        maxLon=nodes[j].GetLon();

        start=false;
      }
      else {
        minLat=std::min(minLat,nodes[j].GetLat());
        minLon=std::min(minLon,nodes[j].GetLon());
        maxLat=std::max(maxLat,nodes[j].GetLat());
        maxLon=std::max(maxLon,nodes[j].GetLon());
      }
    }

    if (start) {
      return false;
    }

    lat=minLat+(maxLat-minLat)/2;
    lon=minLon+(maxLon-minLon)/2;

    return true;
  }

  void Area::Ring::GetBoundingBox(double& minLon,
                                  double& maxLon,
                                  double& minLat,
                                  double& maxLat) const
  {
    assert(!nodes.empty());

    minLon=nodes[0].GetLon();
    maxLon=minLon;
    minLat=nodes[0].GetLat();
    maxLat=minLat;

    for (size_t i=1; i<nodes.size(); i++) {
      minLon=std::min(minLon,nodes[i].GetLon());
      maxLon=std::max(maxLon,nodes[i].GetLon());
      minLat=std::min(minLat,nodes[i].GetLat());
      maxLat=std::max(maxLat,nodes[i].GetLat());
    }
  }

  bool Area::GetCenter(double& lat, double& lon) const
  {
    assert(!rings.empty());

    double minLat=0.0;
    double minLon=0.0;
    double maxLat=0.0;
    double maxLon=0.0;

    bool start=true;

    for (size_t i=0; i<rings.size(); i++) {
      if (rings[i].ring==Area::outerRingId) {
        for (size_t j=0; j<rings[i].nodes.size(); j++) {
          if (start) {
            minLat=rings[i].nodes[j].GetLat();
            maxLat=minLat;
            minLon=rings[i].nodes[j].GetLon();
            maxLon=minLon;

            start=false;
          }
          else {
            minLat=std::min(minLat,rings[i].nodes[j].GetLat());
            minLon=std::min(minLon,rings[i].nodes[j].GetLon());
            maxLat=std::max(maxLat,rings[i].nodes[j].GetLat());
            maxLon=std::max(maxLon,rings[i].nodes[j].GetLon());
          }
        }
      }
    }

    assert(!start);

    if (start) {
      return false;
    }

    lat=minLat+(maxLat-minLat)/2;
    lon=minLon+(maxLon-minLon)/2;

    return true;
  }

  void Area::GetBoundingBox(double& minLon,
                            double& maxLon,
                            double& minLat,
                            double& maxLat) const
  {
    assert(!rings.empty());

    for (std::vector<Area::Ring>::const_iterator role=rings.begin();
         role!=rings.end();
         ++role) {
      if (role->ring==Area::outerRingId) {
        role->GetBoundingBox(minLon,
                             maxLon,
                             minLat,
                             maxLat);
      }
    }
  }

  bool Area::ReadIds(FileScanner& scanner,
                     uint32_t nodesCount,
                     std::vector<Id>& ids)
  {
    Id minId;

    ids.resize(nodesCount);

    scanner.ReadNumber(minId);
    for (size_t j=0; j<nodesCount; j++) {
      Id id;

      scanner.ReadNumber(id);

      ids[j]=minId+id;
    }

    return !scanner.HasError();
  }

  bool Area::ReadCoords(FileScanner& scanner,
                        uint32_t nodesCount,
                        std::vector<GeoCoord>& coords)
  {
    uint32_t minLat;
    uint32_t minLon;

    coords.resize(nodesCount);

    scanner.Read(minLat);
    scanner.Read(minLon);

    for (size_t j=0; j<nodesCount; j++) {
      uint32_t latValue;
      uint32_t lonValue;

      scanner.ReadNumber(latValue);
      scanner.ReadNumber(lonValue);

      coords[j].Set((minLat+latValue)/conversionFactor-90.0,
                     (minLon+lonValue)/conversionFactor-180.0);
    }

    return !scanner.HasError();
  }

  bool Area::Read(FileScanner& scanner)
  {
    if (!scanner.GetPos(fileOffset)) {
      return false;
    }

    uint32_t ringCount=1;
    uint8_t  outerFlags;
    uint32_t nodesCount;

    scanner.Read(outerFlags);

    if (!(outerFlags & AreaAttributes::isSimple)) {
      scanner.ReadNumber(ringCount);
      if (scanner.HasError()) {
        return false;
      }

      ringCount++;
    }

    rings.resize(ringCount);

    scanner.ReadNumber(rings[0].type);

    if (!rings[0].attributes.Read(scanner,
                                  outerFlags)) {
      return false;
    }

    if (ringCount>1) {
      rings[0].ring=masterRingId;
    }
    else {
      rings[0].ring=outerRingId;
    }

    scanner.ReadNumber(nodesCount);

    if (nodesCount>0) {
      if (!ReadIds(scanner,
                   nodesCount,
                   rings[0].ids)) {
        return false;
      }

      if (!ReadCoords(scanner,
                      nodesCount,
                      rings[0].nodes)) {
        return false;
      }
    }

    for (size_t i=1; i<ringCount; i++) {
      scanner.ReadNumber(rings[i].type);

      if (rings[i].type!=typeIgnore) {
        if (!rings[i].attributes.Read(scanner)) {
          return false;
        }
      }

      scanner.Read(rings[i].ring);

      scanner.ReadNumber(nodesCount);

      if (nodesCount>0 &&
          rings[i].type!=typeIgnore) {
        if (!ReadIds(scanner,
                     nodesCount,
                     rings[i].ids)) {
          return false;
        }
      }

      if (nodesCount>0) {
        if (!ReadCoords(scanner,
                        nodesCount,
                        rings[i].nodes)) {
          return false;
        }
      }
    }

    return !scanner.HasError();
  }

  bool Area::ReadOptimized(FileScanner& scanner)
  {
    if (!scanner.GetPos(fileOffset)) {
      return false;
    }

    uint32_t ringCount=1;
    uint8_t  outerFlags;
    uint32_t nodesCount;

    scanner.Read(outerFlags);

    if (!(outerFlags & AreaAttributes::isSimple)) {
      scanner.ReadNumber(ringCount);
      if (scanner.HasError()) {
        return false;
      }

      ringCount++;
    }

    rings.resize(ringCount);

    scanner.ReadNumber(rings[0].type);

    if (!rings[0].attributes.Read(scanner,
                                  outerFlags)) {
      return false;
    }

    if (ringCount>1) {
      rings[0].ring=masterRingId;
    }
    else {
      rings[0].ring=outerRingId;
    }

    scanner.ReadNumber(nodesCount);

    if (nodesCount>0) {
      if (!ReadCoords(scanner,
                      nodesCount,
                      rings[0].nodes)) {
        return false;
      }
    }

    for (size_t i=1; i<ringCount; i++) {
      scanner.ReadNumber(rings[i].type);

      if (rings[i].type!=typeIgnore) {
        if (!rings[i].attributes.Read(scanner)) {
          return false;
        }
      }

      scanner.Read(rings[i].ring);

      scanner.ReadNumber(nodesCount);

      if (nodesCount>0) {
        if (!ReadCoords(scanner,
                        nodesCount,
                        rings[i].nodes)) {
          return false;
        }
      }
    }

    return !scanner.HasError();
  }

  bool Area::WriteIds(FileWriter& writer,
                      const std::vector<Id>& ids) const
  {
    Id minId=std::numeric_limits<Id>::max();

    for (size_t j=0; j<ids.size(); j++) {
      minId=std::min(minId,ids[j]);
    }

    writer.WriteNumber(minId);

    for (size_t j=0; j<ids.size(); j++) {
      writer.WriteNumber(ids[j]-minId);
    }

    return !writer.HasError();
  }

  bool Area::WriteCoords(FileWriter& writer,
                         const std::vector<GeoCoord>& coords) const
  {
    uint32_t minLat=std::numeric_limits<uint32_t>::max();
    uint32_t minLon=std::numeric_limits<uint32_t>::max();

    for (size_t j=0; j<coords.size(); j++) {
      minLat=std::min(minLat,(uint32_t)round((coords[j].GetLat()+90.0)*conversionFactor));
      minLon=std::min(minLon,(uint32_t)round((coords[j].GetLon()+180.0)*conversionFactor));
    }

    writer.Write(minLat);
    writer.Write(minLon);

    for (size_t j=0; j<coords.size(); j++) {
      uint32_t latValue=(uint32_t)round((coords[j].GetLat()+90.0)*conversionFactor);
      uint32_t lonValue=(uint32_t)round((coords[j].GetLon()+180.0)*conversionFactor);

      writer.WriteNumber(latValue-minLat);
      writer.WriteNumber(lonValue-minLon);
    }

    return !writer.HasError();
  }

  bool Area::Write(FileWriter& writer) const
  {
    std::vector<Ring>::const_iterator ring=rings.begin();

    // Outer ring

    uint8_t outerFlags;

    ring->attributes.GetFlags(outerFlags);

    if (rings.size()==1) {
      outerFlags|=AreaAttributes::isSimple;
    }

    writer.Write(outerFlags);

    if (rings.size()>1) {
      writer.WriteNumber((uint32_t)rings.size()-1);
    }

    writer.WriteNumber(ring->type);

    if (!ring->attributes.Write(writer,
                                outerFlags)) {
      return false;
    }

    writer.WriteNumber((uint32_t)ring->nodes.size());

    if (!ring->nodes.empty()) {
      if (!WriteIds(writer,
                    ring->ids)) {
        return false;
      }

      if (!WriteCoords(writer,
                       ring->nodes)) {
        return false;
      }
    }

    ++ring;

    // Potential additional rings

    while (ring!=rings.end()) {
      writer.WriteNumber(ring->type);

      if (ring->type!=typeIgnore) {
        if (!ring->attributes.Write(writer)) {
          return false;
        }
      }

      writer.Write(ring->ring);

      writer.WriteNumber((uint32_t)ring->nodes.size());

      if (!ring->nodes.empty() &&
          ring->type!=typeIgnore) {
        if (!WriteIds(writer,
                      ring->ids)) {
          return false;
        }
      }

      if (!ring->nodes.empty()) {
        if (!WriteCoords(writer,
                        ring->nodes)) {
          return false;
        }
      }

      ++ring;
    }

    return !writer.HasError();
  }

  bool Area::WriteOptimized(FileWriter& writer) const
  {
    std::vector<Ring>::const_iterator ring=rings.begin();

    // Outer ring

    uint8_t outerFlags;

    ring->attributes.GetFlags(outerFlags);

    if (rings.size()==1) {
      outerFlags|=AreaAttributes::isSimple;
    }

    writer.Write(outerFlags);

    if (rings.size()>1) {
      writer.WriteNumber((uint32_t)rings.size()-1);
    }

    writer.WriteNumber(ring->type);

    if (!ring->attributes.Write(writer,
                                outerFlags)) {
      return false;
    }

    writer.WriteNumber((uint32_t)ring->nodes.size());

    if (!ring->nodes.empty()) {
      if (!WriteCoords(writer,
                      ring->nodes)) {
        return false;
      }
    }

    ++ring;

    // Potential additional rings

    while (ring!=rings.end()) {
      writer.WriteNumber(ring->type);

      if (ring->type!=typeIgnore) {
        if (!ring->attributes.Write(writer)) {
          return false;
        }
      }

      writer.Write(ring->ring);

      writer.WriteNumber((uint32_t)ring->nodes.size());

      if (!ring->nodes.empty()) {
        if (!WriteCoords(writer,
                        ring->nodes)) {
          return false;
        }
      }

      ++ring;
    }

    return !writer.HasError();
  }
}

