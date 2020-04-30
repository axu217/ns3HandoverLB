# Computes Average Number of Handovers and Cell Throughput based on ./waf --run scratch/lena-x2-handover 2>&1 | tee output.txt
# Run ./waf --run ___simulation_to_run 2>&1 | tee output.txt
# Also be sure to enable PCAP tracing


class Performance_Metrics:
	def __init__(self, ANOH_fileName, CT_fileName, ANOH=True, CellThroughput=True, OptimizeRatio=True):
		self.ANOH_fileName = ANOH_fileName
		self.CT_fileName = CT_fileName 

		self.num_UE = 0
		self.num_eNB = 0
		self.simTime = 0
		self.findSimParameters()

		self.do_ANOH = ANOH		
		self.do_CellThroughput = CellThroughput
		self.do_OptimizeRatio = OptimizeRatio

		if OptimizeRatio or ANOH:
			self.ANOH = {ue+1:0 for ue in range(self.num_UE)}
		if OptimizeRatio or CellThroughput:
			# DlAverageThroughputKbps
			self.CellThroughput = {cell+1:0 for cell in range(self.num_eNB)}
		if OptimizeRatio:
			self.OptimizeRatio = 0


	def findSimParameters(self):
		output_file = open(self.ANOH_fileName, 'r')
		for line in output_file:
			words = line.split()
			if words[0] == "Number" and words[2] == "UEs:":
				self.num_UE = int(words[3])
			elif words[0] == "Number" and words[2] == "eNBs:":
				self.num_eNB = int(words[3])
			elif words[0] == "Simulation" and words[1] == "Time":
				# in nanoseconds, converting to seconds
				self.simTime = float(words[2][1:-2])/ 1e9
				break
		output_file.close()

	def computeANOH(self):
		output_file = open(self.ANOH_fileName, 'r')
		for line in output_file:
			words = line.split()
			if "successful" in words and "handover" in words:
				ue = int(words[words.index('UE')+2][:-1])
				self.ANOH[ue] += 1 / self.simTime
		output_file.close()

	def computeThroughput(self):
		# RxBytes is the 10th column
		DlRlcStats = open(self.CT_fileName,'r')
		for line in DlRlcStats:
			words = line.split()
			if len(words)==18:
				cell_id = int(words[2])
				DlRxBytes = float(words[10])
				self.CellThroughput[cell_id] += DlRxBytes * 8 / 1000 / self.simTime
		DlRlcStats.close()


	def computeMetrics(self):
		if self.do_OptimizeRatio or self.do_ANOH:
			self.computeANOH()
			combined_ANOH = sum(self.ANOH.values())/self.num_UE
			print("ANOH: ", combined_ANOH)
		if self.do_OptimizeRatio or self.compute_CellThroughput:
			self.computeThroughput()
			combined_Throughput = sum(self.CellThroughput.values())/self.num_eNB
			print("Throughput: ", combined_Throughput)
		if self.do_OptimizeRatio:
			self.OptimizeRatio = combined_Throughput/combined_ANOH
			print("Optimize Ratio", self.OptimizeRatio)


if __name__ == "__main__":
	import argparse
	parser = argparse.ArgumentParser(description='Handover Performance Metrics')
	parser.add_argument('--ANOH_file', type=str, default= 'output.txt', 
		help="""Name of file to compute ANOH Metrics from,
		be sure to enable logging for a3rsrp in example""")
	parser.add_argument('--CT_file', type=str, default= "DlRlcStats.txt", 
		help="""Name of file to compute cell throughput from,
		be sure to enable pcap tracing for the RLC in example""")
	args = parser.parse_args()

	metrics = Performance_Metrics(args.ANOH_file, args.CT_file)
	metrics.computeMetrics()


