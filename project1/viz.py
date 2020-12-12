
import pydot
import imgcat



with open("out.dot") as f:

    tmp = ""
    i = 4

    for line in f.readlines():
        if line == "digraph { //c \n" and tmp != "":
            dot = pydot.graph_from_dot_data(tmp)[0]
            imgcat.imgcat(dot.create_png())

            tmp = ""
            i -= 1


        tmp += line

        if i == 0:
            break



