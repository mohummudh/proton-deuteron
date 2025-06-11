import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import tkinter as tk
from tkinter import messagebox, Button, Label, Entry, filedialog, StringVar, Radiobutton
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import uproot
import awkward as ak

class Evd_display():
    def __init__(self):
        self.all_events_df = None
        self.induction_plane = None
        self.collection_plane = None
        self.loaded_file = ""

    def load_all_events_from_root(self, file_path):
        """
        Loads data for ALL events, including the crucial 'raw_channel' branch.
        """
        try:
            self.loaded_file = file_path.split('/')[-1]
            root_file = uproot.open(file_path)
            tree_name = "ana/raw"
            if tree_name not in root_file:
                messagebox.showerror("Error", f"TTree '{tree_name}' not found.")
                return False
            tree = root_file[tree_name]
            
            # --- ✔️ MUST load raw_channel to map the wires correctly ---
            branches_to_load = ["raw_rawadc", "raw_channel"]
            self.all_events_df = tree.arrays(branches_to_load, library="pd")
            
            if self.all_events_df.empty:
                 self.all_events_df = None
                 return False
            return True
        except Exception as e:
            self.all_events_df = None
            messagebox.showerror("Error", f"Failed to load .root file: {e}")
            return False

    def select_event(self, event_index):
        """
        Selects an event and correctly maps ADC data to the two planes
        using the 'raw_channel' branch.
        """
        if self.all_events_df is None: return False
        try:
            event_data = self.all_events_df.iloc[event_index]
            
            # --- ✔️ The definitive, correct mapping logic ---
            adc_data = ak.to_numpy(event_data["raw_rawadc"])
            channel_map = ak.to_numpy(event_data["raw_channel"])

            total_wires = 480
            num_ticks = adc_data.size // total_wires

            # Reshape the ADC data assuming the order matches the channel map
            adc_data_2d = adc_data.reshape((total_wires, num_ticks))

            # Create empty arrays to hold the correctly sorted plane data
            self.induction_plane = np.zeros((240, num_ticks))
            self.collection_plane = np.zeros((240, num_ticks))

            # Loop through the channel map to sort the data
            for i, channel_num in enumerate(channel_map):
                wire_data = adc_data_2d[i, :]
                if 0 <= channel_num < 240:
                    # It's an Induction wire
                    wire_index = channel_num
                    self.induction_plane[wire_index, :] = wire_data
                elif 240 <= channel_num < 480:
                    # It's a Collection wire
                    wire_index = channel_num - 240
                    self.collection_plane[wire_index, :] = wire_data
            
            return True
        except Exception as e:
            messagebox.showerror("Error", f"Failed to process event {event_index}: {e}")
            return False


class myGUI():
    def __init__(self, root):
        self.root = root
        self.root.title("LArIAT Event Display")
        self.ev_dis = Evd_display()

        self.event_var = StringVar(value="0")
        self.wire_var = StringVar(value="0")
        self.status_var = StringVar(value="No file loaded.")
        self.plane_var = StringVar(value="Collection")

        Button(root, text="Open .root Event File", command=self.load_event_from_root).grid(row=0, column=0, columnspan=2, pady=5, padx=5, sticky="ew")
        Label(root, textvariable=self.status_var, fg="blue").grid(row=1, column=0, columnspan=2, padx=5)
        Label(root, text="Select Event:").grid(row=2, column=0, sticky="e", padx=5)
        Entry(root, textvariable=self.event_var).grid(row=2, column=1, sticky="w")
        Radiobutton(root, text="Collection Plane", variable=self.plane_var, value="Collection").grid(row=3, column=0, sticky="e", padx=10)
        Radiobutton(root, text="Induction Plane", variable=self.plane_var, value="Induction").grid(row=3, column=1, sticky="w")
        Button(root, text="Display Heatmap", command=self.display_heatmap).grid(row=4, column=0, columnspan=2, pady=5, padx=5, sticky="ew")
        Label(root, text="Select Wire (0-239):").grid(row=5, column=0, sticky="e", padx=5)
        Entry(root, textvariable=self.wire_var).grid(row=5, column=1, sticky="w")
        Button(root, text="Display Wire", command=self.display_wire).grid(row=6, column=0, columnspan=2, pady=5, padx=5, sticky="ew")

    def load_event_from_root(self):
        filename = filedialog.askopenfilename(title="Select a .root File", filetypes=(("ROOT files", "*.root"),))
        if not filename: return
        self.status_var.set("Loading...")
        self.root.update_idletasks()
        if self.ev_dis.load_all_events_from_root(filename):
            self.status_var.set(f"Loaded {len(self.ev_dis.all_events_df)} events from {self.ev_dis.loaded_file.split('/')[-1]}")
        else:
            self.status_var.set("Failed to load file.")

    def _prepare_display(self):
        if self.ev_dis.all_events_df is None:
            messagebox.showerror("Error", "No file loaded.")
            return None
        try:
            event_num = int(self.event_var.get())
            if not self.ev_dis.select_event(event_num):
                return None
            selected_plane_name = self.plane_var.get()
            if selected_plane_name == "Collection":
                return self.ev_dis.collection_plane
            else:
                return self.ev_dis.induction_plane
        except ValueError:
            messagebox.showerror("Error", "Please enter a valid event number.")
            return None

    def display_heatmap(self):
        plane_to_display = self._prepare_display()
        if plane_to_display is None: return
        plane_name = self.plane_var.get()
        heatmap_win = tk.Toplevel(self.root)
        heatmap_win.title(f"{plane_name} Plane, Event {self.event_var.get()}")
        fig = Figure(figsize=(12, 7))
        ax = fig.add_subplot(111)
        sns.heatmap(plane_to_display.T, cmap="viridis", ax=ax, cbar=True)
        ax.invert_yaxis()
        ax.set_xlabel("Wire (0-239)")
        ax.set_ylabel("Tick")
        ax.set_title(f"{plane_name} Heatmap for Event {self.event_var.get()}")
        canvas = FigureCanvasTkAgg(fig, master=heatmap_win)
        canvas.draw()
        canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def display_wire(self):
        plane_to_display = self._prepare_display()
        if plane_to_display is None: return
        try:
            wire_num = int(self.wire_var.get())
            if not 0 <= wire_num < 240:
                return messagebox.showerror("Error", "Wire number must be between 0 and 239.")
        except ValueError:
            return messagebox.showerror("Error", "Please enter a valid wire number.")
        plane_name = self.plane_var.get()
        wire_win = tk.Toplevel(self.root)
        wire_win.title(f"{plane_name}, Event {self.event_var.get()}, Wire {wire_num}")
        fig = Figure(figsize=(14, 4))
        ax = fig.add_subplot(111)
        wire_data = plane_to_display[wire_num, :]
        sns.lineplot(x=range(len(wire_data)), y=wire_data, ax=ax)
        ax.set_xlabel("Tick")
        ax.set_ylabel("ADC Value")
        ax.set_title(f"Data for Wire {wire_num}")
        canvas = FigureCanvasTkAgg(fig, master=wire_win)
        canvas.draw()
        canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

if __name__ == "__main__":
    root = tk.Tk()
    gui = myGUI(root)
    root.mainloop()