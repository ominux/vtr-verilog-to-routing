classdef Tester %< handle (or some other parent class)
    
   properties
       vtrPath = '/home/oleg/Documents/work/UofT/Grad/my_vtr';                     %path to vtr folder
       vprPath = [vtrPath '/vpr/'];
       tasksPath = [vtrPath '/vtr_flow/tasks'];                                    %run all .blif circuits here
       architecture = 'k6_frac_N10_mem32K_40nm_mine.xml';
       archPath = [vtrPath '/vtr_flow/arch/timing/' architecture];  
       outLogPath = [tasksPath '/regression_mcnc'];                                %read results from this output

       rrGraphPath = [vtrPath '/vpr/SRC/route/rr_graph.c'];
       configPath = [tasksPath '/regression_mcnc/config/config.txt'];
   end %properties
   
   
   
   methods
        %prints tab-delimited data to file. append=false means existing file
        %contents will be erased before printing.
        function printDataToFile(filepath, data, labels, append, obj)

            %make labels a column vector
            [labels_row, labels_col] = size(labels);
            num_labels = labels_row;
            if labels_col > labels_row
                labels = labels';
                num_labels = labels_col;
            end

            %open file and print the labels
            if (append)
               fopen_rw = 'a+'; 
            else
               fopen_rw = 'w+';
            end
            fid = fopen(filepath, fopen_rw);
            if (fid < 0)
               error('printDataToFile: couldnt open specified file'); 
            end
            fprintf(fid, '\n');                      %start with newline
            fprintf(fid, '%s\t', labels{1:end-1});  %tab-delimited
            fprintf(fid, '%s\n', labels{end});      %last label has newline
            fclose(fid);

            %now print the data       
            dlmwrite(filepath, data, '-append', 'delimiter', '\t');

            %should hopefully be done now...
        end

        function result = makeVPR(vprPath, obj)
            restoreDir = cd;
            cd(vprPath); 
            system('pwd');
            result = system('make');
            if (result ~= 0)
               error('makeVPR: failed to make VPR');
            else
               result = 1;
            end
            cd(restoreDir);
        end

        function result = switchToRouteChan(filePath, chanWidth, obj)
            lineRegex = 'script_params=-no_mem -starting_stage vpr.*';
            newLine = ['script_params=-no_mem -starting_stage vpr -vpr_route_chan_width ' num2str(chanWidth) ' -move_net_and_placement ../../../temp'];

            result = replaceSingleLineInFile(lineRegex, newLine, filePath);
            return; 
        end

        function result = switchToAlgorithm1(filePath, obj) 
            lineRegex = '/*#define MY_ALGORITHM';
            newLine = '//#define MY_ALGORITHM';

            result = replaceSingleLineInFile(lineRegex, newLine, filePath);
            return;
        end

        function result = switchToAlgorithm2(filePath, obj)
            lineRegex = '/+#define MY_ALGORITHM';
            newLine = '#define MY_ALGORITHM';

            result = replaceSingleLineInFile(lineRegex, newLine, filePath);
            return;
        end

        function result = switchToRouteOnly(filePath, chanWidth, obj)
            lineRegex = 'script_params=-no_mem -starting_stage vpr -vpr_route_chan_width \d+ -move_net_and_placement ../../../temp';
            newLine = ['script_params=-no_mem -starting_stage vpr -vpr_route_chan_width ' num2str(chanWidth) ' -route_from_net_and_placement ../../../temp'];

            result = replaceSingleLineInFile(lineRegex, newLine, filePath);
            return;
        end

        function result = switchToFullFlow(filePath, obj)
            lineRegex = 'script_params=-no_mem -starting_stage vpr.*';
            %lineRegex = 'script_params=-no_mem -starting_stage vpr -vpr_route_chan_width 80 -route_from_net_and_placement ../../../temp';
            newLine = 'script_params=-no_mem -starting_stage vpr -move_net_and_placement ../../../temp';

            result = replaceSingleLineInFile(lineRegex, newLine, filePath);
            return;
        end

        %A function to replace the clb fc_in and fc_out with the specified values
        %in the specified (full) path
        function result = replaceFcInArchFile(archPath, fcIn, fcOut, obj)
            result = false;

            %we will be using regex
            clbRegex = '.*pb_type name="clb".*';
            fcRegex = 'fc default_in_type="frac" default_in_val="\d\.*\d*" default_out_type="frac" default_out_val="\d\.*\d*"';
            newLine = ['fc default_in_type="frac" default_in_val="' num2str(fcIn) '" default_out_type="frac" default_out_val="' num2str(fcOut) '"'];

            lineRegex = ['(?<=' clbRegex ')' fcRegex];  %use positive lookbehind
                                                        %TODO: change to -ve
                                                        %lookahead

            result = replaceSingleLineInFile(lineRegex, newLine, archPath);

            return;
        end

        %uses regexprep to replace first line matching lineRegex with newLine
        function result = replaceSingleLineInFile( lineRegex, newLine, filePath, obj)
            %read in file
            oldFile = fileread(filePath);
            if isempty(oldFile)
               error('replaceParamInFile: file is empty'); 
            end

            %replace first matching line
            newFile = regexprep(oldFile, lineRegex, newLine, 'once');
            if isempty(newFile)
               error('replaceParamInFile: couldnt regex the file'); 
            end

            %write the new architecture text to file -- file completely replaced
            fid = fopen(filePath,'w');    %open, discarding current contents
            if fid < 0
               error('replaceParamInFile: couldnt open the file for replacement'); 
            end

            fprintf(fid, '%s', newFile);
            fclose(fid);

            result = true;
            return
        end

        %This function parses the output log file of VPR using regex to get the
        %desired stats for current vpr run. Averages the high and low stress delays
        %over all test circuits
        function resultArray = parseOutput(path, metrics, obj)

            numMetrics = length(metrics);

            %will be taking geometric avg. so initialize to 1
            resultArray = ones(numMetrics,1);
            allZeros = ones(numMetrics,1);

            %open file
            fid = fopen(path,'r');
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

       
   end %methods
    
end