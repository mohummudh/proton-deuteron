# search_worker.py

import uproot

def process_single_file(file_path, target_events_set):
    """
    Processes a single ROOT file to find matching events.
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