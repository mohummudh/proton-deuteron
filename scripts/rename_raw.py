import uproot
import shutil
import os
from pathlib import Path

def process_raw_digit_files(source_folder, dest_folder):
    """
    Process ROOT files created by RawDigitExtractor
    Tree name: "ana/raw"
    Variables: "run", "subrun", "event"
    """
    source_path = Path(source_folder)
    dest_path = Path(dest_folder)
    
    # Create destination folder
    dest_path.mkdir(parents=True, exist_ok=True)
    
    # Process each ROOT file
    for root_file in source_path.glob("*.root"):
        try:
            # Open ROOT file and read "raw" tree
            with uproot.open(root_file) as file:
                raw_tree = file["ana/raw"]
                
                # Get run, subrun, event from first entry
                run = int(raw_tree["run"].array()[0])
                subrun = int(raw_tree["subrun"].array()[0])
                event = int(raw_tree["event"].array()[0])
                
                # Create new filename: run_subrun_event.root
                new_filename = f"{run}_{subrun}_{event}.root"
                dest_file = dest_path / new_filename
                
                # Handle duplicates by adding counter
                counter = 1
                while dest_file.exists():
                    new_filename = f"{run}_{subrun}_{event}_{counter}.root"
                    dest_file = dest_path / new_filename
                    counter += 1
                
                # Copy file with new name
                shutil.copy2(root_file, dest_file)
                print(f"✓ {root_file.name} → {new_filename}")
                
        except Exception as e:
            print(f"✗ Error processing {root_file.name}: {e}")

# Usage
process_raw_digit_files(
    "/Users/user/data/research/proton-deuteron/rawprotons/proton_extracted_root",
    "/Users/user/data/research/proton-deuteron/rawprotons/onetrackprotons"
)