# Machine Learning-Based Anomaly Detection in 5G Networks

This project integrates NS-3 mmWave simulations with machine learning to detect anomalies in 5G networks. It combines network traffic monitoring with Isolation Forest to identify abnormal behaviors based on real-time metrics.

## 🧩 Features

- Simulate 5G mmWave network traffic in NS-3
- Generate FlowMonitor and NetAnim outputs
- Convert network metrics to CSV format
- Apply unsupervised ML (Isolation Forest) for anomaly detection
- Visualize results using scatter plots

## 📁 Project Structure

- `ns3_simulation/`: NS-3 scripts and output files
- `ml_detection/`: Python scripts for anomaly detection
- `documentation/`: Reports and diagrams
- `images/`: Output visuals like plots and animations

## 🛠️ Tools Used

- NS-3 with mmWave module
- Python (pandas, scikit-learn, matplotlib)
- NetAnim
- Wireshark (optional)

## 🚀 How to Run

### Simulation

```bash
cd ns3_mmwave
./waf --run scratch/custom_anomaly
