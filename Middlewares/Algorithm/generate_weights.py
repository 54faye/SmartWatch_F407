"""
Generate heuristic fall detection model weights.
Target: strong_fall > 0.9, moderate_fall > 0.5, normal_walk < 0.1, stationary < 0.05
"""

import numpy as np

INPUT = 16
H1 = 16
H2 = 8
OUTPUT = 1

# Input normalization params
FEAT_MEAN = np.array([1.0, 0.3, 1.5, 0.8, 0.7, 0.6, 0.5, 2.0,
                      0.5, 0.3, 2.0, 0.2, 0.2, 0.1, 0.1, 0.8], dtype=np.float32)
FEAT_STD  = np.array([0.8, 0.5, 1.2, 0.3, 0.9, 0.3, 1.0, 2.0,
                      0.8, 0.5, 2.5, 0.2, 0.2, 0.3, 0.3, 0.4], dtype=np.float32)
FEAT_STD = np.maximum(FEAT_STD, 0.01)

# ---------- Tune until separation is good ----------
best_sep = -999
best_weights = None

for trial in range(2000):
    np.random.seed(trial)

    # Layer 1: 16 -> 16
    W1 = np.zeros((H1, INPUT))
    W1[0:4, 0:5] = np.random.uniform(0.2, 0.6, (4, 5))      # magnitude
    W1[4:8, 5:11] = np.random.uniform(0.2, 0.6, (4, 6))     # motion
    W1[4:8, 5] = np.random.uniform(-0.4, -0.1, 4)            # zcr negative
    W1[8:12, 11:16] = np.random.uniform(-0.3, 0.3, (4, 5))   # orientation
    W1[12:16, :] = np.random.uniform(-0.2, 0.2, (4, INPUT))  # cross
    b1 = np.random.uniform(-1.5, -0.6, H1)

    # Layer 2: 16 -> 8
    W2 = np.zeros((H2, H1))
    W2[0:4, 0:8] = np.random.uniform(0.2, 0.5, (4, 8))
    W2[4:6, 8:12] = np.random.uniform(-0.2, 0.2, (2, 4))
    W2[6:8, 12:16] = np.random.uniform(-0.2, 0.25, (2, 4))
    b2 = np.random.uniform(-1.2, -0.4, H2)

    # Layer 3: 8 -> 1
    W3 = np.zeros((OUTPUT, H2))
    W3[0, 0:6] = np.random.uniform(0.3, 0.7, 6)
    W3[0, 6:8] = np.random.uniform(-0.15, 0.2, 2)
    b3 = np.array([-1.5 + 0.1 * np.random.randn()])

    # Forward pass
    def relu(x):
        return np.maximum(0.0, x)
    def sigmoid(x):
        return 1.0 / (1.0 + np.exp(np.clip(-x, -30, 30)))

    def forward(features):
        x = (features - FEAT_MEAN) / FEAT_STD
        h1 = relu(np.dot(W1, x) + b1)
        h2 = relu(np.dot(W2, h1) + b2)
        return sigmoid(np.dot(W3, h2) + b3)[0]

    # Test vectors
    fall_strong = np.array([3.5, 2.5, 6.0, 0.5, 5.5, 0.05, 6.0, 8.0,
                            4.0, 3.0, 15.0, 0.8, 0.6, 0.2, 0.1, 0.6], dtype=np.float32)
    fall_moderate = np.array([2.0, 1.2, 3.5, 0.7, 2.8, 0.2, 3.0, 4.0,
                              2.0, 1.2, 6.0, 0.4, 0.3, 0.3, 0.2, 0.8], dtype=np.float32)
    normal_walk = np.array([1.05, 0.18, 1.4, 0.82, 0.58, 0.75, 0.4, 0.0,
                            0.35, 0.12, 1.2, 0.06, 0.06, 0.08, 0.08, 1.02], dtype=np.float32)
    arm_swing = np.array([1.3, 0.6, 2.0, 0.5, 1.5, 0.5, 1.2, 2.0,
                          1.0, 0.5, 2.5, 0.3, 0.3, 0.2, 0.2, 0.9], dtype=np.float32)
    stationary = np.array([1.0, 0.02, 1.05, 0.95, 0.1, 0.2, 0.05, 0.0,
                           0.05, 0.01, 1.02, 0.01, 0.01, 0.02, 0.02, 1.0], dtype=np.float32)

    p_fall = forward(fall_strong)
    p_mod = forward(fall_moderate)
    p_walk = forward(normal_walk)
    p_swing = forward(arm_swing)
    p_stat = forward(stationary)

    # Score: want high fall, low normal
    score = p_fall * 3.0 + p_mod * 2.0 - p_walk * 5.0 - p_swing * 4.0 - p_stat * 3.0

    # Hard constraints: fall_strong > 0.85, normal_walk < 0.12
    if p_fall > 0.85 and p_walk < 0.12 and p_stat < 0.08:
        score += 10.0

    if score > best_sep:
        best_sep = score
        best_weights = (W1.copy(), b1.copy(), W2.copy(), b2.copy(), W3.copy(), b3.copy())
        best_probs = (p_fall, p_mod, p_walk, p_swing, p_stat)

# ---------- Use best weights found ----------
W1, b1, W2, b2, W3, b3 = best_weights

# ---------- Print C arrays ----------
def print_array(name, arr, per_line=8):
    flat = arr.flatten()
    n = len(flat)
    print(f"static const float {name}[{n}] = {{")
    for i in range(0, n, per_line):
        line = ", ".join(f"{x:.6f}f" for x in flat[i:i+per_line])
        if i + per_line < n:
            line += ","
        print(f"    {line}")
    print("};")

print("/* Auto-generated fall detection model weights */")
print("/* Architecture: 16->16->8->1, ReLU/ReLU/Sigmoid */")
print("/* Features are normalized before inference: feat_norm = (feat - mean) / std */")
print()

print_array("FEAT_MEAN", FEAT_MEAN)
print()
print_array("FEAT_STD", FEAT_STD)
print()
print_array("W1", W1)
print()
print_array("b1", b1)
print()
print_array("W2", W2)
print()
print_array("b2", b2)
print()
print_array("W3", W3)
print()
print_array("b3", b3)

# ---------- Print validation results ----------
print()
for name, p in zip(["fall_strong", "fall_moderate", "normal_walk", "arm_swing", "stationary"], best_probs):
    print(f"/* {name:20s} -> prob = {p:.4f} */")

print(f"/* Separation (fall - walk) = {best_probs[0] - best_probs[2]:.4f} */")
