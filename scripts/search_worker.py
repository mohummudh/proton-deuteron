# search_worker.py

import uproot
import os
from pathlib import Path
from multiprocessing import Pool
import glob

def count_events_in_file(file_path):
    """
    Counts the number of events in a single ROOT file.
    This function is designed to be run in a separate process.
    """
    try:
        with uproot.open(file_path) as file:
            if "ana/raw" not in file:
                return {
                    'file_path': str(file_path),
                    'filename': Path(file_path).name,
                    'event_count': 0,
                    'error': 'No ana/raw tree found'
                }

            tree = file["ana/raw"]
            event_count = tree.num_entries
            
            return {
                'file_path': str(file_path),
                'filename': Path(file_path).name,
                'event_count': event_count,
                'error': None
            }
        
    except Exception as e:
        return {
            'file_path': str(file_path),
            'filename': Path(file_path).name,
            'event_count': 0,
            'error': str(e)
        }

def count_total_events_in_directory(directory_path, pattern="*.root", num_processes=None):
    """
    Counts the total number of events in all ROOT files in a directory.
    
    Args:
        directory_path (str): Path to the directory containing ROOT files
        pattern (str): File pattern to match (default: "*.root")
        num_processes (int): Number of processes to use (default: None for auto)
    
    Returns:
        dict: Summary of event counts with detailed results
    """
    directory_path = Path(directory_path)
    
    if not directory_path.exists():
        raise ValueError(f"Directory does not exist: {directory_path}")
    
    # Find all ROOT files matching the pattern
    root_files = list(directory_path.glob(pattern))
    
    if not root_files:
        print(f"No files matching pattern '{pattern}' found in {directory_path}")
        return {
            'total_events': 0,
            'total_files': 0,
            'successful_files': 0,
            'failed_files': 0,
            'file_details': []
        }
    
    print(f"Found {len(root_files)} ROOT files to process...")
    
    # Process files in parallel
    with Pool(processes=num_processes) as pool:
        results = pool.map(count_events_in_file, root_files)
    
    # Analyze results
    total_events = 0
    successful_files = 0
    failed_files = 0
    
    for result in results:
        if result['error'] is None:
            total_events += result['event_count']
            successful_files += 1
        else:
            failed_files += 1
            print(f"Warning: {result['filename']}: {result['error']}")
    
    summary = {
        'total_events': total_events,
        'total_files': len(root_files),
        'successful_files': successful_files,
        'failed_files': failed_files,
        'file_details': results
    }
    
    return summary

def process_single_file(file_path, target_events_set):
    """
    Original function: Processes a single ROOT file to find matching events.
    This function is designed to be run in a separate process.
    """
    try:
        with uproot.open(file_path) as file:
            if "ana/raw" not in file:
                return []

            tree = file["ana/raw"]
            events_in_file = tree.arrays(["run", "subrun", "event"], library="ak")
            
            found_matches = []
            for idx, event_id in enumerate(zip(events_in_file.run, events_in_file.subrun, events_in_file.event)):
                if event_id in target_events_set:
                    found_matches.append({
                        'run': event_id[0],
                        'subrun': event_id[1],
                        'event': event_id[2],
                        'file_path': str(file_path),
                        'filename': file_path.name,
                        'event_index_in_file': idx,
                    })
            return found_matches
        
    except Exception as e:
        print(f"Warning: Could not process file {file_path.name}: {e}")
        return []

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="Count total events in ROOT files")
    parser.add_argument("directory", help="Directory containing ROOT files")
    parser.add_argument("--pattern", default="*.root", help="File pattern to match (default: *.root)")
    parser.add_argument("--processes", type=int, default=None, help="Number of processes to use")
    parser.add_argument("--verbose", "-v", action="store_true", help="Show detailed file information")
    
    args = parser.parse_args()
    
    try:
        print(f"Counting events in ROOT files in: {args.directory}")
        print(f"File pattern: {args.pattern}")
        print("-" * 50)
        
        results = count_total_events_in_directory(
            args.directory, 
            pattern=args.pattern, 
            num_processes=args.processes
        )
        
        print("\nSUMMARY:")
        print(f"Total events: {results['total_events']:,}")
        print(f"Total files processed: {results['total_files']}")
        print(f"Successful files: {results['successful_files']}")
        print(f"Failed files: {results['failed_files']}")
        
        if args.verbose and results['file_details']:
            print("\nDETAILED RESULTS:")
            print("-" * 50)
            for detail in results['file_details']:
                if detail['error'] is None:
                    print(f"{detail['filename']}: {detail['event_count']:,} events")
                else:
                    print(f"{detail['filename']}: ERROR - {detail['error']}")
                    
    except Exception as e:
        print(f"Error: {e}")
        exit(1)