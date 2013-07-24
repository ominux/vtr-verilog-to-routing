%metricTester script

%process will be to: 
%regex new desired metric value
%make vpr
%run vpr through command shell - new
%parse the output of the command shell -new
%repeat
tic
metricRange = 0.0 : 0.0008 : 0.01;

labels = { 
            'High Stress Delay',...
            'Low Stress Delay',...
            'Low Stress Wirelength',...
            'Low Stress W',...
            'LS clb PD',...
            'LS clb WH',...
            'LS clb HD',...
            'LS clb HP',...
            'LS clb PH'
          };

t = Tester();   %instantiate tester class

%delete the existing run* folders
system(['rm ' t.outLogPath '/run* -R']);

%disable metric
t.replaceSingleLineInFile('/*#define TEST_METRICS', '//#define TEST_METRICS', t.rrGraphPath);
t.makeVPR();
%switch to route chan width
t.switchToRouteChan(t.configPath, 90);
%run first time
t.runVtrTask('regression_mcnc');
t.parseVtrTask('regression_mcnc');

i = 0;
numInnerIter = 1;
%avgResults = zeros(1, length(labels));
for metric = metricRange
    i = i + 1;
    disp(['Run ' num2str(i) '  Target Metric ' num2str(metric)]);

    %regex new metric
    t.replaceSingleLineInFile('target_metric = \d*\.*\d+;', ['target_metric = ' num2str(metric) ';'], t.rrGraphPath);
    %enable metric
    t.replaceSingleLineInFile('/+#define TEST_METRICS', '#define TEST_METRICS', t.rrGraphPath);
    t.makeVPR();
    t.switchToRouteOnly(t.configPath, 90);
    system(['rm ' t.outLogPath '/run* -R']);
    avgResults(i,1:length(labels)) = 0;
    for irun = 1:numInnerIter
        t.runVtrTask('regression_mcnc');
        t.parseVtrTask('regression_mcnc');
        results(irun, :) = t.parseOutput([t.outLogPath '/run' num2str(irun) '/parse_results.txt'], labels, true);
        disp(['Metric: ' num2str(results(irun, 7))]);
        disp(['Delay: ' num2str(results(irun, 2))]);
        avgResults(i,:) = avgResults(i,:) + results(irun,:);
    end
    avgResults(i,:) = avgResults(i,:) ./ numInnerIter;
    disp(['Avg Delay: ' num2str(avgResults(i,2))]);
    
end
%and print data to files
%first prepend Fc_out to labels and data
avgResults = [metricRange' avgResults];
labels = ['PD' labels];
%and then print
t.printDataToFile('./run_metrics.txt', avgResults, labels, false);

toc