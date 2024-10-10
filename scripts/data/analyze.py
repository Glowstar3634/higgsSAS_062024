import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
from collections import Counter

def analyze_data(file_path, parameter, fixed_value=None):
    # Load data from the CSV file
    data = pd.read_csv(file_path)

    if parameter == 'production_channel':
        # Analyze by production channel
        production_channel_counts = data['ProductionChannel'].value_counts(normalize=True)  # Normalize to get ratios
        print("Production Channel Ratios:")
        print(production_channel_counts)

        # Plot histogram of production channels with ratios
        plt.bar(production_channel_counts.index, production_channel_counts.values)
        plt.xlabel('Production Channel')
        plt.ylabel('Ratio')
        plt.title('Production Channel Ratio Histogram')
        plt.xticks(rotation=45)  # Rotate x-axis labels for better readability
        plt.show()

    elif parameter == 'decay_products':
        # Analyze by decay products
        decay_products = data['DecayProducts'].str.split(';', expand=True).stack().value_counts(normalize=True)  # Normalize for ratios
        print("Decay Product Ratios:")
        print(decay_products)

        # Plot histogram of decay products with ratios
        plt.bar(decay_products.index.astype(int), decay_products.values)
        plt.xlabel('Decay Product ID')
        plt.ylabel('Ratio')
        plt.title('Decay Products Ratio Histogram')
        plt.xticks(rotation=45)
        plt.show()

    elif parameter == 'jet_stats':
        # Analyze by jet stats
        jet_ids = data['Jet_ID'].str.split(';', expand=True).stack().astype(int)
        unique_jets = jet_ids.unique()
        print(f"Number of unique jets: {len(unique_jets)}")
        print(f"Unique jet IDs: {unique_jets}")

        # Analyze production channels or decay products based on fixed_value
        for jet in unique_jets:
            rows_with_jet = data[data['Jet_ID'].str.contains(f';?{jet};?')]

            if fixed_value == 0:  # Production channel bins
                # Get the production channel counts for the current jet and normalize them
                production_channel_counts = rows_with_jet['ProductionChannel'].value_counts(normalize=True)

                plt.bar(production_channel_counts.index, production_channel_counts.values)
                plt.xlabel('Production Channel')
                plt.ylabel('Ratio')
                plt.title(f'Jet ID {jet}: Production Channel Ratio Histogram')
                plt.xticks(rotation=45)
                plt.show()

            elif fixed_value == 1:  # Decay product pairs
                # Count decay product pairs instead of individual particles
                decay_products = rows_with_jet['DecayProducts'].str.split(';', expand=True)
                decay_pairs = decay_products.apply(lambda row: tuple(sorted(row.dropna().astype(int).values)), axis=1)

                pair_counts = Counter(decay_pairs)
                total_pairs = sum(pair_counts.values())
                pair_ratios = {pair: count / total_pairs for pair, count in pair_counts.items()}

                # Sort pairs by ratio
                sorted_pairs = sorted(pair_ratios.items(), key=lambda x: -x[1])
                labels, ratios = zip(*sorted_pairs)

                plt.bar(range(len(ratios)), ratios, tick_label=labels)
                plt.xlabel('Decay Product Pairs')
                plt.ylabel('Ratio')
                plt.title(f'Jet ID {jet}: Decay Product Pair Ratios')
                plt.xticks(rotation=90)  # Rotate for readability
                plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python analyze3.py <input_file> <parameter> <fixed_value>")
        print("Parameters: 'production_channel', 'decay_products', or 'jet_stats'")
        print("fixed_value: 0 (production channels) or 1 (decay product pairs)")
        sys.exit(1)

    input_file = sys.argv[1]
    parameter = sys.argv[2]
    fixed_value = int(sys.argv[3])

    analyze_data(input_file, parameter, fixed_value)
