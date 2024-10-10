import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from collections import Counter

def analyze_data(file_path, parameter, fixed_value=None):
    # Load data from the CSV file
    data = pd.read_csv(file_path)

    if parameter == 'production_channel':
        # Analyze by production channel
        production_channel_counts = data['ProductionChannel'].value_counts()
        print("Production Channel Counts:")
        print(production_channel_counts)

        # Plot histogram of production channels
        plt.hist(data['ProductionChannel'], bins=np.arange(data['ProductionChannel'].min(), data['ProductionChannel'].max() + 1))
        plt.xlabel('Production Channel')
        plt.ylabel('Count')
        plt.title('Production Channel Histogram')
        plt.show()

    elif parameter == 'decay_products':
        # Analyze by decay products
        decay_products = data['DecayProducts'].str.split(';', expand=True).stack().value_counts()
        print("Decay Product Counts:")
        print(decay_products)

        # Plot histogram of decay products
        plt.hist(decay_products.index.astype(int), bins=np.arange(decay_products.index.astype(int).min(), decay_products.index.astype(int).max() + 1), weights=decay_products.values)
        plt.xlabel('Decay Product ID')
        plt.ylabel('Count')
        plt.title('Decay Products Histogram')
        plt.show()

    elif parameter == 'jet_stats':
        # Analyze by jet stats
        jet_ids = data['Jet_ID'].str.split(';', expand=True).stack().astype(int)
        unique_jets = jet_ids.unique()
        print(f"Number of unique jets: {len(unique_jets)}")
        print(f"Unique jet IDs: {unique_jets}")

        # Create histograms for each unique jet
        for jet in unique_jets:
            if fixed_value == 0:  # Production channel bins
                # Get all rows where the current jet ID appears
                rows_with_jet = data[data['Jet_ID'].str.contains(f';?{jet};?')]
                
                # Count each production channel associated with this jet
                production_channel_counts = rows_with_jet['ProductionChannel'].value_counts()

                # Ensure that production channels are counted twice if decay products are associated with the same jet
                channel_repeated = production_channel_counts * 2

                plt.bar(channel_repeated.index, channel_repeated.values)
                plt.xlabel('Production Channel')
                plt.ylabel('Count (considering jet decay)')
                plt.title(f'Jet ID {jet}: Production Channel Histogram')
                plt.show()

            elif fixed_value == 1:  # Decay product bins
                rows_with_jet = data[data['Jet_ID'].str.contains(f';?{jet};?')]

                # Count each decay product associated with this jet
                decay_products = rows_with_jet['DecayProducts'].str.split(';', expand=True).stack().astype(int).value_counts()

                plt.bar(decay_products.index, decay_products.values)
                plt.xlabel('Decay Product ID')
                plt.ylabel('Count')
                plt.title(f'Jet ID {jet}: Decay Product Histogram')
                plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python analyze3.py <input_file> <parameter> <fixed_value>")
        print("Parameters: 'production_channel', 'decay_products', or 'jet_stats'")
        print("fixed_value: 0 (production channels) or 1 (decay product pairs)")
        sys.exit(1)

    input_file = sys.argv[1]
    parameter = sys.argv[2]
    fixed_value = sys.argv[3]

    analyze_data(input_file, parameter, fixed_value)
