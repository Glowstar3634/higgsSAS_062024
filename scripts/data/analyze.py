import pandas as pd
import matplotlib.pyplot as plt
import sys

def plot_histogram(data, parameter, fixed_value):
    if parameter == "production_channel":
        fixed_value = int(fixed_value)
    
    if parameter == "production_channel":
        print(f"Filtering for Production Channel {fixed_value} ...")
        filtered_data = data[data['ProductionChannel'] == fixed_value]
        print(f"Total: \n{len(filtered_data)}")
        
        if filtered_data.empty:
            print("No matching data found for this production channel.")
            return
        
        decay_pairs = filtered_data['DecayProducts'].str.split(';')
        decay_pairs = pd.Series([tuple(sorted([decay[i], decay[i+1]])) for decay in decay_pairs for i in range(0, len(decay)-1, 2)]).value_counts(normalize=True)
        title = f'Decay Product Pair Ratios for Production Channel {fixed_value}'
        xlabel = 'Decay Product Pairs'

    
    elif parameter == "decay_products":
        print(f"Filtering for Decay Products containing {fixed_value} ...")
        filtered_data = data[data['DecayProducts'].str.contains(fixed_value, regex=False)]
        print(f"Total: \n{len(filtered_data)}")
        
        if filtered_data.empty:
            print("No matching data found for these decay products.")
            return
        
        ratios = filtered_data['ProductionChannel'].value_counts(normalize=True)
        title = f'Production Channel Ratios for Decay Products {fixed_value}'
        xlabel = 'Production Channels'
    
    else:
        print("Invalid parameter. Use 'production_channel', 'decay_products', or 'jet_stats'.")
        return
    
    # Plot the histogram
    plt.figure(figsize=(10,6))
    ratios.plot(kind='bar', color='skyblue')
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel('Ratio')
    plt.xticks(rotation=45)
    plt.grid(axis='y')
    plt.tight_layout()
    plt.show()

def plot_jet_histograms(data, fixed_value):
    print("Plotting Jet Statistics...")

    unique_jet_ids = set([jet_id for sublist in data['Jet_ID'] for jet_id in sublist])
    print(f"Unique Jet IDs found: {unique_jet_ids}")

    if not unique_jet_ids:
        print("No valid Jet_IDs found in the data.")
        return

    for jet_id in unique_jet_ids:
        print(f"Analyzing data for Jet_ID {jet_id} ...")

        filtered_data = data[data['Jet_ID'].apply(lambda jet_list: jet_id in jet_list)]
        print(f"Data found for Jet_ID {jet_id}: {len(filtered_data)} entries")

        if filtered_data.empty:
            print(f"No data found for Jet_ID {jet_id}. Skipping...")
            continue

        if fixed_value == 0:
            ratios = filtered_data['ProductionChannel'].value_counts(normalize=True)
            title = f'Production Channel Ratios for Jet_ID {jet_id}'
            xlabel = 'Production Channels'
        
        elif fixed_value == 1:
            decay_pairs = filtered_data['DecayProducts'].str.split(';')
            ratios = pd.Series([tuple(sorted([dp[i], dp[j]])) for dp in decay_pairs for i in range(len(dp)) for j in range(i+1, len(dp))]).value_counts(normalize=True)
            title = f'Decay Product Pair Ratios for Jet_ID {jet_id}'
            xlabel = 'Decay Product Pairs'
        
        else:
            print(f"Invalid fixed_value: {fixed_value}. Use 0 for production channels or 1 for decay product pairs.")
            return

        plt.figure(figsize=(10,6))
        print(f"Plotting histogram for Jet_ID {jet_id} with {len(ratios)} bins...")
        ratios.plot(kind='bar', color='lightcoral')
        plt.title(title)
        plt.xlabel(xlabel)
        plt.ylabel('Ratio')
        plt.xticks(rotation=45)
        plt.grid(axis='y')
        plt.tight_layout()
        plt.show()

def main(input_file, parameter, fixed_value):
    data = pd.read_csv(input_file)
    
    data['ProductionChannel'] = data['ProductionChannel'].astype(int)
    data['Jet_ID'] = data['Jet_ID'].apply(lambda x: [int(i) for i in x.split(';')] if pd.notna(x) else [])
    
    if parameter == 'jet_stats':
        plot_jet_histograms(data, int(fixed_value))
    else:
        plot_histogram(data, parameter, fixed_value)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python analyze2.py <input_file> <parameter> <fixed_value>")
        print("Parameters: 'production_channel', 'decay_products', or 'jet_stats'")
        print("fixed_value: 0 (production channels) or 1 (decay product pairs)")
        sys.exit(1)

    input_file = sys.argv[1]
    parameter = sys.argv[2]
    fixed_value = int(sys.argv[3])
    main(input_file, parameter, fixed_value)
