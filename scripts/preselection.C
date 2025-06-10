#define preselection_cxx
#include "preselection.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <fstream>
#include <iostream>

void preselection::Loop() {
   if (!fChain) {
      std::cerr << "Error: fChain is null." << std::endl;
      return;
   }

   // Enable only required branches
   fChain->SetBranchStatus("*", 0); // Disable all branches
   fChain->SetBranchStatus("run", 1);
   fChain->SetBranchStatus("subrun", 1);
   fChain->SetBranchStatus("event", 1);
   fChain->SetBranchStatus("wctrk_picky", 1);
   fChain->SetBranchStatus("trkWCtoTPCMatch", 1);
   fChain->SetBranchStatus("beamline_mass", 1);

   // Open CSV file for writing
   FILE *fout = fopen("output.csv", "w");
   if (!fout) {
      std::cerr << "Error: Could not open CSV file for writing." << std::endl;
      return;
   }
   fprintf(fout, "run,subrun,event,p,m,beamline_mass\n"); // Updated CSV header

   // Loop through all entries
   Long64_t nentries = fChain->GetEntriesFast();
   std::cout << "Processing " << nentries << " entries..." << std::endl;
   for (Long64_t jentry = 0; jentry < nentries; jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;

      fChain->GetEntry(jentry);

      // Assign binary values for "p" and "m"
      int p = (wctrk_picky[0] == 1) ? 1 : 0;
      int m = (trkWCtoTPCMatch[0] == 1) ? 1 : 0;


      // Print debug info before writing
      std::cout << "Entry " << jentry << " - run: " << run << ", subrun: " << subrun 
                << ", event: " << event << ", p: " << p << ", m: " << m 
                << ", beamline_mass: " << beamline_mass << std::endl;

      // Write to CSV file
      fprintf(fout, "%d,%d,%d,%d,%d,%.2f\n", run, subrun, event, p, m, beamline_mass);
   }

   fclose(fout);
   std::cout << "Data successfully written to output.csv" << std::endl;
}