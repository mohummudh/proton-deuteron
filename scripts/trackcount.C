#define trackcount_cxx
#include "trackcount.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <fstream>
#include <iostream>

void trackcount::Loop() {
    // Check if the TChain is valid
    if (!fChain) {
        std::cerr << "Error: fChain is null." << std::endl;
        return;
    }

    // Enable only the 4 required branches
    fChain->SetBranchStatus("*", 0); // Disable all branches first
    fChain->SetBranchStatus("run", 1);
    fChain->SetBranchStatus("subrun", 1);
    fChain->SetBranchStatus("event", 1);
    fChain->SetBranchStatus("ntracks_reco", 1);

    // Open CSV file for writing
    FILE *fout = fopen("output.csv", "w");
    if (!fout) {
        std::cerr << "Error: Could not open CSV file for writing." << std::endl;
        return;
    }
    
    // Write the CSV header
    fprintf(fout, "run,subrun,event,ntracks_reco\n");

    // Loop through all entries
    Long64_t nentries = fChain->GetEntriesFast();
    std::cout << "Processing " << nentries << " entries..." << std::endl;
    for (Long64_t jentry = 0; jentry < nentries; jentry++) {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;

        fChain->GetEntry(jentry);

        // Optional: Print debug info to the console
        std::cout << "Entry " << jentry << " - run: " << run << ", subrun: " << subrun
                  << ", event: " << event << ", ntracks_reco: " << ntracks_reco << std::endl;

        // Write the variables to the CSV file
        fprintf(fout, "%d,%d,%d,%d\n", run, subrun, event, ntracks_reco);
    }

    fclose(fout);
    std::cout << "Data successfully written to output.csv" << std::endl;
}