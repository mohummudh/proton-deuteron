import duckdb

# Set memory limit in DuckDB
duckdb.sql("SET memory_limit='1GB';")  # Adjust to available RAM

# Convert CSV to Parquet using streaming (limits RAM usage)
duckdb.sql("""
    COPY (
        SELECT * FROM read_csv_auto('/Users/mohammed/code/proton-deuterium-data/data/csv/all_events.csv', all_varchar=True)
    ) TO '/Users/mohammed/code/proton-deuterium-data/data/all_events.parquet' (FORMAT PARQUET);
""")
