# =============================================================================
# MobileNetV2 Edge Crop Leaf Disease Classifier
# Smart Agriculture Edge-Control System
# Dataset: PlantVillage (Tomato Leaf Conditions)
# =============================================================================

import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers

IMG_SIZE = (224, 224)
BATCH_SIZE = 32
NUM_CLASSES = 10 # 9 Tomato disease categories + 1 healthy class

def build_crop_disease_model():
    """Build lightweight edge vision classifier using MobileNetV2."""
    base_model = tf.keras.applications.MobileNetV2(
        input_shape=(*IMG_SIZE, 3),
        include_top=False,
        weights='imagenet',
        alpha=0.35 # Ultra-compact alpha for sub-400ms edge inference
    )
    base_model.trainable = False # Feature extraction stage

    inputs = keras.Input(shape=(*IMG_SIZE, 3))
    x = tf.keras.applications.mobilenet_v2.preprocess_input(inputs)
    x = base_model(x, training=False)
    x = layers.GlobalAveragePooling2D()(x)
    x = layers.Dropout(0.2)(x)
    outputs = layers.Dense(NUM_CLASSES, activation='softmax')(x)

    model = keras.Model(inputs, outputs, name="Crop_Disease_MobileNetV2")
    return model

if __name__ == "__main__":
    model = build_crop_disease_model()
    model.summary()
    print("[INFO] MobileNetV2 architecture compiled for edge crop pathology.")
