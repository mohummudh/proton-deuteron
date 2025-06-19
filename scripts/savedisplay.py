#!/usr/bin/env python3
"""
Event Display Generator

This script takes a CSV file containing (run, subrun, event) and searches
through ROOT files to generate event displays for each matching event.

Usage:
    python savedisplay.py <csv_file> <root_files_dir> [output_dir]

Arguments:
    csv_file: Path to CSV file with columns 'run', 'subrun', 'event'
    root_files_dir: Directory containing ROOT files to search
    output_dir: Optional output directory for images (default: 'event_images')
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import uproot
import awkward as ak
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed
from tqdm import tqdm
import argparse
import sys

def process_single_file(file_path, target_events_set):
    """
    Process a single ROOT file to find matching events
    """
    matches = []
    try:
        with uproot.open(file_path) as root_file:
            if "ana/raw" not in root_file:
                return matches
            
            tree = root_file["ana/raw"]
            events_data = tree.arrays(["run", "subrun", "event"], library="np")
            
            runs = events_data["run"].astype(int)
            subruns = events_data["subrun"].astype(int)
            events = events_data["event"].astype(int)
            
            for idx in range(len(runs)):
                event_tuple = (runs[idx], subruns[idx], events[idx])
                if event_tuple in target_events_set:
                    matches.append({
                        'run': runs[idx],
                        'subrun': subruns[idx],
                        'event': events[idx],
                        'file_path': str(file_path),
                        'filename': file_path.name,
                        'event_index_in_file': idx,
                    })
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
    
    return matches

class EventDisplayGenerator():
    """
    An optimized event display generator that uses parallel processing to quickly
    search through ROOT files and generate event displays.
    """
    def __init__(self, events_csv, root_files_dir, output_dir="event_images"):
        self.events_df = pd.read_csv(events_csv)
        self.root_files_dir = Path(root_files_dir)
        self.output_dir = Path(output_dir)
        self.matched_events = []
        
        # Validate input
        required_columns = ['run', 'subrun', 'event']
        if not all(col in self.events_df.columns for col in required_columns):
            raise ValueError(f"CSV must contain columns: {required_columns}")
        
        print(f"Loaded {len(self.events_df)} events from {events_csv}")
        print(f"ROOT files directory: {root_files_dir}")
        print(f"Output directory: {output_dir}")

    def search_and_generate_all(self, max_workers=None):
        """
        Complete pipeline: search files and generate all event displays
        """
        print("\n" + "="*60)
        print("STARTING EVENT DISPLAY GENERATION PIPELINE")
        print("="*60)
        
        # Step 1: Search for matching events
        self._search_files(max_workers)
        
        if not self.matched_events:
            print("No matching events found. Exiting.")
            return
        
        # Step 2: Generate all displays
        self._generate_all_displays()
        
        print("\n" + "="*60)
        print("PIPELINE COMPLETE")
        print("="*60)

    def _search_files(self, max_workers=None):
        """
        Search all ROOT files in parallel for events that match the CSV.
        """
        print("\nStep 1: Searching for matching events...")
        target_events_set = set(zip(
            self.events_df['run'].astype(int),
            self.events_df['subrun'].astype(int),
            self.events_df['event'].astype(int)
        ))

        root_files = list(self.root_files_dir.glob("*.root"))
        print(f"Found {len(root_files)} ROOT files to search")
        print(f"Looking for {len(target_events_set)} target events")

        matched = []
        with ProcessPoolExecutor(max_workers=max_workers) as executor:
            futures = [executor.submit(process_single_file, path, target_events_set) for path in root_files]
            
            for future in tqdm(as_completed(futures), total=len(root_files), desc="Searching Files"):
                result = future.result()
                if result:
                    matched.extend(result)
        
        self.matched_events = sorted(matched, key=lambda x: (x['run'], x['subrun'], x['event']))
        print(f"Found {len(self.matched_events)} matching events")
        return self.matched_events

    def _get_event_image_data(self, match_index):
        """
        Internal helper to fetch and process data for plotting using the correct
        physical mapping of channels to a fixed canvas.
        """
        if not (0 <= match_index < len(self.matched_events)):
            print(f"Match index {match_index} out of range.")
            return None, None

        event_info = self.matched_events[match_index]
        try:
            with uproot.open(event_info['file_path']) as root_file:
                tree = root_file["ana/raw"]
                event_start = event_info['event_index_in_file']
                event_data = tree.arrays(
                    ["raw_rawadc", "raw_channel"],
                    entry_start=event_start, entry_stop=event_start + 1, library="ak"
                )
                
                if len(event_data) == 0:
                    print(f"Warning: No data found in file for event at index {match_index}.")
                    return None, None

                # --- CORRECTED LOGIC ADOPTED FROM YOUR EXAMPLE ---
                # Use ak.to_numpy for explicit conversion
                adc_data = ak.to_numpy(event_data["raw_rawadc"][0])
                channel_map = ak.to_numpy(event_data["raw_channel"][0])
                
                # Reshape based on the total number of channels present in the data
                num_channels_in_event = len(channel_map)
                if num_channels_in_event == 0:
                    print(f"Warning: Event at index {match_index} has no channel data.")
                    return None, None
                
                num_ticks = len(adc_data) // num_channels_in_event
                adc_data_2d = adc_data.reshape((num_channels_in_event, num_ticks))

                # Create a fixed-size canvas for the collection plane (240 wires)
                collection_plane = np.zeros((240, num_ticks))

                # Populate the canvas at the correct physical wire location
                for i, channel_num in enumerate(channel_map):
                    if 240 <= channel_num < 480:
                        wire_index = channel_num - 240 # Map channel 240 to index 0
                        collection_plane[wire_index, :] = adc_data_2d[i, :]
                
                title = f"Collection Plane - Run {event_info['run']}, Subrun {event_info['subrun']}, Event {event_info['event']}"
                
                # Transpose for plotting (Time vs. Wire)
                return collection_plane.T, title
                
        except Exception as e:
            print(f"Error processing data for event at index {match_index}: {e}")
            return None, None

    def show_event(self, match_index=0):
        """Shows a single event display (for testing/debugging)"""
        if not self.matched_events:
            print("No matches found. Run search_and_generate_all() first.")
            return
            
        image_data, title = self._get_event_image_data(match_index)
        
        if image_data is None:
            print(f"Could not generate plot for event at index {match_index}.")
            return
        
        print(f"Displaying: {title}")
        plt.figure(figsize=(12, 6))
        sns.heatmap(image_data, cmap="viridis", cbar_kws={'label': 'ADC Counts'})
        plt.title(title)
        plt.xlabel("Collection Plane Wire Number (0-239)")
        plt.ylabel("Time Tick")
        plt.gca().invert_yaxis()
        plt.tight_layout()
        plt.show()

    def _generate_all_displays(self):
        """
        Generate and save event displays for all matched events.
        """
        print(f"\nStep 2: Generating {len(self.matched_events)} event displays...")
        
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Use matplotlib non-interactive backend for batch processing
        plt.ioff()
        
        successful = 0
        for i, event_info in enumerate(tqdm(self.matched_events, desc="Generating Images")):
            filename = f"run_{event_info['run']}_subrun_{event_info['subrun']}_event_{event_info['event']}.png"
            full_path = self.output_dir / filename

            if full_path.exists():
                successful += 1
                continue  # Skip if already exists

            image_data, title = self._get_event_image_data(i)
            
            if image_data is None:
                continue

            try:
                fig, ax = plt.subplots(figsize=(12, 6))
                sns.heatmap(image_data, cmap="viridis", cbar_kws={'label': 'ADC Counts'}, ax=ax)
                ax.set_title(title, fontsize=14)
                ax.set_xlabel("Collection Plane Wire Number (0-239)", fontsize=12)
                ax.set_ylabel("Time Tick", fontsize=12)
                ax.invert_yaxis()
                
                plt.tight_layout()
                plt.savefig(full_path, dpi=150, bbox_inches='tight')
                plt.close(fig)
                successful += 1
                
            except Exception as e:
                print(f"\nError saving {filename}: {e}")
                plt.close('all')  # Clean up in case of error

        print(f"\nSuccessfully generated {successful}/{len(self.matched_events)} event displays")
        print(f"Images saved to: {self.output_dir.resolve()}")

    def list_matches(self):
        """List all matched events"""
        if not self.matched_events:
            print("No matches found. Run search_and_generate_all() first.")
            return

        print("\nMatched events found:")
        for i, event in enumerate(self.matched_events):
            print(f"{i}: Run {event['run']}, Subrun {event['subrun']}, Event {event['event']} (from {event['filename']})")
        print(f"\nTotal: {len(self.matched_events)} matches")


def main():
    """Main function to handle command line arguments"""
    parser = argparse.ArgumentParser(
        description="Generate event displays from ROOT files based on CSV input",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    python savedisplay.py events.csv /path/to/root/files
    python savedisplay.py events.csv /path/to/root/files my_output_dir
    python savedisplay.py events.csv /path/to/root/files --workers 8
        """
    )
    
    parser.add_argument('csv_file', help='CSV file with run, subrun, event columns')
    parser.add_argument('root_files_dir', help='Directory containing ROOT files')
    parser.add_argument('output_dir', nargs='?', default='event_images', 
                       help='Output directory for images (default: event_images)')
    parser.add_argument('--workers', type=int, help='Number of parallel workers')
    
    args = parser.parse_args()
    
    # Validate inputs
    csv_path = Path(args.csv_file)
    root_dir = Path(args.root_files_dir)
    
    if not csv_path.exists():
        print(f"Error: CSV file not found: {csv_path}")
        sys.exit(1)
    
    if not root_dir.exists():
        print(f"Error: ROOT files directory not found: {root_dir}")
        sys.exit(1)
    
    try:
        # Create generator and run pipeline
        generator = EventDisplayGenerator(
            events_csv=args.csv_file,
            root_files_dir=args.root_files_dir,
            output_dir=args.output_dir
        )
        
        generator.search_and_generate_all(max_workers=args.workers)
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()