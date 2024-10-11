import pandas as pd
import sys

def combine(files, output_file):
    combined_data = pd.DataFrame()
    
    for i, file in enumerate(files):
        if i == 0:
            data = pd.read_csv(file)
        else:
            data = pd.read_csv(file, header=0)
        
        combined_data = pd.concat([combined_data, data], ignore_index=True)
    
    combined_data.to_csv(output_file, index=False)
    print(f"Files concatenated into {output_file} successfully.")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python concatenate_csv.py file1.csv file2.csv ... output.csv")
        sys.exit(1)
    
    input_files = sys.argv[1:-1]
    output_file = sys.argv[-1]
    
    combine(input_files, output_file)
