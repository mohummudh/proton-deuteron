from dask.diagnostics import ProgressBar
import dask_cudf
import gc

ProgressBar().register()

dataset_path = "run.csv"
ddf = dask_cudf.read_csv(dataset_path)

ddf.columns = ddf.columns.str.strip()

grouped_ddf = ddf.groupby(["beamline_mass", "wctrkmomentum", "tof", "trklength", "trkendx", "trkendy", "trkendz", "ntrkcalopts"]).agg({
    "run": "first",
    "subrun": "first",
    "eventtype": "first",
    "incint": "list",
    "negativeke": "first",
    "positionxtpc": "list",
    "positionytpc": "list",
    "positionztpc": "list",
    "decayatrest": "first",
    "residualrange": "list",
    "trkdedx": "list",
    "nslice": "list",
    "cke": "list",
}).reset_index()

import rmm
gc.collect()
rmm.reinitialize()

df_cpu = grouped_ddf.compute().to_pandas()

output_parquet = "grouped_data.parquet"
df_cpu.to_parquet(output_parquet, engine="pyarrow")