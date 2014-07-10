tic

%initialize tester class
t = Tester();

%variable setup
%benchmarks_dir = [t.vtrPath '/vtr_flow/benchmarks/blif/wiremap6/'];
%benchmark_list = strcat(benchmark_list, '.pre-vpr.blif');
%benchmark_list = strcat(benchmarks_dir, benchmark_list);

benchmarks_dir = [t.vtrPath '/vtr_flow/benchmarks/vtr_benchmarks_blif/'];
benchmark_list = {'bgm',...
'blob_merge',...
'boundtop',...
'LU8PEEng',...
'mkDelayWorker32B',...
'mkSMAdapter4B',...
'or1200',...
'raygentop',...
'sha',...
'stereovision0',...
'stereovision1'};
benchmark_list = strcat(benchmark_list, '.blif');
benchmark_list = strcat(benchmarks_dir, benchmark_list);


numCkts = length(benchmark_list);
arch = t.archPath;

vprBaseOptions = '-nodisp';
vprOptionsFullFlow = vprBaseOptions;

labels = { 
            'Low Stress Delay',...
            'Chan Width',...
            'Low Stress Wirelength',....
            'LS clb PD',...
            'LS clb WH',...
            'LS clb HD',...
            'LS clb HP',...
            'LS clb PH'
          };

%parseRegex ordering has to match labels ordering
parseRegex = {
                'Final critical path: (\d*\.*\d*)',...
                'channel width factor of (\d+)',...
                'Total wirelength: (\d+)',...
                'clb\s+Pin Diversity:\s+(\d*\.*\d*)',...
                'clb\s+Pin Diversity:\s+\d*\.*\d*\s+Wire Homogeneity:\s+(\d*\.*\d*)',...
                'clb\s+Pin Diversity:\s+\d*\.*\d*\s+Wire Homogeneity:\s+\d*\.*\d*\s+Hamming Distance:\s+(\d*\.*\d*)',...
                'clb\s+Pin Diversity:\s+\d*\.*\d*\s+Wire Homogeneity:\s+\d*\.*\d*\s+Hamming Distance:\s+\d*\.*\d*\s+Hamming Proximity:\s+(\d*\.*\d*)',...
                'clb\s+Pin Diversity:\s+\d*\.*\d*\s+Wire Homogeneity:\s+\d*\.*\d*\s+Hamming Distance:\s+\d*\.*\d*\s+Hamming Proximity:\s+\d*\.*\d*\s+Pin Homogeneity:\s+(\d*\.*\d*)'
             };

%goal is to run through each benchmark
%do a full route - get metrics and stuff
%then do a route only at previous chan width - get metrics

%we can then do this at different Fc values, etc.

metricRange = 0.0 : 0.1 : 0.6;
 
matlabpool open 5;

%generate initial placement using normal connection blocks
t.replaceSingleLineInFile('boolean test_metrics = \w+;', 'boolean test_metrics = FALSE;', t.globalsPath);
t.replaceSingleLineInFile('boolean manage_trackmap = \w+;', 'boolean manage_trackmap = FALSE;', t.globalsPath)
t.makeVPR();
i = 0;
parfor ickt = 1:numCkts
        disp(['Gen Run: ' num2str(ickt) '  Circuit: ' num2str(ickt)]);
        benchmark = benchmark_list{ickt};

         %generate placement using normal connection blocks
           vprString = [arch ' ' benchmark ' ' vprBaseOptions ' -pack -place'];
           vprOut = t.runVprManual(vprString);
end



i = 0;
for metric = metricRange
    i = i + 1;
    
    baselineCktMetrics = 0;
    adjustedCktMetrics = 0;
    t.replaceSingleLineInFile('target_metric = \d*\.*\d+;', ['target_metric = ' num2str(metric) ';'], t.rrGraphPath);
    t.replaceSingleLineInFile('boolean test_metrics = \w+;', 'boolean test_metrics = TRUE;', t.globalsPath);
    %t.replaceSingleLineInFile('boolean manage_trackmap = \w+;', 'boolean manage_trackmap = TRUE;   ', t.globalsPath);
    t.makeVPR();

    adjustedCktMetrics = zeros(numCkts, length(parseRegex));
    %this is the inner loop
    adjustedCktMetrics = zeros(numCkts, length(parseRegex));
    parfor ickt = 1:numCkts
        disp(['Run: ' num2str(i) '   Metric: ' num2str(metric) '  Circuit: ' num2str(ickt)]);
        benchmark = benchmark_list{ickt};

        vprString = [arch ' ' benchmark ' ' vprBaseOptions ' -route'];% ' -route_chan_width 300'];
        vprOut = t.runVprManual(vprString);
	adjustedCktMetrics_tmp = 0;
        for imetric = 1:length(parseRegex)
           adjustedCktMetrics_tmp(imetric) =  str2double(t.regexLastToken(vprOut, parseRegex{imetric}));
		end
		
        adjustedCktMetrics(ickt, :) =  adjustedCktMetrics_tmp;
    end
    %now have to compute the geometric average
    baselineAvgMetrics(i,:) = geomean(baselineCktMetrics,1);
    adjustedAvgMetrics(i,:) = geomean(adjustedCktMetrics,1);
    adjustedAvgMetrics(i,1)
    adjustedAvgMetrics(i,2)
end




%print data to file
%baselineAvgMetrics = [metricRange' baselineAvgMetrics];
labels = ['metric' labels];
%and then print
%t.printDataToFile('./run_metrics.txt', baselineAvgMetrics, labels, false);

adjustedAvgMetrics = [metricRange' adjustedAvgMetrics];
t.printDataToFile('./conn_block_routability_vtr3.txt', adjustedAvgMetrics, labels, false); %CHANGE BACK TO TRUE!

toc
