ns3-mmwave-anomaly-project/results/flowmon_xml_to_csv.py
import xml.etree.ElementTree as ET
import csv

tree = ET.parse('flowmon_results.xml')
root = tree.getroot()

with open('flowmon_results.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow([
        "FlowId", "TimeFirstTxPacket", "TimeLastTxPacket", "TimeFirstRxPacket", "TimeLastRxPacket",
        "DelaySum", "JitterSum", "TxBytes", "RxBytes", "TxPackets", "RxPackets", "LostPackets", "PacketLossRatio"
    ])

    for flow in root.iter('Flow'):
        fid = flow.attrib.get('flowId', 'N/A')
        stats = flow.find('FlowStats')

        if stats is not None:
            writer.writerow([
                fid,
                stats.attrib.get('timeFirstTxPacket', '0'),
                stats.attrib.get('timeLastTxPacket', '0'),
                stats.attrib.get('timeFirstRxPacket', '0'),
                stats.attrib.get('timeLastRxPacket', '0'),
                stats.attrib.get('delaySum', '0'),
                stats.attrib.get('jitterSum', '0'),
                stats.attrib.get('txBytes', '0'),
                stats.attrib.get('rxBytes', '0'),
                stats.attrib.get('txPackets', '0'),
                stats.attrib.get('rxPackets', '0'),
                stats.attrib.get('lostPackets', '0'),
                stats.attrib.get('lostPacketsRatio', '0')
            ])
        else:
            print(f"[WARNING] No <FlowStats> found for Flow ID {fid}, skipping.")