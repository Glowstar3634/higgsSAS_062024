import pandas as pd
import sys
from scipy.stats import chisquare

def normalize_ratios(ratios):
    total = sum(ratios.values())
    return {key: value / total for key, value in ratios.items()} if total > 0 else ratios

# Load observed data and filter as needed
def filter_data_by_production_channel(data, channel):
    print(f"Filtering for Production Channel {channel} ...")
    return data[data['ProductionChannel'] == channel]

def filter_data_by_decay_products(data, decay_products):
    print(f"Filtering for Decay Products {decay_products} ...")
    return data[data['DecayProducts'].str.contains(decay_products, regex=False)]

def get_decay_product_bins(filtered_data):
    decay_pairs = filtered_data['DecayProducts'].str.split(';')
    # Create keys in the format "#;#"
    return pd.Series([f"{decay[i]};{decay[i+1]}" for decay in decay_pairs for i in range(0, len(decay)-1, 2)]).value_counts()

def get_production_channel_bins(filtered_data):
    return filtered_data['ProductionChannel'].value_counts()

# Remove bins with 0 counts in either the observed or expected datasets
def calculate_chi_square(observed_bins, expected_ratios, filter_value):
    total_observed = sum(observed_bins)  # Total number of observed events
    print(f"Total Observed: {total_observed}...")

    expected_counts = []
    observed_counts = []

    for bin_name in expected_ratios.keys():
        expected_count = expected_ratios[bin_name] * total_observed  # Calculate expected count
        observed_count = observed_bins.get(bin_name, 0)  # Count as 0 if bin is missing

        observed_counts.append(observed_count)
        expected_counts.append(expected_count)

    if len(observed_counts) == 0:
        print("No valid bins for chi-square calculation.")
        return

    # Print sums of observed and expected counts
    print(f"Sum of Observed Frequencies: {sum(observed_counts)}")
    print(f"Sum of Expected Frequencies (before normalization): {sum(expected_counts)}")

    # Normalize expected_counts to match total_observed
    total_expected = sum(expected_counts)
    if total_expected > 0:
        normalization_factor = total_observed / total_expected
        expected_counts = [count * normalization_factor for count in expected_counts]

    # Print sums after normalization
    print(f"Sum of Expected Frequencies (after normalization): {sum(expected_counts)}")

    # Perform chi-square test
    chi2, p = chisquare(f_obs=observed_counts, f_exp=expected_counts)

    print(f"Chi-Square Statistic: {chi2}")
    print(f"P-value: {p}")

def main(observed_file, filter_type, filter_value):
    # Load observed data
    observed_data = pd.read_csv(observed_file)
    filter_value = str(filter_value) 

    # Apply the appropriate filter
    if filter_type == "production_channel":
        observed_filtered = filter_data_by_production_channel(observed_data, int(filter_value))
        observed_bins = get_decay_product_bins(observed_filtered)
        
    elif filter_type == "decay_products":
        observed_filtered = filter_data_by_decay_products(observed_data, filter_value)
        observed_bins = get_production_channel_bins(observed_filtered)

    else:
        print("Invalid filter type. Use 'production_channel' or 'decay_products'.")
        sys.exit(1)

    # Using the expected_ratios object
    expected_ratios = {
        "production_channel": {
            "902": 0.8610,
            "907": 0.0501,
            "905": 0.0313,
            "906": 0.0203,
            "904": 0.0185,
            "901": 0.0091,
            "908": 0.0038,
            "909": 0.0019,
        },
        "decay_products": {
            "5;-5": 0.571,
            "24;-24": 0.22,
            "21;21": 0.0853,
            "15;-15": 0.0626,
            "4;-4": 0.0288,
            "23;23": 0.0273,
            "22;22": 0.00228,
            "22;23": 0.00157,
            "3;-3": 0.00044,
            "13;-13": 0.00022,
        }
    }
    expected_ratios["production_channel"] = normalize_ratios(expected_ratios["production_channel"])
    expected_ratios["decay_products"] = normalize_ratios(expected_ratios["decay_products"])

    # Perform chi-square goodness of fit test
    if filter_type == "production_channel":
        calculate_chi_square(observed_bins, expected_ratios["decay_products"], filter_value)
        
    elif filter_type == "decay_products":
        calculate_chi_square(observed_bins, expected_ratios["production_channel"], filter_value)
    

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python chisquare.py <observed_file> <filter_type> <filter_value>")
        print("filter_type: 'production_channel' or 'decay_products'")
        print("filter_value: the value for filtering (e.g., 902 or '5;-5')")
        sys.exit(1)

    observed_file = sys.argv[1]
    filter_type = sys.argv[2]
    filter_value = sys.argv[3]
    main(observed_file, filter_type, filter_value)
