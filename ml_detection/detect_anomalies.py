import pandas as pd
from sklearn.ensemble import IsolationForest
import matplotlib.pyplot as plt

# Load CSV
df = pd.read_csv('flowmon_results.csv')

# Features for anomaly detection
features = df[[
    "DelaySum", "JitterSum", "TxBytes", "RxBytes",
    "TxPackets", "RxPackets", "LostPackets", "PacketLossRatio"
]]

# Train model
model = IsolationForest(contamination=0.2, random_state=42)
df['anomaly'] = model.fit_predict(features)

# Map anomaly column: -1 = anomaly, 1 = normal
df['anomaly'] = df['anomaly'].map({1: 'normal', -1: 'anomaly'})

# Save result
df.to_csv('anomaly_output.csv', index=False)
print("[✔] Anomaly detection complete! Saved as anomaly_output.csv")

# Plot
plt.figure(figsize=(10, 5))
colors = df['anomaly'].map({'normal': 'green', 'anomaly': 'red'})
plt.scatter(df.index, df['PacketLossRatio'], c=colors)
plt.title('Anomaly Detection: Packet Loss Ratio')
plt.xlabel('Flow Index')
plt.ylabel('Packet Loss Ratio')
plt.grid(True)
plt.savefig('anomaly_plot.png')
plt.show()