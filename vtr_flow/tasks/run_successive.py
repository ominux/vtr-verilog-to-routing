import os

def main():
    vals = [0, 10, 20, 30, 40, 50, 60, 70, 80, 90]
    delays = [0, 500, 1000]
    #vals = [0, 20, 40, 60, 80]

    for i in xrange(len(delays)):
        print "\nRunning with delay_increase = %d" % (delays[i])

        for p in xrange(len(vals)):
            print "Running with percent_wires_cut = %d" % (vals[p])
            os.system("../scripts/run_vtr_task.pl timing_small -p 4 -percent_wires_cut %d -num_cuts 3 -delay_increase %d" % (vals[p], delays[i]) )
            os.system("../scripts/parse_vtr_task.pl timing_small")
            print "Finished running with percent_wires_cut = %d" % (vals[p])

        print "\nFinished running with delay_increase = %d" % (delays[i])


main()
