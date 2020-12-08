
import pydot
import imgcat

for graph in pydot.graph_from_dot_file("out.dot"):
    imgcat.imgcat(graph.create_png())
