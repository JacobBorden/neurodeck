#!/usr/bin/env python3
import sys
import re
import os # For path manipulation
from collections import defaultdict

def parse_lcov_output(lcov_output):
    coverage_data = defaultdict(lambda: {'lines_hit': 0, 'lines_found': 0, 'functions_hit': 0, 'functions_found': 0})
    current_file_abs_path = None
    for line in lcov_output.splitlines():
        if line.startswith('SF:'):
            # LCOV paths are usually absolute. If not, make them absolute from CWD.
            # For this project, they seem to be absolute from the root /app
            current_file_abs_path = os.path.abspath(line[3:])
        elif line.startswith('DA:'):
            # DA:line_number,hit_count[,checksum]
            parts = line[3:].split(',')
            # line_num = int(parts[0]) # Not strictly needed for line coverage percentage
            hit_count = int(parts[1].split(';')[0]) # Remove checksum if present
            if current_file_abs_path:
                if hit_count > 0:
                    coverage_data[current_file_abs_path]['lines_hit'] += 1
                coverage_data[current_file_abs_path]['lines_found'] += 1
        elif line.startswith('FNF:'): # Functions Found
            if current_file_abs_path:
                coverage_data[current_file_abs_path]['functions_found'] = int(line[4:])
        elif line.startswith('FNH:'): # Functions Hit
             if current_file_abs_path:
                coverage_data[current_file_abs_path]['functions_hit'] = int(line[4:])
        elif line == 'end_of_record':
            current_file_abs_path = None
    return coverage_data

def calculate_file_coverage(coverage_data, target_files_relative):
    # Convert target_files to absolute paths based on current working directory (project root)
    project_root = os.getcwd()
    target_files_abs = [os.path.join(project_root, f) for f in target_files_relative]

    file_coverage = defaultdict(lambda: {'lines_hit': 0, 'lines_found': 0, 'lines_percentage': 0.0,
                                         'functions_hit': 0, 'functions_found': 0, 'functions_percentage': 0.0})
    
    found_any_target_file_in_lcov = False

    for abs_target_file in target_files_abs:
        # Normalize paths for comparison (e.g., remove ../, ./)
        normalized_abs_target_file = os.path.normpath(abs_target_file)
        
        # Check if this specific file's data exists in LCOV output
        # LCOV paths are absolute in the coverage.info file
        if normalized_abs_target_file in coverage_data:
            found_any_target_file_in_lcov = True
            data = coverage_data[normalized_abs_target_file]
            file_coverage[abs_target_file]['lines_hit'] = data['lines_hit']
            file_coverage[abs_target_file]['lines_found'] = data['lines_found']
            if data['lines_found'] > 0:
                file_coverage[abs_target_file]['lines_percentage'] = (data['lines_hit'] / data['lines_found']) * 100
            else:
                file_coverage[abs_target_file]['lines_percentage'] = 100.0 # Or 0.0 if preferred for empty files
            
            file_coverage[abs_target_file]['functions_hit'] = data['functions_hit']
            file_coverage[abs_target_file]['functions_found'] = data['functions_found']
            if data['functions_found'] > 0:
                file_coverage[abs_target_file]['functions_percentage'] = (data['functions_hit'] / data['functions_found']) * 100
            else:
                file_coverage[abs_target_file]['functions_percentage'] = 100.0 # Or 0.0

    return file_coverage, found_any_target_file_in_lcov

def main():
    if len(sys.argv) < 4:
        print("Usage: python check_coverage.py <lcov_output_file> <min_percentage> <file1> [file2 ...]")
        sys.exit(1)

    lcov_file_path = sys.argv[1]
    min_percentage = float(sys.argv[2])
    # These are relative paths from the project root, e.g., "core/lua_manager.cpp"
    target_files_relative = sys.argv[3:]


    try:
        with open(lcov_file_path, 'r') as f:
            lcov_output = f.read()
    except FileNotFoundError:
        print(f"Error: LCOV output file not found at {lcov_file_path}")
        sys.exit(1)

    if not lcov_output.strip():
        print(f"Error: LCOV file {lcov_file_path} is empty.")
        sys.exit(1)

    parsed_data = parse_lcov_output(lcov_output)
    if not parsed_data:
        print(f"Error: No coverage data parsed from {lcov_file_path}. Is it a valid LCOV info file?")
        sys.exit(1)

    file_coverage_stats, found_any_target = calculate_file_coverage(parsed_data, target_files_relative)

    if not found_any_target:
        print(f"ERROR: None of the specified target files were found in the LCOV report: {', '.join(target_files_relative)}")
        print("LCOV SF entries are absolute. Parsed LCOV SF paths include:")
        for lcov_sf_path in parsed_data.keys():
            print(f"  - {lcov_sf_path}")
        print(f"Target files were resolved to absolute paths like: {os.path.abspath(target_files_relative[0]) if target_files_relative else 'N/A'}")
        sys.exit(1)

    all_met_threshold = True
    for file_path_rel, stats in file_coverage_stats.items():
        # Find the original relative path for display
        original_rel_path = ""
        abs_file_path_norm = os.path.normpath(file_path_rel) # This is already absolute in file_coverage_stats keys
        for rel_path_arg in target_files_relative:
            if os.path.normpath(os.path.abspath(rel_path_arg)) == abs_file_path_norm:
                original_rel_path = rel_path_arg
                break
        
        display_path = original_rel_path if original_rel_path else abs_file_path_norm

        print(f"Coverage for {display_path}:")
        print(f"  Lines: {stats['lines_percentage']:.2f}% ({stats['lines_hit']}/{stats['lines_found']})")
        print(f"  Functions: {stats['functions_percentage']:.2f}% ({stats['functions_hit']}/{stats['functions_found']})")
        
        # Check line coverage against threshold
        if stats['lines_percentage'] < min_percentage:
            print(f"  ERROR: Line coverage for {display_path} ({stats['lines_percentage']:.2f}%) is below the required {min_percentage}%")
            all_met_threshold = False
    
    if not target_files_relative:
        print("No target files specified for coverage check.")
        sys.exit(0)

    if not all_met_threshold:
        sys.exit(1)
    else:
        print("\nAll specified files meet the line coverage threshold.")

if __name__ == "__main__":
    main()
