import tensorflow as tf
from tensorflow.keras.models import Sequential, load_model
from tensorflow.keras.layers import Dense
import pandas as pd
import glob
import os
import sys

def evaluate_on_test_set(model_path, test_file):
    if not os.path.exists(model_path):
        print(f"Model file {model_path} not found.")
        return
    
    model = load_model(model_path)
    test_data = load_dataset(test_file)
    
    X_test = test_data[['HiggsBoson', 'DecayProducts', 'InvMasses', 'pT', 'Rapidity', 'JetMultiplicity']].values
    y_test = test_data[['Coefficient1', 'Coefficient2', 'Coefficient3', 'Coefficient4', 'Coefficient5',
                        'Coefficient6', 'Coefficient7', 'Coefficient8', 'Coefficient9']].values
    
    test_loss, test_mae = model.evaluate(X_test, y_test, verbose=1)
    print(f"Test Loss: {test_loss}, Test MAE: {test_mae}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 modelTesting.py <test_file> <model_path>")
        sys.exit(1)

    test_file = sys.argv[1]
    model_path = sys.argv[2]

    evaluate_on_test_set(model_path, test_file)