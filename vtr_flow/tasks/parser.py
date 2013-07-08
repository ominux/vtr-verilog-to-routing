import os
import re

def natural_sort(l): 
    convert = lambda text: int(text) if text.isdigit() else text.lower() 
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ] 
    return sorted(l, key = alphanum_key)

def create_files(vals, percent_cut, constants, filename, circuits):
    f = open("%s.txt" % (filename), 'w')
    i = 0
    while (i < len(vals)):
        f.write("\n\nConstant = %f\n\n" % (constants[i]))
        k = len(vals)
        for j in xrange(i+1, k):
            if(constants[j] != constants[i]):
                k = j
                break

        f.write("\t")
        for j in xrange(i, k):
            f.write("%d\t" % (percent_cut[j]))
        f.write("\n")

        for l in xrange(len(circuits)):
            f.write("%s\t" % (circuits[l]))
            for j in xrange(i, k):
                f.write("%s\t" % (vals[j][l]))
            f.write("\n")
        i = k

    f.close()

def main():
    #directory of the script
    dire = "/home/andre/VPR/pereira/vtr_flow/tasks/timing_small"
    #parameters to be analyzed

    runs = os.listdir(dire)
    runs = natural_sort(runs)

    min_chan = []
    path_delay = []
    percent_cut = []
    constants = []
    circuits = []

    for run in runs:
        folder = dire + "/" + run
        if(str(run)[0] != 'r'):
            continue
        try:
            f = open("%s/parse_results.txt" % (folder), 'r')
        except Exception, e:
            break

        l = f.read().split('\n')
        f.close()

        tmp_chan = []
        tmp_delay = []
        percent = -1
        constant = -1.0
        circuits = []

        titles = l[0].split('\t')
        #circuit, min_chan_width, critical_path_delay, constant, percent_wires_cut
        for i in xrange(1, len(l)):
            items = l[i].split('\t')
            for j in xrange(len(items)):
                if(titles[j] == "circuit"):
                    circuits.append(items[j])
                if(titles[j] == 'min_chan_width'):
                    tmp_chan.append(items[j])
                if(titles[j] == 'critical_path_delay'):
                    tmp_delay.append(items[j])
                if(titles[j] == 'constant'):
                    constant = float(items[j])
                if(titles[j] == 'percent_wires_cut'):
                    percent = int(items[j])

        percent_cut.append(percent)
        constants.append(constant)
        min_chan.append(tmp_chan)
        path_delay.append(tmp_delay)

    create_files(min_chan, percent_cut, constants, "minimum_channel_width", circuits)

    create_files(path_delay, percent_cut, constants, "critical_path_delay", circuits)

main()
