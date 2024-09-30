import pandas as pd
import matplotlib.pyplot as plt
import sys

def plot_histogram(data, parameter, fixed_value):
    # Ensure fixed_value is of the correct type
    if parameter == "production_channel":
        fixed_value = int(fixed_value)
    
    # Filter the data based on the fixed parameter
    print(data.head())
    if parameter == "production_channel":
        print(f"Filtering for Production Channel {fixed_value}...")
        filtered_data = data[data['ProductionChannel'] == fixed_value]
        print(f"Total:\n{len(filtered_data)}")
        
        if filtered_data.empty:
            print("No matching data found for this production channel.")
            return
        
        ratios = filtered_data['DecayProducts'].value_counts(normalize=True)
        title = f'Decay Product Ratios for Production Channel {fixed_value}'
        xlabel = 'Decay Products'
    elif parameter == "decay_products":
        print(f"Filtering for Decay Products containing {fixed_value}...")
        filtered_data = data[data['DecayProducts'].str.contains(fixed_value, regex=False)]
        print(f"Total:\n{len(filtered_data)}")

        if filtered_data.empty:
            print("No matching data found for these decay products.")
            return
        
        ratios = filtered_data['ProductionChannel'].value_counts(normalize=True)
        title = f'Production Channel Ratios for Decay Products {fixed_value}'
        xlabel = 'Production Channels'
    else:
        print("Invalid parameter. Use 'production_channel' or 'decay_products'.")
        return

    # Plot the histogram
    plt.figure(figsize=(10, 6))
    ratios.plot(kind='bar', color='skyblue')
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel('Ratio')
    plt.xticks(rotation=45)
    plt.grid(axis='y')
    plt.tight_layout()
    plt.show()

def main(input_file, parameter, fixed_value):
    # Read the CSV file
    data = pd.read_csv(input_file)
    
    # Call the function to plot the histogram
    plot_histogram(data, parameter, fixed_value)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python decay_histogram.py <input_file> <parameter> <fixed_value>")
        print("Parameters: 'production_channel' or 'decay_products'")
        sys.exit(1)

    input_file = sys.argv[1]
    parameter = sys.argv[2]
    fixed_value = sys.argv[3]

    main(input_file, parameter, fixed_value)
