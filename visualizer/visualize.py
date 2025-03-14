import sys
import graph_tool.all as gt

def read_graph(filepath):
	g = gt.Graph(directed=False)
	with open(filepath, "r") as f:
		for line in f:
			line = line.strip()
			if line.startswith("c"):
				continue
			if line.startswith("p ds"):
				_, _, N, M = line.split()
				N, M = int(N), int(M)
				g.add_vertex(N)
				continue
			u, v = map(int, line.split())
			g.add_edge(u - 1, v - 1)
	return g

def visualize_graph(input_filepath, output_filepath):
	print("reading graph...")
	g = read_graph(input_filepath)
	print("creating layout...")
	pos = gt.sfdp_layout(g)
	deg = g.degree_property_map("total")
	print("drawing graph...")
	gt.graph_draw(g, pos, output=output_filepath, output_size=(5000, 5000))

if __name__ == "__main__":
	if len(sys.argv) != 3:
		print("usage: python3 visualize.py <input file path> <output file path>")
		sys.exit(1)
	input_filepath = sys.argv[1]
	output_filepath = sys.argv[2]
	visualize_graph(input_filepath, output_filepath)
