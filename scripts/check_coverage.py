import sys
import re
from collections import defaultdict

def parse_lcov_output(lcov_output):
    coverage_data = defaultdict(lambda: {'lines_hit': 0, 'lines_found': 0})
    current_file = None
    for line in lcov_output.splitlines():
        if line.startswith('SF:'):
            current_file = line[3:]
        elif line.startswith('DA:'):
            parts = line[3:].split(',')
            line_num, hit_count = int(parts[0]), int(parts[1].split(';')[0]) # some DA lines have checksum like DA:30,1;35B8000C
            if hit_count > 0:
                coverage_data[current_file]['lines_hit'] += 1
            coverage_data[current_file]['lines_found'] += 1
        elif line == 'end_of_record':
            current_file = None
    return coverage_data

def calculate_directory_coverage(coverage_data, directories):
    dir_coverage = defaultdict(lambda: {'lines_hit': 0, 'lines_found': 0})
    for file_path, data in coverage_data.items():
        for directory in directories:
            if file_path.startswith(directory):
                dir_coverage[directory]['lines_hit'] += data['lines_hit']
                dir_coverage[directory]['lines_found'] += data['lines_found']
                break # Avoid double counting if a file matches multiple directory prefixes
    
    results = {}
    for directory, data in dir_coverage.items():
        if data['lines_found'] == 0:
            results[directory] = 100.0
        else:
            results[directory] = (data['lines_hit'] / data['lines_found']) * 100
    return results

def main():
    if len(sys.argv) < 4:
        print("Usage: python check_coverage.py <lcov_output_file> <min_percentage> <dir1> [dir2 ...]")
        sys.exit(1)

    lcov_file_path = sys.argv[1]
    min_percentage = float(sys.argv[2])
    target_directories = sys.argv[3:]

    try:
        with open(lcov_file_path, 'r') as f:
            lcov_output = f.read()
    except FileNotFoundError:
        print(f"Error: LCOV output file not found at {lcov_file_path}")
        sys.exit(1)

    coverage_data = parse_lcov_output(lcov_output)
    directory_coverage_percentages = calculate_directory_coverage(coverage_data, target_directories)

    all_met_threshold = True
    for directory, percentage in directory_coverage_percentages.items():
        print(f"Coverage for {directory}: {percentage:.2f}%")
        if percentage < min_percentage:
            print(f"ERROR: Coverage for {directory} ({percentage:.2f}%) is below the required {min_percentage}%")
            all_met_threshold = False
    
    if not target_directories:
        print("No target directories specified for coverage check.")
        sys.exit(0) # Or 1, depending on desired behavior

    if not directory_coverage_percentages:
        print(f"ERROR: No coverage data found for specified directories: {', '.join(target_directories)}")
        print("Make sure your LCOV report includes these directories and they are not empty.")
        sys.exit(1)


    if not all_met_threshold:
        sys.exit(1)
    else:
        print("All specified directories meet the coverage threshold.")

if __name__ == "__main__":
    main()
