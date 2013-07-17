tic

%initialize tester class
t = Tester();

%variable setup
benchmarks_dir = [t.vtrPath '/vtr_flow/benchmarks/blif/wiremap6/'];
benchmark_list = {'alu4',...
'apex2',...
'apex4',...
'bigkey',...
'clma',...
'des',...
'diffeq',...
'dsip',...
'elliptic',...
'ex1010',...
'ex5p',...
'frisc',...
'misex3',...
'pdc',...
's298',...
's38417',...
's38584.1',...
'seq',...
'spla',...
'tseng'};
benchmark_list = strcat(benchmark_list, '.pre-vpr.blif');
benchmark_list = strcat(benchmarks_dir, benchmark_list);
numCkts = length(benchmark_list);
arch = t.archPath;

vprBaseOptions = '-nodisp';
vprOptionsFullFlow = vprBaseOptions;

labels = { 
            'Low Stress Delay',...
            'Low Stress Wirelength',....
            'LS clb PD',...
            'LS clb WH',...
            'LS clb HD',...
            'LS clb HP',...
            'LS clb PH'
          };

%parseRegex ordering has to match labels ordering
parseRegex = {
                'Final critical path: (\d*\.\d*)',...
                'Total wirelength: (\d+)',...
                'clb\s+Pin Diversity:\s+(\d*\.\d*)',...
                'clb\s+Pin Diversity:\s+\d*\.\d*\s+Wire Homogeneity:\s+(\d*\.\d*)',...
                'clb\s+Pin Diversity:\s+\d*\.\d*\s+Wire Homogeneity:\s+\d*\.\d*\s+Hamming Distance:\s+(\d*\.\d*)',...
                'clb\s+Pin Diversity:\s+\d*\.\d*\s+Wire Homogeneity:\s+\d*\.\d*\s+Hamming Distance:\s+\d*\.\d*\s+Hamming Proximity:\s+(\d*\.\d*)',...
                'clb\s+Pin Diversity:\s+\d*\.\d*\s+Wire Homogeneity:\s+\d*\.\d*\s+Hamming Distance:\s+\d*\.\d*\s+Hamming Proximity:\s+\d*\.\d*\s+Pin Homogeneity:\s+(\d*\.\d*)'
             };

%goal is to run through each benchmark
%do a full route - get metrics and stuff
%then do a route only at previous chan width - get metrics

%we can then do this at different Fc values, etc.

metricRange = 0.0 : 0.025 : 0.55;

%get low stress channel widths for each circuit
for ickt = 1:numCkts
    benchmark = benchmark_list{ickt};
        
    %switch to full flow
    t.replaceSingleLineInFile('/*#define TEST_METRICS', '//#define TEST_METRICS', t.rrGraphPath);
    t.makeVPR();
    vprString = [arch ' ' benchmark ' ' vprBaseOptions];
    vprOut = t.runVprManual(vprString);

    %get min chan width
    minW = t.regexLastToken(vprOut, '.*channel width factor of (\d+).');
    minW = str2double(minW);

    %get low stress chan width
    lowStressW(ickt) = floor(1.3 * minW);
end

i = 0;
for metric = metricRange
    i = i + 1;
    disp(['Run: ' num2str(i) '   Metric: ' num2str(metric)]);
    
    %this is the inner loop
    baselineCktMetrics = 0;
    adjustedCktMetrics = 0;
    for ickt = 1:numCkts
        benchmark = benchmark_list{ickt};

        %rerun at low stress to generate placement
        t.replaceSingleLineInFile('/*#define TEST_METRICS', '//#define TEST_METRICS', t.rrGraphPath);
        t.makeVPR();
        vprString = [arch ' ' benchmark ' ' vprBaseOptions ' -route_chan_width ' num2str(lowStressW(ickt))];
        vprOut = t.runVprManual(vprString);
        %now parse
        for imetric = 1:length(parseRegex)
           baselineCktMetrics(ickt, imetric) =  str2double(t.regexLastToken(vprOut, parseRegex{imetric}));
        end

        %rerun with metric adjustment enabled
        t.replaceSingleLineInFile('target_metric = \d*\.*\d+;', ['target_metric = ' num2str(metric) ';'], t.rrGraphPath);
        t.replaceSingleLineInFile('/+#define TEST_METRICS', '#define TEST_METRICS', t.rrGraphPath);
        t.makeVPR();
        vprString = [arch ' ' benchmark ' ' vprBaseOptions ' -route_chan_width ' num2str(lowStressW(ickt)) ' -route'];
        vprOut = t.runVprManual(vprString);
        for imetric = 1:length(parseRegex)
           adjustedCktMetrics(ickt, imetric) =  str2double(t.regexLastToken(vprOut, parseRegex{imetric}));
        end
    end
    %now have to compute the geometric average
    baselineAvgMetrics(i,:) = geomean(baselineCktMetrics,1);
    adjustedAvgMetrics(i,:) = geomean(adjustedCktMetrics,1);
end

%print data to file
baselineAvgMetrics = [metricRange' baselineAvgMetrics];
labels = ['metric' labels];
%and then print
t.printDataToFile('./run_metrics.txt', baselineAvgMetrics, labels, false);

adjustedAvgMetrics = [metricRange' adjustedAvgMetrics];
t.printDataToFile('./run_metrics.txt', adjustedAvgMetrics, labels, true);

toc