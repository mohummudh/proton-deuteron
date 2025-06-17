//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Feb 26 15:02:06 2025 by ROOT version 6.32.10
// from TTree anatree/analysis tree
// found on file: MergedAnaTree_Run2_Beam_Pos100A_2022_01.root
//////////////////////////////////////////////////////////

#ifndef preselection_h
#define preselection_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class preselection {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           run;
   Int_t           subrun;
   Int_t           event;
   Double_t        evttime;
   Float_t         efield;
   Float_t         lifetime;
   Int_t           t0;
   Int_t           nclus;
   Float_t         clustertwire[550];   //[nclus]
   Float_t         clusterttick[550];   //[nclus]
   Float_t         cluendwire[550];   //[nclus]
   Float_t         cluendtick[550];   //[nclus]
   Int_t           cluplane[550];   //[nclus]
   Int_t           ntracks_reco;
   Float_t         trkvtxx[104];   //[ntracks_reco]
   Float_t         trkvtxy[104];   //[ntracks_reco]
   Float_t         trkvtxz[104];   //[ntracks_reco]
   Float_t         trkendx[104];   //[ntracks_reco]
   Float_t         trkendy[104];   //[ntracks_reco]
   Float_t         trkendz[104];   //[ntracks_reco]
   Float_t         trkstartdcosx[104];   //[ntracks_reco]
   Float_t         trkstartdcosy[104];   //[ntracks_reco]
   Float_t         trkstartdcosz[104];   //[ntracks_reco]
   Float_t         trkenddcosx[104];   //[ntracks_reco]
   Float_t         trkenddcosy[104];   //[ntracks_reco]
   Float_t         trkenddcosz[104];   //[ntracks_reco]
   Int_t           trkWCtoTPCMatch[104];   //[ntracks_reco]
   Float_t         trklength[104];   //[ntracks_reco]
   Int_t           trkg4id[104];   //[ntracks_reco]
   Int_t           primarytrkkey;
   Int_t           ntrkcalopts[104][2];   //[ntracks_reco]
   Float_t         trkpida[104][2];   //[ntracks_reco]
   Float_t         trkke[104][2];   //[ntracks_reco]
   Float_t         trkdedx[104][2][1000];   //[ntracks_reco]
   Float_t         trkdqdx[104][2][1000];   //[ntracks_reco]
   Float_t         trkrr[104][2][1000];   //[ntracks_reco]
   Float_t         trkpitch[104][2][1000];   //[ntracks_reco]
   Float_t         trkxyz[104][2][1000][3];   //[ntracks_reco]
   Int_t           nhits;
   Int_t           hit_plane[15740];   //[nhits]
   Int_t           hit_wire[15740];   //[nhits]
   Int_t           hit_channel[15740];   //[nhits]
   Float_t         hit_peakT[15740];   //[nhits]
   Float_t         hit_driftT[15740];   //[nhits]
   Float_t         hit_charge[15740];   //[nhits]
   Float_t         hit_electrons[15740];   //[nhits]
   Float_t         hit_ph[15740];   //[nhits]
   Float_t         hit_rms[15740];   //[nhits]
   Int_t           hit_g4id[15740];   //[nhits]
   Float_t         hit_g4frac[15740];   //[nhits]
   Float_t         hit_g4nelec[15740];   //[nhits]
   Float_t         hit_g4energy[15740];   //[nhits]
   Float_t         hit_tstart[15740];   //[nhits]
   Float_t         hit_tend[15740];   //[nhits]
   Int_t           hit_trkid[15740];   //[nhits]
   Float_t         hit_dQds[15740];   //[nhits]
   Float_t         hit_dEds[15740];   //[nhits]
   Float_t         hit_ds[15740];   //[nhits]
   Float_t         hit_resrange[15740];   //[nhits]
   Float_t         hit_x[15740];   //[nhits]
   Float_t         hit_y[15740];   //[nhits]
   Float_t         hit_z[15740];   //[nhits]
   Float_t         beamline_mass;
   Int_t           nwctrks;
   Float_t         wctrk_XFaceCoor[1];   //[nwctrks]
   Float_t         wctrk_YFaceCoor[1];   //[nwctrks]
   Float_t         wctrk_theta[1];   //[nwctrks]
   Float_t         wctrk_phi[1];   //[nwctrks]
   Float_t         wctrk_momentum[1];   //[nwctrks]
   Float_t         wctrk_Px[1];   //[nwctrks]
   Float_t         wctrk_Py[1];   //[nwctrks]
   Float_t         wctrk_Pz[1];   //[nwctrks]
   Float_t         wctrk_residual[1];   //[nwctrks]
   Int_t           wctrk_wcmissed[1];   //[nwctrks]
   Int_t           wctrk_picky[1];   //[nwctrks]
   Int_t           wctrk_WC1XMult[1];   //[nwctrks]
   Int_t           wctrk_WC1YMult[1];   //[nwctrks]
   Int_t           wctrk_WC2XMult[1];   //[nwctrks]
   Int_t           wctrk_WC2YMult[1];   //[nwctrks]
   Int_t           wctrk_WC3XMult[1];   //[nwctrks]
   Int_t           wctrk_WC3YMult[1];   //[nwctrks]
   Int_t           wctrk_WC4XMult[1];   //[nwctrks]
   Int_t           wctrk_WC4YMult[1];   //[nwctrks]
   Float_t         wctrk_XDist[1];   //[nwctrks]
   Float_t         wctrk_YDist[1];   //[nwctrks]
   Float_t         wctrk_ZDist[1];   //[nwctrks]
   Float_t         wctrk_YKink[1];   //[nwctrks]
   Int_t           ntof;
   Float_t         tof[1];   //[ntof]
   Float_t         tof_timestamp[1];   //[ntof]
   Int_t           maxTrackIDE;
   Float_t         IDEEnergy[1];   //[maxTrackIDE]
   Float_t         IDEPos[1][3];   //[maxTrackIDE]
   Int_t           no_primaries;
   Int_t           geant_list_size;
   Int_t           pdg[1];   //[geant_list_size]
   Float_t         Mass[1];   //[geant_list_size]
   Float_t         Eng[1];   //[geant_list_size]
   Float_t         Px[1];   //[geant_list_size]
   Float_t         Py[1];   //[geant_list_size]
   Float_t         Pz[1];   //[geant_list_size]
   Float_t         EndEng[1];   //[geant_list_size]
   Float_t         EndPx[1];   //[geant_list_size]
   Float_t         EndPy[1];   //[geant_list_size]
   Float_t         EndPz[1];   //[geant_list_size]
   Float_t         StartPointx[1];   //[geant_list_size]
   Float_t         StartPointy[1];   //[geant_list_size]
   Float_t         StartPointz[1];   //[geant_list_size]
   Float_t         EndPointx[1];   //[geant_list_size]
   Float_t         EndPointy[1];   //[geant_list_size]
   Float_t         EndPointz[1];   //[geant_list_size]
   Float_t         StartT[1];   //[geant_list_size]
   Float_t         EndT[1];   //[geant_list_size]
   Float_t         PathLenInTpcAV[1];   //[geant_list_size]
   Bool_t          StartInTpcAV[1];   //[geant_list_size]
   Bool_t          EndInTpcAV[1];   //[geant_list_size]
   Int_t           Process[1];   //[geant_list_size]
   Int_t           NumberDaughters[1];   //[geant_list_size]
   Int_t           Mother[1];   //[geant_list_size]
   Int_t           TrackId[1];   //[geant_list_size]
   Int_t           process_primary[1];   //[geant_list_size]
   vector<string>  *G4Process;
   vector<string>  *G4FinalProcess;

   // List of branches
   TBranch        *b_run;   //!
   TBranch        *b_subrun;   //!
   TBranch        *b_event;   //!
   TBranch        *b_evttime;   //!
   TBranch        *b_efield;   //!
   TBranch        *b_lifetime;   //!
   TBranch        *b_t0;   //!
   TBranch        *b_nclus;   //!
   TBranch        *b_clustertwire;   //!
   TBranch        *b_clusterttick;   //!
   TBranch        *b_cluendwire;   //!
   TBranch        *b_cluendtick;   //!
   TBranch        *b_cluplane;   //!
   TBranch        *b_ntracks_reco;   //!
   TBranch        *b_trkvtxx;   //!
   TBranch        *b_trkvtxy;   //!
   TBranch        *b_trkvtxz;   //!
   TBranch        *b_trkendx;   //!
   TBranch        *b_trkendy;   //!
   TBranch        *b_trkendz;   //!
   TBranch        *b_trkstartdcosx;   //!
   TBranch        *b_trkstartdcosy;   //!
   TBranch        *b_trkstartdcosz;   //!
   TBranch        *b_trkenddcosx;   //!
   TBranch        *b_trkenddcosy;   //!
   TBranch        *b_trkenddcosz;   //!
   TBranch        *b_trkWCtoTPCMatch;   //!
   TBranch        *b_trklength;   //!
   TBranch        *b_trkg4id;   //!
   TBranch        *b_primarytrkkey;   //!
   TBranch        *b_ntrkcalopts;   //!
   TBranch        *b_trkpida;   //!
   TBranch        *b_trkke;   //!
   TBranch        *b_trkdedx;   //!
   TBranch        *b_trkdqdx;   //!
   TBranch        *b_trkrr;   //!
   TBranch        *b_trkpitch;   //!
   TBranch        *b_trkxyz;   //!
   TBranch        *b_nhits;   //!
   TBranch        *b_hit_plane;   //!
   TBranch        *b_hit_wire;   //!
   TBranch        *b_hit_channel;   //!
   TBranch        *b_hit_peakT;   //!
   TBranch        *b_hit_driftT;   //!
   TBranch        *b_hit_charge;   //!
   TBranch        *b_hit_electrons;   //!
   TBranch        *b_hit_ph;   //!
   TBranch        *b_hit_rms;   //!
   TBranch        *b_hit_g4id;   //!
   TBranch        *b_hit_g4frac;   //!
   TBranch        *b_hit_g4nelec;   //!
   TBranch        *b_hit_g4energy;   //!
   TBranch        *b_hit_tstart;   //!
   TBranch        *b_hit_tend;   //!
   TBranch        *b_hit_trkid;   //!
   TBranch        *b_hit_dQds;   //!
   TBranch        *b_hit_dEds;   //!
   TBranch        *b_hit_ds;   //!
   TBranch        *b_hit_resrange;   //!
   TBranch        *b_hit_x;   //!
   TBranch        *b_hit_y;   //!
   TBranch        *b_hit_z;   //!
   TBranch        *b_beamline_mass;   //!
   TBranch        *b_nwctrks;   //!
   TBranch        *b_wctrk_XFaceCoor;   //!
   TBranch        *b_wctrk_YFaceCoor;   //!
   TBranch        *b_wctrk_theta;   //!
   TBranch        *b_wctrk_phi;   //!
   TBranch        *b_wctrk_momentum;   //!
   TBranch        *b_wctrk_Px;   //!
   TBranch        *b_wctrk_Py;   //!
   TBranch        *b_wctrk_Pz;   //!
   TBranch        *b_wctrk_residual;   //!
   TBranch        *b_wctrk_wcmissed;   //!
   TBranch        *b_wctrk_picky;   //!
   TBranch        *b_wctrk_WC1XMult;   //!
   TBranch        *b_wctrk_WC1YMult;   //!
   TBranch        *b_wctrk_WC2XMult;   //!
   TBranch        *b_wctrk_WC2YMult;   //!
   TBranch        *b_wctrk_WC3XMult;   //!
   TBranch        *b_wctrk_WC3YMult;   //!
   TBranch        *b_wctrk_WC4XMult;   //!
   TBranch        *b_wctrk_WC4YMult;   //!
   TBranch        *b_wctrk_XDist;   //!
   TBranch        *b_wctrk_YDist;   //!
   TBranch        *b_wctrk_ZDist;   //!
   TBranch        *b_wctrk_YKink;   //!
   TBranch        *b_ntof;   //!
   TBranch        *b_tof;   //!
   TBranch        *b_tof_timestamp;   //!
   TBranch        *b_maxTrackIDE;   //!
   TBranch        *b_IDEEnergy;   //!
   TBranch        *b_IDEPos;   //!
   TBranch        *b_no_primaries;   //!
   TBranch        *b_geant_list_size;   //!
   TBranch        *b_pdg;   //!
   TBranch        *b_Mass;   //!
   TBranch        *b_Eng;   //!
   TBranch        *b_Px;   //!
   TBranch        *b_Py;   //!
   TBranch        *b_Pz;   //!
   TBranch        *b_EndEng;   //!
   TBranch        *b_EndPx;   //!
   TBranch        *b_EndPy;   //!
   TBranch        *b_EndPz;   //!
   TBranch        *b_StartPointx;   //!
   TBranch        *b_StartPointy;   //!
   TBranch        *b_StartPointz;   //!
   TBranch        *b_EndPointx;   //!
   TBranch        *b_EndPointy;   //!
   TBranch        *b_EndPointz;   //!
   TBranch        *b_StartT;   //!
   TBranch        *b_EndT;   //!
   TBranch        *b_PathLenInTpcAV;   //!
   TBranch        *b_StartInTpcAV;   //!
   TBranch        *b_EndInTpcAV;   //!
   TBranch        *b_Process;   //!
   TBranch        *b_NumberDaughters;   //!
   TBranch        *b_Mother;   //!
   TBranch        *b_TrackId;   //!
   TBranch        *b_process_primary;   //!
   TBranch        *b_G4Process;   //!
   TBranch        *b_G4FinalProcess;   //!

   preselection(TTree *tree=0);
   virtual ~preselection();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual bool     Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef preselection_cxx
preselection::preselection(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("MergedAnaTree_Run2_Beam_Pos100A_2022_01.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("MergedAnaTree_Run2_Beam_Pos100A_2022_01.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("MergedAnaTree_Run2_Beam_Pos100A_2022_01.root:/anatree");
      dir->GetObject("anatree",tree);

   }
   Init(tree);
}

preselection::~preselection()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t preselection::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t preselection::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void preselection::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   G4Process = 0;
   G4FinalProcess = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("run", &run, &b_run);
   fChain->SetBranchAddress("subrun", &subrun, &b_subrun);
   fChain->SetBranchAddress("event", &event, &b_event);
   fChain->SetBranchAddress("evttime", &evttime, &b_evttime);
   fChain->SetBranchAddress("efield", &efield, &b_efield);
   fChain->SetBranchAddress("lifetime", &lifetime, &b_lifetime);
   fChain->SetBranchAddress("t0", &t0, &b_t0);
   fChain->SetBranchAddress("nclus", &nclus, &b_nclus);
   fChain->SetBranchAddress("clustertwire", clustertwire, &b_clustertwire);
   fChain->SetBranchAddress("clusterttick", clusterttick, &b_clusterttick);
   fChain->SetBranchAddress("cluendwire", cluendwire, &b_cluendwire);
   fChain->SetBranchAddress("cluendtick", cluendtick, &b_cluendtick);
   fChain->SetBranchAddress("cluplane", cluplane, &b_cluplane);
   fChain->SetBranchAddress("ntracks_reco", &ntracks_reco, &b_ntracks_reco);
   fChain->SetBranchAddress("trkvtxx", trkvtxx, &b_trkvtxx);
   fChain->SetBranchAddress("trkvtxy", trkvtxy, &b_trkvtxy);
   fChain->SetBranchAddress("trkvtxz", trkvtxz, &b_trkvtxz);
   fChain->SetBranchAddress("trkendx", trkendx, &b_trkendx);
   fChain->SetBranchAddress("trkendy", trkendy, &b_trkendy);
   fChain->SetBranchAddress("trkendz", trkendz, &b_trkendz);
   fChain->SetBranchAddress("trkstartdcosx", trkstartdcosx, &b_trkstartdcosx);
   fChain->SetBranchAddress("trkstartdcosy", trkstartdcosy, &b_trkstartdcosy);
   fChain->SetBranchAddress("trkstartdcosz", trkstartdcosz, &b_trkstartdcosz);
   fChain->SetBranchAddress("trkenddcosx", trkenddcosx, &b_trkenddcosx);
   fChain->SetBranchAddress("trkenddcosy", trkenddcosy, &b_trkenddcosy);
   fChain->SetBranchAddress("trkenddcosz", trkenddcosz, &b_trkenddcosz);
   fChain->SetBranchAddress("trkWCtoTPCMatch", trkWCtoTPCMatch, &b_trkWCtoTPCMatch);
   fChain->SetBranchAddress("trklength", trklength, &b_trklength);
   fChain->SetBranchAddress("trkg4id", trkg4id, &b_trkg4id);
   fChain->SetBranchAddress("primarytrkkey", &primarytrkkey, &b_primarytrkkey);
   fChain->SetBranchAddress("ntrkcalopts", ntrkcalopts, &b_ntrkcalopts);
   fChain->SetBranchAddress("trkpida", trkpida, &b_trkpida);
   fChain->SetBranchAddress("trkke", trkke, &b_trkke);
   fChain->SetBranchAddress("trkdedx", trkdedx, &b_trkdedx);
   fChain->SetBranchAddress("trkdqdx", trkdqdx, &b_trkdqdx);
   fChain->SetBranchAddress("trkrr", trkrr, &b_trkrr);
   fChain->SetBranchAddress("trkpitch", trkpitch, &b_trkpitch);
   fChain->SetBranchAddress("trkxyz", trkxyz, &b_trkxyz);
   fChain->SetBranchAddress("nhits", &nhits, &b_nhits);
   fChain->SetBranchAddress("hit_plane", hit_plane, &b_hit_plane);
   fChain->SetBranchAddress("hit_wire", hit_wire, &b_hit_wire);
   fChain->SetBranchAddress("hit_channel", hit_channel, &b_hit_channel);
   fChain->SetBranchAddress("hit_peakT", hit_peakT, &b_hit_peakT);
   fChain->SetBranchAddress("hit_driftT", hit_driftT, &b_hit_driftT);
   fChain->SetBranchAddress("hit_charge", hit_charge, &b_hit_charge);
   fChain->SetBranchAddress("hit_electrons", hit_electrons, &b_hit_electrons);
   fChain->SetBranchAddress("hit_ph", hit_ph, &b_hit_ph);
   fChain->SetBranchAddress("hit_rms", hit_rms, &b_hit_rms);
   fChain->SetBranchAddress("hit_g4id", hit_g4id, &b_hit_g4id);
   fChain->SetBranchAddress("hit_g4frac", hit_g4frac, &b_hit_g4frac);
   fChain->SetBranchAddress("hit_g4nelec", hit_g4nelec, &b_hit_g4nelec);
   fChain->SetBranchAddress("hit_g4energy", hit_g4energy, &b_hit_g4energy);
   fChain->SetBranchAddress("hit_tstart", hit_tstart, &b_hit_tstart);
   fChain->SetBranchAddress("hit_tend", hit_tend, &b_hit_tend);
   fChain->SetBranchAddress("hit_trkid", hit_trkid, &b_hit_trkid);
   fChain->SetBranchAddress("hit_dQds", hit_dQds, &b_hit_dQds);
   fChain->SetBranchAddress("hit_dEds", hit_dEds, &b_hit_dEds);
   fChain->SetBranchAddress("hit_ds", hit_ds, &b_hit_ds);
   fChain->SetBranchAddress("hit_resrange", hit_resrange, &b_hit_resrange);
   fChain->SetBranchAddress("hit_x", hit_x, &b_hit_x);
   fChain->SetBranchAddress("hit_y", hit_y, &b_hit_y);
   fChain->SetBranchAddress("hit_z", hit_z, &b_hit_z);
   fChain->SetBranchAddress("beamline_mass", &beamline_mass, &b_beamline_mass);
   fChain->SetBranchAddress("nwctrks", &nwctrks, &b_nwctrks);
   fChain->SetBranchAddress("wctrk_XFaceCoor", wctrk_XFaceCoor, &b_wctrk_XFaceCoor);
   fChain->SetBranchAddress("wctrk_YFaceCoor", wctrk_YFaceCoor, &b_wctrk_YFaceCoor);
   fChain->SetBranchAddress("wctrk_theta", wctrk_theta, &b_wctrk_theta);
   fChain->SetBranchAddress("wctrk_phi", wctrk_phi, &b_wctrk_phi);
   fChain->SetBranchAddress("wctrk_momentum", wctrk_momentum, &b_wctrk_momentum);
   fChain->SetBranchAddress("wctrk_Px", wctrk_Px, &b_wctrk_Px);
   fChain->SetBranchAddress("wctrk_Py", wctrk_Py, &b_wctrk_Py);
   fChain->SetBranchAddress("wctrk_Pz", wctrk_Pz, &b_wctrk_Pz);
   fChain->SetBranchAddress("wctrk_residual", wctrk_residual, &b_wctrk_residual);
   fChain->SetBranchAddress("wctrk_wcmissed", wctrk_wcmissed, &b_wctrk_wcmissed);
   fChain->SetBranchAddress("wctrk_picky", wctrk_picky, &b_wctrk_picky);
   fChain->SetBranchAddress("wctrk_WC1XMult", wctrk_WC1XMult, &b_wctrk_WC1XMult);
   fChain->SetBranchAddress("wctrk_WC1YMult", wctrk_WC1YMult, &b_wctrk_WC1YMult);
   fChain->SetBranchAddress("wctrk_WC2XMult", wctrk_WC2XMult, &b_wctrk_WC2XMult);
   fChain->SetBranchAddress("wctrk_WC2YMult", wctrk_WC2YMult, &b_wctrk_WC2YMult);
   fChain->SetBranchAddress("wctrk_WC3XMult", wctrk_WC3XMult, &b_wctrk_WC3XMult);
   fChain->SetBranchAddress("wctrk_WC3YMult", wctrk_WC3YMult, &b_wctrk_WC3YMult);
   fChain->SetBranchAddress("wctrk_WC4XMult", wctrk_WC4XMult, &b_wctrk_WC4XMult);
   fChain->SetBranchAddress("wctrk_WC4YMult", wctrk_WC4YMult, &b_wctrk_WC4YMult);
   fChain->SetBranchAddress("wctrk_XDist", wctrk_XDist, &b_wctrk_XDist);
   fChain->SetBranchAddress("wctrk_YDist", wctrk_YDist, &b_wctrk_YDist);
   fChain->SetBranchAddress("wctrk_ZDist", wctrk_ZDist, &b_wctrk_ZDist);
   fChain->SetBranchAddress("wctrk_YKink", wctrk_YKink, &b_wctrk_YKink);
   fChain->SetBranchAddress("ntof", &ntof, &b_ntof);
   fChain->SetBranchAddress("tof", tof, &b_tof);
   fChain->SetBranchAddress("tof_timestamp", tof_timestamp, &b_tof_timestamp);
   fChain->SetBranchAddress("maxTrackIDE", &maxTrackIDE, &b_maxTrackIDE);
   fChain->SetBranchAddress("IDEEnergy", &IDEEnergy, &b_IDEEnergy);
   fChain->SetBranchAddress("IDEPos", &IDEPos, &b_IDEPos);
   fChain->SetBranchAddress("no_primaries", &no_primaries, &b_no_primaries);
   fChain->SetBranchAddress("geant_list_size", &geant_list_size, &b_geant_list_size);
   fChain->SetBranchAddress("pdg", &pdg, &b_pdg);
   fChain->SetBranchAddress("Mass", &Mass, &b_Mass);
   fChain->SetBranchAddress("Eng", &Eng, &b_Eng);
   fChain->SetBranchAddress("Px", &Px, &b_Px);
   fChain->SetBranchAddress("Py", &Py, &b_Py);
   fChain->SetBranchAddress("Pz", &Pz, &b_Pz);
   fChain->SetBranchAddress("EndEng", &EndEng, &b_EndEng);
   fChain->SetBranchAddress("EndPx", &EndPx, &b_EndPx);
   fChain->SetBranchAddress("EndPy", &EndPy, &b_EndPy);
   fChain->SetBranchAddress("EndPz", &EndPz, &b_EndPz);
   fChain->SetBranchAddress("StartPointx", &StartPointx, &b_StartPointx);
   fChain->SetBranchAddress("StartPointy", &StartPointy, &b_StartPointy);
   fChain->SetBranchAddress("StartPointz", &StartPointz, &b_StartPointz);
   fChain->SetBranchAddress("EndPointx", &EndPointx, &b_EndPointx);
   fChain->SetBranchAddress("EndPointy", &EndPointy, &b_EndPointy);
   fChain->SetBranchAddress("EndPointz", &EndPointz, &b_EndPointz);
   fChain->SetBranchAddress("StartT", &StartT, &b_StartT);
   fChain->SetBranchAddress("EndT", &EndT, &b_EndT);
   fChain->SetBranchAddress("PathLenInTpcAV", &PathLenInTpcAV, &b_PathLenInTpcAV);
   fChain->SetBranchAddress("StartInTpcAV", &StartInTpcAV, &b_StartInTpcAV);
   fChain->SetBranchAddress("EndInTpcAV", &EndInTpcAV, &b_EndInTpcAV);
   fChain->SetBranchAddress("Process", &Process, &b_Process);
   fChain->SetBranchAddress("NumberDaughters", &NumberDaughters, &b_NumberDaughters);
   fChain->SetBranchAddress("Mother", &Mother, &b_Mother);
   fChain->SetBranchAddress("TrackId", &TrackId, &b_TrackId);
   fChain->SetBranchAddress("process_primary", &process_primary, &b_process_primary);
   fChain->SetBranchAddress("G4Process", &G4Process, &b_G4Process);
   fChain->SetBranchAddress("G4FinalProcess", &G4FinalProcess, &b_G4FinalProcess);
   Notify();
}

bool preselection::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return true;
}

void preselection::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t preselection::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef preselection_cxx
