
%if matlab shoots some glibc not found error, check out this thread:
%http://ubuntuforums.org/showthread.php?t=1809300
%basically solved by renaming the offending files in matlab/sys/os/glnx86.
%do NOT make links between files, as has been suggested. no need to.


%This is a script to run the 20 MCNC test circuits on some specified
%architecture. Can also vary some parameters in the specified architecture


%% Test Variables
defaultFcIn = 0.50;        %default
fcOutRange = 0.1 : 0.05 : 0.5;

taskToRun = 'regression_mcnc';

%These have to match the order of the parsed metrics in the parse file used
%by vtr_parse_task.
metrics = { 
            'High Stress Delay',...
            'Low Stress Delay',...
            'Low Stress Wirelength',...
            'Low Stress W',...
            'LS clb PD',...
            'LS clb WH',...
            'LS clb HD'
          };


%% Main Code
t = Tester();  % initialize t class

disp(['Architecture: ' t.archPath]);

%delete the existing run* folders
system(['rm ' t.outLogPath '/run* -R']);

%run mcnc tests for various Fc_out values
tic
i = 0;
irun = 0;
for fcOut = fcOutRange
    i = i + 1;
    irun = irun + 1;
    fprintf('\n\n');
    disp(['Run ' num2str(i)]);
    disp(['Fc_in=' num2str(defaultFcIn) ' Fc_out=' num2str(fcOut)]);
    
    %update Fc value in the architecture file
    t.replaceFcInArchFile(t.archPath, defaultFcIn, fcOut);
    
    %algorithm1 - full flow
    t.switchToAlgorithm1(t.rrGraphPath);
    t.switchToRouteChan(t.configPath, 80);
    %switchToFullFlow(configPath);
    t.makeVPR();
    t.runVtrTask(taskToRun);
    t.parseVtrTask(taskToRun);
    alg1Res(i,:) = t.parseOutput([t.outLogPath '/run' num2str(irun) '/parse_results.txt'], metrics, false);
    
    irun = irun + 1;
    
    %Is there no way to route two circuits at the same chan width???
    %focus on PD vs HD tests for a single circuit then...
    
    %algorithm2 - route only
    t.switchToAlgorithm2(t.rrGraphPath);
    t.switchToRouteChan(t.configPath, 80);
    t.makeVPR();
    t.runVtrTask(taskToRun);
    t.parseVtrTask(taskToRun);
    alg2Res(i,:) = t.parseOutput([t.outLogPath '/run' num2str(irun) '/parse_results.txt'], metrics, false);
    
end


%plot the data
figure()
plot((fcOutRange), alg1Res)
title('Delay vs Fc Out Algorithm1');
legend(metrics);

figure()
plot((fcOutRange), alg2Res)
title('Delay vs Fc Out Algorithm2');
legend(metrics);

%and print data to files
%first prepend Fc_out to labels and data
alg1Res = [fcOutRange' alg1Res];
alg2Res = [fcOutRange' alg2Res];
metrics = ['Fc_out' metrics];
%and then print
t.printDataToFile('./run.txt', alg1Res, metrics, false);
t.printDataToFile('./run.txt', alg2Res, metrics, true);


toc
