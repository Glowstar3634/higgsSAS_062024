import pandas as pd
import sys
from scipy.stats import chisquare

def normalize_ratios(ratios):
    total = sum(ratios.values())
    return {key: value / total for key, value in ratios.items()} if total > 0 else ratios

def filter_data_by_jet_stats(data):
    print(f"Filtering for Leading Jet (jetID == 0 for either jet in the pair) ...")
    data[['Jet1', 'Jet2']] = data['Jet_ID'].str.split(';', expand=True).astype(int)
    
    # Filter for events where a decay is associated with leading jet
    data_first_jet = data[data['Jet1'] == 0]
    data_second_jet = data[data['Jet2'] == 0]
    filtered_data = pd.concat([data_first_jet, data_second_jet])
    
    return filtered_data

def filter_data_by_production_channel(data, channel):
    print(f"Filtering for Production Channel {channel} ...")
    return data[data['ProductionChannel'] == channel]

def filter_data_by_decay_products(data, decay_products):
    print(f"Filtering for Decay Products {decay_products} ...")
    return data[data['DecayProducts'].str.contains(decay_products, regex=False)]

def get_decay_product_bins(filtered_data):
    decay_pairs = filtered_data['DecayProducts'].str.split(';')
    return pd.Series([f"{decay[i]};{decay[i+1]}" for decay in decay_pairs for i in range(0, len(decay)-1, 2)]).value_counts()

def get_production_channel_bins(filtered_data):
    return filtered_data['ProductionChannel'].value_counts()

def get_jet_bins(filtered_data):
    return filtered_data['Jet_ID'].value_counts()

def calculate_chi_square(observed_bins, expected_ratios, filter_value):
    total_observed = sum(observed_bins)
    print(f"Total Observed: {total_observed}...")

    expected_counts = []
    observed_counts = []

    if filter_type == "production_channel":
        for bin_name in expected_ratios.keys():
            expected_count = expected_ratios[bin_name] * total_observed
            observed_count = observed_bins.get(bin_name, 0)  # Count as 0 if bin is missing (decay did not occur)
            observed_counts.append(observed_count)
            expected_counts.append(expected_count)
    elif filter_type == "decay_products":
        for bin_name in expected_ratios.keys():
            expected_count = expected_ratios[bin_name] * total_observed
            observed_count = observed_bins.get(int(bin_name), 0) 
            observed_counts.append(observed_count)
            expected_counts.append(expected_count)
    else:
        if filter_value == "1":
            for bin_name in expected_ratios.keys():
                expected_count = expected_ratios[bin_name] * total_observed  
                observed_count = observed_bins.get(bin_name, 0) 
                observed_counts.append(observed_count)
                expected_counts.append(expected_count)
        elif filter_value == "0":
            for bin_name in expected_ratios.keys():
                expected_count = expected_ratios[bin_name] * total_observed 
                observed_count = observed_bins.get(int(bin_name), 0) 
                observed_counts.append(observed_count)
                expected_counts.append(expected_count)

    if len(observed_counts) == 0:
        print("No valid bins for chi-square calculation.")
        return

    total_expected = sum(expected_counts)

    # Normalize expected_counts to match total_observed
    if total_expected > 0:
        normalization_factor = total_observed / total_expected
        expected_counts = [count * normalization_factor for count in expected_counts]

    print(f"Sum of Observed Frequencies: {sum(observed_counts)}")
    print(f"Sum of Expected Frequencies (after normalization): {sum(expected_counts)}")

    # Perform chi-square test
    chi2, p = chisquare(f_obs=observed_counts, f_exp=expected_counts)

    print(f"Chi-Square Statistic: {chi2}")
    print(f"P-value: {p}")

def main(observed_file, filter_type, filter_value):
    observed_data = pd.read_csv(observed_file)
    filter_value = str(filter_value) 

    # Apply the appropriate filter based on the filter_type
    if filter_type == "production_channel":
        observed_filtered = observed_data
        observed_bins = get_decay_product_bins(observed_filtered)
        
    elif filter_type == "decay_products":
        observed_filtered = observed_data
        observed_bins = get_production_channel_bins(observed_filtered)

    elif filter_type == "jet_stats":
        observed_filtered = observed_data
        if filter_value == "0":
            observed_bins = get_production_channel_bins(observed_filtered)
        elif filter_value == "1":
            observed_bins = get_decay_product_bins(observed_filtered)
        else:
            print("Invalid filter value for 'jet_stats'. Use 0 for production channel bins, or 1 for decay product bins.")
            sys.exit(1)

    else:
        print("Invalid filter type. Use 'production_channel', 'decay_products', or 'jet_stats'.")
        sys.exit(1)

    #Standard Model ratio predictions (to be confirmed)
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
            "903": 0.0001,
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

    else:
        if filter_value == "0":
            calculate_chi_square(observed_bins, expected_ratios["production_channel"], filter_value)
        elif filter_value == "1":
            calculate_chi_square(observed_bins, expected_ratios["decay_products"], filter_value)
        else:
            print("Invalid filter value. Use 0 for production channel bins, or 1 for decay product bins.")
            sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 4 or len(sys.argv) > 4:
        print("Usage: python chisquare.py <observed_file> <filter_type> <filter_value>")
        print("filter_type: 'production_channel', 'decay_products', or 'jet_stats'")
        print("filter_value: the value for filtering (e.g., 902 or '5;-5')")
        sys.exit(1)

    observed_file = sys.argv[1]
    filter_type = sys.argv[2]
    filter_value = sys.argv[3]
    main(observed_file, filter_type, filter_value)
