import networkx as nx
import matplotlib.pyplot as plt

'''The loader is not optimized for very large graphs since it 
stores the file in memory. If needed the load_graph function
must be optimized to read the single lines without loading 
all the graph in memory'''

class VFGraph(nx.DiGraph):

    def load(self, gfile):
        '''Load the graph from the VF format as a NetworkX DiGraph'''
        with open(gfile, 'r') as f:
            lines = [l for l in f.readlines() if not l.startswith('#')] #filtering file comment lines
            nodes_count = int(lines[0])
            nodes_lines = lines[1:nodes_count+1]
            edge_lines = lines[nodes_count+1:]
            #Reading node lables
            n = 0
            for nline in nodes_lines:
                line_data = nline.split()
                #print(f'Reading node: {line_data}')
                node_label = line_data[1]
                #adding node with label
                self.add_node(n, label=node_label)
                n += 1
            
            #Reading edges, without no control on the file format (number of edges)
            for eline in edge_lines:
                line_data = eline.split()
                #print(f'Reading edge: {line_data}')
                if len(line_data) == 2:   #edge with no label
                    n1, n2 = line_data
                    self.add_edge(int(n1), int(n2))    
                elif len(line_data) == 3: #edge with label
                    n1, n2, l = line_data
                    self.add_edge(int(n1), int(n2), label=l)
                else:
                    pass #edge count line or invilid
        

    def print(self, outfile):
        '''Plot the graph on file'''        

        #Position of the nodes
        pos = nx.spring_layout(self)    

        # Drawing node and edges
        nx.draw(self, pos, with_labels=True, node_color='lightblue', node_size=1500, font_size=10)

        # Drawing labels
        node_labels = nx.get_node_attributes(self, 'label')
        nx.draw_networkx_labels(self, pos, labels=node_labels, font_size=12)

        edge_labels = nx.get_edge_attributes(self, 'label')
        nx.draw_networkx_edge_labels(self, pos, edge_labels=edge_labels, font_color='red', font_size=10)

        plt.axis('off')
        plt.savefig(outfile)


    def write(self, outfile):
        '''Write the graph in VF format'''
        with open(outfile, "w", encoding='utf-8') as gfile:
            #Writing nodes
            node_count = len(self.nodes())
            gfile.write(f'{node_count}\n')

            for node, label in self.nodes.data('label', default=''):
                gfile.write(f"{node} {label}\n")

            #Writing edges
            edge_labels = nx.get_edge_attributes(self, 'label')
            for source in sorted(self.nodes()):
                #listing outgoing edges sorted by destination
                outgoing = sorted(self.out_edges(source), key=lambda x: x[1])  # ordina per nodo destinazione
                
                #writing the number of edges
                gfile.write(f"{len(outgoing)}\n")

                #writing edges
                for u, v in outgoing:
                    edge_label = edge_labels.get((u, v), '')
                    gfile.write(f"{u} {v} {edge_label}\n")

