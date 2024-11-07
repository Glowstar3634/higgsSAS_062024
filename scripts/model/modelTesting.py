import tensorflow as tf
from tensorflow.keras.models import load_model
import pandas as pd
import numpy as np
import re
import sys
import os

def load_dataset(file_path):
    """Load a single CSV file as a DataFrame."""
    return pd.read_csv(file_path, skiprows=1)

def preprocess_features(data):
    # Split DecayProducts into two columns and convert them to floats
    decay_products_split = data[' DecayProducts'].str.split(';', expand=True)
    data['DecayProduct1'] = decay_products_split[0].astype(np.float32)
    data['DecayProduct2'] = decay_products_split[1].astype(np.float32)
    
    # Select the columns, replacing DecayProducts with the two new columns
    features = data[['HiggsBoson', 'DecayProduct1', 'DecayProduct2', ' InvMasses', ' pT', ' Rapidity', ' JetMultiplicity']]
    
    # Convert to float32 for compatibility with TensorFlow
    X = np.asarray(features).astype(np.float32)
    
    return X

def test_model(testing_dataset, model_path):
    # Load the test dataset and remove the header row
    data = load_dataset(testing_dataset).iloc[1:]
    print(data.head())
    
    # Load and preprocess features
    data.columns = ['HiggsBoson', ' DecayProducts', ' InvMasses', ' pT', ' Rapidity', ' JetMultiplicity']
    X_test = preprocess_features(data)
    
    # Load the trained model
    model = load_model(model_path, custom_objects={'mse': tf.keras.losses.mean_squared_error})
    
    # Predict Wilson coefficients
    predictions = model.predict(X_test)
    
    # Output the Wilson coefficient predictions
    for i, pred in enumerate(predictions):
        print(f"Prediction for sample {i+1}:")
        for j, coeff in enumerate(pred):
            print(f"  Wilson coefficient {j+1}: {coeff}")
        print("\n")

def main(testing_dataset, model_path):
    test_model(testing_dataset, model_path)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 modelTesting.py <testing_dataset> <model_path>")
        sys.exit(1)
    testing_dataset = sys.argv[1]
    model_path = sys.argv[2]
    main(testing_dataset, model_path)
