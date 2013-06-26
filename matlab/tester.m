function mcncTester()

%if matlab shoots some glibc not found error, check out this thread:
%http://ubuntuforums.org/showthread.php?t=1809300
%basically solved by renaming the offending files in matlab/sys/os/glnx86.
%do NOT make links between files, as has been suggested. no need to.


%This is a script to run the 20 MCNC test circuits on some specified
%architecture. Can also vary some parameters in the specified architecture



%% Paths
vtrPath = '/home/oleg/Documents/work/UofT/Grad/my_vtr';     %path to vtr folder
tasksPath = [vtrPath '/vtr_flow/tasks'];                                    %run all .blif circuits here
architecture = 'k6_frac_N10_mem32K_40nm_mine.xml';
archPath = [vtrPath '/vtr_flow/arch/timing/' architecture];  
%architecture = 'k4_n4_v7_bidir.xml';                                        %architecture Tim used
%archPath = [vtrPath '/vtr_flow/arch/mine/' architecture];
outLogPath = [tasksPath '/regression_mcnc'];                                %read results from this output



%% Test Variables
defaultFcIn = 0.50;        %default
fcOutLow = 0.20;%0.05;
fcOutHigh = 0.20;%1;
fcOutStep = 0.05; %0.025

taskToRun = 'regression_mcnc';

%These have to match the order of the parsed metrics in the parse file used
%by vtr_parse_task.
metrics = { 
            'High Stress Delay',...
            'Low Stress Delay',...
            'Low Stress Wirelength',...
            'Low Stress W',...
            'LS io PH',...
            'LS clb PH',...
            'LS mult36 PH',...
            'LS memory PH',...
            'LS FPGA PH',...
            'LS io WH',...
            'LS clb WH',...
            'LS mult36 WH',...
            'LS memory WH',...
            'LS FPGA WH'
          };


%% Main Code
disp(['Architecture: ' archPath]);

%delete the existing run* folders
system(['rm ' outLogPath '/run* -R']);

%run mcnc tests for various Fc_out values
tic
i = 0;
for fcOut=fcOutLow:fcOutStep:fcOutHigh
    i = i + 1;
    fprintf('\n\n');
    disp(['Run ' num2str(i)]);
    disp(['Fc_in=' num2str(defaultFcIn) ' Fc_out=' num2str(fcOut)]);
    
    %update Fc value in the architecture file
    replaceFcInArchFile(archPath, defaultFcIn, fcOut);
    
    %run the benchmark circuits
    system([vtrPath '/vtr_flow/scripts/run_vtr_task.pl ' taskToRun]);
    
    %parse the output
    system([vtrPath '/vtr_flow/scripts/parse_vtr_task.pl ' taskToRun]);
    
    %read the output into Matlab
    res(i,:) = parseOutput([outLogPath '/run' num2str(i) '/parse_results.txt'], metrics);
end

%now plot the results
figure()
plot((fcOutLow:fcOutStep:fcOutHigh), res)
title('Delay vs Fc Out');
legend(metrics);


toc


%% Helper Functions

%A function to replace the clb fc_in and fc_out with the specified values
%in the specified (full) path
function result = replaceFcInArchFile(archPath, fcIn, fcOut)
    result = false;
    
    %we will be using regex
    clbRegex = '.*pb_type name="clb".*';
    fcRegex = 'fc default_in_type="frac" default_in_val="\d\.*\d*" default_out_type="frac" default_out_val="\d\.*\d*"';
    newLine = ['fc default_in_type="frac" default_in_val="' num2str(fcIn) '" default_out_type="frac" default_out_val="' num2str(fcOut) '"'];
    
    %read in current arch file
    oldArch = fileread(archPath);
    if isempty(oldArch)
       error('nothing in architecture'); 
    end
    
    %replace the first occurence of fc, after the clbRegex line, with the
    %new fc values:
    newArch = regexprep(oldArch,['(?<=' clbRegex ')' fcRegex],newLine,'once'); %using (?<=a)b format for positive lookbehind
    if isempty(newArch)
       error('couldnt regex the arch file'); 
    end

    %write the new architecture text to file -- file completely replaced
    fid = fopen(archPath,'w');    %open, discarding current contents
    if fid < 0
       error('couldnt open arch file'); 
    end

    fprintf(fid, '%s', newArch);
    fclose(fid);
    
    result = true;
    return;
end

%This function parses the output log file of VPR using regex to get the
%desired stats for current vpr run. Averages the high and low stress delays
%over all test circuits
function resultArray = parseOutput(outLogPath, metrics)
    
    numMetrics = length(metrics);
    
    %will be taking geometric avg. so initialize to 1
    resultArray = ones(numMetrics,1);
    allZeros = ones(numMetrics,1);
    
    %open file
    fid = fopen(outLogPath,'r');
    if fid < 0
       error('couldnt open the outputs file'); 
    end
    
    %parse file into resultArray
    j = 0;
    while true
        newLine = fgetl(fid);
        if ~ischar(newLine)
           %warning('parseOutput: reached end of file'); %#ok<*WNTAG>
           break;
        end
        
        results = regexp(newLine, '\s+-*(\d+\.*\d*)','match');
        if isempty(results)
           continue; 
        end
        
        %convert from string to number and load into resultArray
        for k = 1:numMetrics
            value = str2double(strtrim(results{k}));
            if value == -1
               %sometimes a result of a rare error in vpr. occasionally a
               %high-stress routing will work, but a low stress will not.
               if k > 1
                  value = nthroot(resultArray(k), j);
               else   
                  value = 1;
               end
               warning('parseOutput: got a -1 for a result!!! Fixing to average of previous circuits');
            end
            %we don't want a single 0 to spoil the geometric awerage.
            %but we still want to show the effect of this low value on the
            %overall average.
            if 0 ~= value
                resultArray(k) = resultArray(k) * value;
                allZeros(k) = 0;
            elseif allZeros(k) == 0;
                resultArray(k) = resultArray(k) * nthroot(resultArray(k),j) / 2;
            else
                %nothing
            end
        end
        
        j = j + 1;
    end
    
    %if all values of kth metric were 0, we'd like the average to be 0
    for k = 1:numMetrics
       if allZeros(k)
          resultArray(k) = 0; 
       end
    end
    
    %get the geometric averages
    for k = 1:numMetrics
       resultArray(k) = nthroot(resultArray(k), j);
       disp([metrics{k} ' = ' num2str(resultArray(k))]);
    end
    
    %close file
    fclose(fid);
    return;
end



end