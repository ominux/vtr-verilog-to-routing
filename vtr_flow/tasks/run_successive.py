import os

def main():
    vals = [0, 10, 20, 30, 40, 50, 60, 70, 80, 85, 90]
    print "Enter initial value for percent_wires_cut"
    init = int(raw_input())
    i = 0
    for j in xrange(len(vals)):
        if(vals[j] >= init):
            i = j
            break

    for p in xrange(i, len(vals)):
        print "Running with percent_wires_cut=%d" % (vals[p])
        os.system("../scripts/run_vtr_task.pl timing_small -p 4 -percent_wires_cut %d -num_cuts 1 -delay_increase 1000" % (vals[p]))
        os.system("../scripts/parse_vtr_task.pl timing_small")
        print "Finished running with percent_wires_cut=%d" % (vals[p])



main()
