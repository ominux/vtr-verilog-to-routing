%metricTester script

%this script will have to change one thing - the desired value of the
%metric. simple line regex here.

%process will be to: 
%regex new desired metric value
%make vpr
%run vpr through command shell - new
%parse the output of the command shell -new
%repeat

metricRange = 0.65 : 0.3 : 0.95;
chanWidth = 60;

outputs = { 
            'High Stress Delay',...
            'Low Stress Delay',...
            'Low Stress Wirelength',...
            'Low Stress W',...
            'LS clb PD',...
            'LS clb WH',...
            'LS clb HD'
          };

t = Tester();   %instantiate tester class

%runVprRouteChan = [t.archPath ' ' circuit ' -nodisp -route_chan_width ' num2str(chanWidth)];
%runVprRouteOnly = [t.archPath ' ' circuit ' -nodisp -route_chan_width ' num2str(chanWidth) ' -route'];

%delete the existing run* folders
system(['rm ' t.outLogPath '/run* -R']);

i = 0;
numInnerIter = 5;
%avgResults = zeros(1, length(outputs));
for metric = metricRange
    i = i + 1;
    disp(['Run ' num2str(i) '  Target Metric ' num2str(metric)]);
    
    %regex new metric
    t.replaceSingleLineInFile('target_metric = \d+\.\d+;', ['target_metric = ' num2str(metric) ';'], t.rrGraphPath);
    %disable metric
    t.replaceSingleLineInFile('/*#define TEST_METRICS', '//#define TEST_METRICS', t.rrGraphPath);
    t.makeVPR();
    %switch to route chan width
    t.switchToRouteChan(t.configPath, 40);
    %run first time
    t.runVtrTask('regression_mcnc');
    t.parseVtrTask('regression_mcnc');
    %parse output for: delay, clb PD -- actually, do it later


    %enable metric
    t.replaceSingleLineInFile('/+#define TEST_METRICS', '#define TEST_METRICS', t.rrGraphPath);
    t.makeVPR();
    t.switchToRouteOnly(t.configPath, 40);
    system(['rm ' t.outLogPath '/run* -R']);
    avgResults(i,1:length(outputs)) = 0;
    for irun = 1:numInnerIter
        t.runVtrTask('regression_mcnc');
        t.parseVtrTask('regression_mcnc');
        i
        results(irun, :) = t.parseOutput([t.outLogPath '/run' num2str(irun) '/parse_results.txt'], outputs, true);
        results(irun, 5)
        avgResults(i,:) = avgResults(i,:) + results(irun,:);
    end
    avgResults(i,:) = avgResults(i,:) ./ numInnerIter;
    
    %run a bunch of times
    %parse output
    %get average
    
    
end
%and print data to files
%first prepend Fc_out to labels and data
avgResults = [metricRange' avgResults];
outputs = ['PD' outputs];
%and then print
t.printDataToFile('./run_metrics.txt', avgResults, outputs, false);

%write all the data to file