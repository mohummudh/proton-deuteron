////////////////////////////////////////////////////////////////////////
// Class:       BoundingBoxCalDataFilter
// Plugin Type: producer (art v3_05_01)
// File:        BoundingBoxCalDataFilter_module.cc
//
// In LArIAT RecoModule
//
// Generated at Mon Aug 12 2025 by msultan using cetskelgen
// from cetlib version v3_10_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "larcore/Geometry/Geometry.h"
#include "larcorealg/Geometry/GeometryCore.h"
#include "lardataobj/RecoBase/Wire.h"
#include "lardataobj/RawData/RawDigit.h"

#include <fstream>
#include <sstream>
#include <map>
#include <memory>
#include <algorithm>

namespace bbox {

  struct EventBox {
    int run, subrun, event;
    int c_w_i, c_w_f, c_t_i, c_t_f;  // collection plane bounds
    int i_w_i, i_w_f, i_t_i, i_t_f;  // induction plane bounds
  };

  class BoundingBoxCalDataFilter;
}

class bbox::BoundingBoxCalDataFilter : public art::EDProducer {
public:
  explicit BoundingBoxCalDataFilter(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  BoundingBoxCalDataFilter(BoundingBoxCalDataFilter const&) = delete;
  BoundingBoxCalDataFilter(BoundingBoxCalDataFilter&&) = delete;
  BoundingBoxCalDataFilter& operator=(BoundingBoxCalDataFilter const&) = delete;
  BoundingBoxCalDataFilter& operator=(BoundingBoxCalDataFilter&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;

private:
  std::string fCalDataLabel;
  std::string fCSVFile;
  bool fZeroOutside;
  bool fDebugMode;
  std::map<std::tuple<int,int,int>, EventBox> fEventBoxes;
  
  art::ServiceHandle<geo::Geometry const> fGeometry;
  
  void loadCSV();
  bool isWireInBox(raw::ChannelID_t channel, const EventBox& box, int& t_start, int& t_end);
};

bbox::BoundingBoxCalDataFilter::BoundingBoxCalDataFilter(fhicl::ParameterSet const& p)
  : EDProducer{p}
  , fCalDataLabel(p.get<std::string>("CalDataModuleLabel"))
  , fCSVFile(p.get<std::string>("CSVFile"))
  , fZeroOutside(p.get<bool>("ZeroOutside", true))
  , fDebugMode(p.get<bool>("DebugMode", false))
{
  produces<std::vector<recob::Wire>>();
  loadCSV();
  
  if (fDebugMode) {
    mf::LogInfo("BoundingBoxCalDataFilter") << "Loaded " << fEventBoxes.size() 
                                            << " bounding boxes from " << fCSVFile;
  }
}

void bbox::BoundingBoxCalDataFilter::loadCSV() {
  std::ifstream file(fCSVFile);
  if (!file.is_open()) {
    throw cet::exception("BoundingBoxCalDataFilter") 
      << "Cannot open CSV file: " << fCSVFile;
  }
  
  std::string line;
  
  // Skip header line
  if (!std::getline(file, line)) {
    throw cet::exception("BoundingBoxCalDataFilter") 
      << "Empty CSV file: " << fCSVFile;
  }
  
  int lineNum = 1;
  while (std::getline(file, line)) {
    lineNum++;
    std::stringstream ss(line);
    std::string item;
    EventBox box;
    
    try {
      std::getline(ss, item, ','); box.run = std::stoi(item);
      std::getline(ss, item, ','); box.subrun = std::stoi(item);
      std::getline(ss, item, ','); box.event = std::stoi(item);
      std::getline(ss, item, ','); box.c_w_i = std::stoi(item);
      std::getline(ss, item, ','); box.c_w_f = std::stoi(item);
      std::getline(ss, item, ','); box.c_t_i = std::stoi(item);
      std::getline(ss, item, ','); box.c_t_f = std::stoi(item);
      std::getline(ss, item, ','); box.i_w_i = std::stoi(item);
      std::getline(ss, item, ','); box.i_w_f = std::stoi(item);
      std::getline(ss, item, ','); box.i_t_i = std::stoi(item);
      std::getline(ss, item, ','); box.i_t_f = std::stoi(item);
      
      auto key = std::make_tuple(box.run, box.subrun, box.event);
      fEventBoxes[key] = box;
      
      if (fDebugMode) {
        mf::LogInfo("BoundingBoxCalDataFilter") 
          << "Loaded box for event " << box.run << ":" << box.subrun << ":" << box.event
          << " Collection: wires " << box.c_w_i << "-" << box.c_w_f 
          << ", ticks " << box.c_t_i << "-" << box.c_t_f
          << " Induction: wires " << box.i_w_i << "-" << box.i_w_f 
          << ", ticks " << box.i_t_i << "-" << box.i_t_f;
      }
      
    } catch (const std::exception& e) {
      throw cet::exception("BoundingBoxCalDataFilter") 
        << "Error parsing CSV line " << lineNum << ": " << line << "\n"
        << "Error: " << e.what();
    }
  }
  file.close();
}

bool bbox::BoundingBoxCalDataFilter::isWireInBox(raw::ChannelID_t channel, 
                                                  const EventBox& box, 
                                                  int& t_start, int& t_end) {
  // Get wire information from geometry
  auto const& wireIDs = fGeometry->ChannelToWire(channel);
  if (wireIDs.empty()) return false;
  
  auto const& wid = wireIDs.front();
  unsigned int wireNum = wid.Wire;
  
  // Use SignalType to determine collection vs induction
  bool is_collection = (fGeometry->SignalType(channel) == geo::kCollection);
  
  // Get maximum wire number for this plane to clamp bounds
  int max_wire = fGeometry->Nwires(wid.Plane, wid.TPC, wid.Cryostat) - 1;
  
  bool wireInBox = false;
  if (is_collection) { // Collection plane
    // Clamp CSV bounds to valid ranges
    int c_w_i_clamped = std::clamp(box.c_w_i, 0, max_wire);
    int c_w_f_clamped = std::clamp(box.c_w_f, 0, max_wire);
    
    wireInBox = (wireNum >= static_cast<unsigned int>(c_w_i_clamped) && 
                 wireNum <= static_cast<unsigned int>(c_w_f_clamped));
    t_start = box.c_t_i;
    t_end = box.c_t_f;
    
    if (fDebugMode && wireInBox) {
      mf::LogInfo("BoundingBoxCalDataFilter") 
        << "Collection wire " << wireNum << " (channel " << channel 
        << ") in box [" << c_w_i_clamped << "-" << c_w_f_clamped << "]";
    }
  } else { // Induction plane(s)
    // Clamp CSV bounds to valid ranges
    int i_w_i_clamped = std::clamp(box.i_w_i, 0, max_wire);
    int i_w_f_clamped = std::clamp(box.i_w_f, 0, max_wire);
    
    wireInBox = (wireNum >= static_cast<unsigned int>(i_w_i_clamped) && 
                 wireNum <= static_cast<unsigned int>(i_w_f_clamped));
    t_start = box.i_t_i;
    t_end = box.i_t_f;
    
    if (fDebugMode && wireInBox) {
      mf::LogInfo("BoundingBoxCalDataFilter") 
        << "Induction wire " << wireNum << " (channel " << channel 
        << ") in box [" << i_w_i_clamped << "-" << i_w_f_clamped << "]";
    }
  }
  
  return wireInBox;
}

void bbox::BoundingBoxCalDataFilter::produce(art::Event& evt) {
  auto calDataHandle = evt.getValidHandle<std::vector<recob::Wire>>(fCalDataLabel);
  auto outputWires = std::make_unique<std::vector<recob::Wire>>();
  outputWires->reserve(calDataHandle->size());
  
  // Get event information
  int run = evt.run();
  int subrun = evt.subRun();
  int event = evt.event();
  auto key = std::make_tuple(run, subrun, event);
  
  if (fDebugMode) {
    mf::LogInfo("BoundingBoxCalDataFilter") 
      << "Processing event " << run << ":" << subrun << ":" << event
      << " with " << calDataHandle->size() << " input wires";
  }
  
  // Check if this event has a bounding box defined
  auto boxIt = fEventBoxes.find(key);
  if (boxIt == fEventBoxes.end()) {
    // No box defined, pass through all wires unchanged
    for (const auto& wire : *calDataHandle) {
      outputWires->push_back(wire);
    }
    
    if (fDebugMode) {
      mf::LogInfo("BoundingBoxCalDataFilter") 
        << "No bounding box defined for event " << run << ":" << subrun << ":" << event
        << ", passing through " << outputWires->size() << " wires unchanged";
    }
  } else {
    const EventBox& box = boxIt->second;
    int wiresFiltered = 0;
    int wiresZeroed = 0;
    int wiresKept = 0;
    
    for (const auto& wire : *calDataHandle) {
      raw::ChannelID_t channel = wire.Channel();
      int t_start, t_end;
      
      if (isWireInBox(channel, box, t_start, t_end)) {
        // Wire is in bounding box, apply time filtering
        const auto& originalSignal = wire.Signal();
        
        // Clamp time bounds to valid signal range
        int signal_size = static_cast<int>(originalSignal.size());
        t_start = std::clamp(t_start, 0, signal_size - 1);
        t_end = std::clamp(t_end, 0, signal_size - 1);
        
        std::vector<float> filteredSignal;
        filteredSignal.reserve(originalSignal.size());
        
        // Filter signal by time bounds
        for (size_t tick = 0; tick < originalSignal.size(); ++tick) {
          if (static_cast<int>(tick) >= t_start && static_cast<int>(tick) <= t_end) {
            filteredSignal.push_back(originalSignal[tick]);
          } else if (fZeroOutside) {
            filteredSignal.push_back(0.0);
          } else {
            filteredSignal.push_back(originalSignal[tick]);
          }
        }
        
        // Use direct Wire construction
        outputWires->emplace_back(filteredSignal, channel, wire.View());
        wiresFiltered++;
        
      } else if (fZeroOutside) {
        // Wire outside box, zero it out
        std::vector<float> zeroSignal(wire.Signal().size(), 0.0);
        outputWires->emplace_back(zeroSignal, channel, wire.View());
        wiresZeroed++;
      } else {
        // Keep original wire unchanged
        outputWires->push_back(wire);
        wiresKept++;
      }
    }
    
    if (fDebugMode) {
      mf::LogInfo("BoundingBoxCalDataFilter") 
        << "Applied bounding box to event " << run << ":" << subrun << ":" << event
        << ": " << wiresFiltered << " wires filtered, " 
        << wiresZeroed << " wires zeroed, " 
        << wiresKept << " wires kept unchanged";
    }
  }
  
  evt.put(std::move(outputWires));
}

DEFINE_ART_MODULE(bbox::BoundingBoxCalDataFilter)
