import sys
import tensorflow as tf
from tensorflow.keras.models import load_model
from tensorflow.keras.utils import plot_model

def main(model_path):
    # Load the model from the provided path
    model = load_model(model_path, custom_objects={'mse': tf.keras.losses.MeanSquaredError})
    
    plot_model(model, to_file='model_diagram.png', show_shapes=True, show_layer_names=True)
    print(f"Model diagram saved as 'model_diagram.png'.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python plotModel.py <model_path>")
        sys.exit(1)
    
    model_path = sys.argv[1]
    main(model_path)
