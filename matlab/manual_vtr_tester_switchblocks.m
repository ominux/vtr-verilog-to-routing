
function manual_vtr_tester_switchblocks()

	tic

	%initialize tester class
	t = Tester();

	t.architecture = 'k6_frac_N10_mem32K_40nm_mine.xml';
	t.archPath = [t.vtrPath '/vtr_flow/arch/timing/' t.architecture];

	%variable setup
	benchmarks_dir = [t.vtrPath '/vtr_flow/benchmarks/vtr_benchmarks_blif/'];
	%'LU32PEEng',...                 %huge circuit. also slow
	%'stereovision2',...		 %also a huge circuit. takes ~24 hours	
	%benchmark_list = {'bgm',...
	%'blob_merge',...
	%'boundtop',...
	%'ch_intrinsics',...
	%'diffeq1',...
	%'diffeq2',...
	%'LU8PEEng',...
	%'mcml',...
	%'mkDelayWorker32B',...
	%'mkPktMerge',...
	%'mkSMAdapter4B',...
	%'or1200',...
	%'raygentop',...
	%'sha',...
	%'stereovision0',...
	%'stereovision1',...
	%'stereovision3'};



	%TODO: test
	benchmark_list = {'ch_intrinsics',...
	'sha'};


	benchmark_list_path = strcat(benchmark_list, '.blif');
	benchmark_list_path = strcat(benchmarks_dir, benchmark_list_path);
	numCkts = length(benchmark_list_path);
	arch = t.archPath;

	vprBaseOptions = '-nodisp';
	vprOptionsFullFlow = vprBaseOptions;

	labels = { 
		    'Low Stress Delay',...
		    'Chan Width',...
		    'Low Stress Wirelength',....
		    'Per-tile LB Routing Area'
		  };

	%parseRegex ordering has to match labels ordering
	parseRegex = {
			'Final critical path: (\d*\.*\d*)',...
			'channel width factor of (\d+)',... 
			'Total wirelength: (\d+)',...
			'\s*Assuming no buffer sharing \(pessimistic\)\. Total: \d+\.\d+e\+\d+, per logic tile: (\d+)'
		     };
			%'\s*Total routing area: .*, per logic tile: (\d+)'

		 
	% run from Fc_out = 0.1 to Fc_out = 0.3 in steps of 0.1         
	metricRange = 0.10:0.10:0.10;

	%if we use 4 workers, we can run 2 jobs in parallel; and it probably wont affect speed that much cause one of the big circuits
	% is the real bottleneck the other 3 workers will eventually have to wait for 
	matlabpool open 3;

	% make VPR
	t.replaceSingleLineInFile('/*#define TEST_METRICS', '//#define TEST_METRICS', t.rrGraphPath);
	t.makeVPR();

	i = 0;
	for metric = metricRange
	    i = i + 1;  
	    
	    result = t.replaceFcInArchFile(arch, 0.15, metric);
	    
	    lowStressW = 0;
	    %get low stress channel widths then get delay at low stress W
	    parfor ickt = 1:numCkts
		benchmark = benchmark_list_path{ickt};

		pause(ickt);
		vprString = [arch ' ' benchmark ' ' vprBaseOptions];
		vprOut = t.runVprManual(vprString);

		%get min chan width
		minW = t.regexLastToken(vprOut, '.*channel width factor of (\d+).');
		minW = str2double(minW);
		lowStressW(ickt) = floor(minW*1.3);

		%for a unidir architecture, we can only route with even channel width
		if mod(lowStressW(ickt), 2) == 1
			lowStressW(ickt) = lowStressW(ickt) + 1;
		end

		display(['ckt: ' benchmark_list{ickt} ' hi-stress W: ' num2str(minW) '  low-stress W: ' num2str(lowStressW(ickt))]);
		
		%now get delay at the low stress W
		vprString = [arch ' ' benchmark ' ' vprBaseOptions ' -route_chan_width ' num2str(lowStressW(ickt))];
		vprOut = t.runVprManual(vprString);
		temp_parfor_var = 0;
		for imetric = 1:length(parseRegex)
		   temp_parfor_var(imetric) = str2double(t.regexLastToken(vprOut, parseRegex{imetric}));
		end
		adjustedCktMetrics(ickt, :) = temp_parfor_var;
		
		benchmark_data{i, ickt, :} = [benchmark_list{ickt}, metric, num2cell(temp_parfor_var)]
	    end
	    
	    %now have to compute the geometric average
	    adjustedAvgMetrics(i,:) = geomean(adjustedCktMetrics,1);
	end

	%matlabpool('close');

	%print data to file
	%baselineAvgMetrics = [metricRange' baselineAvgMetrics];
	labels = ['metric' labels];
	%and then print
	%t.printDataToFile('./run_metrics.txt', baselineAvgMetrics, labels, false);

	adjustedAvgMetrics = [metricRange' adjustedAvgMetrics];
	append = false;

	cell_matrix = create_cell_matrix_for_print(benchmark_data, adjustedAvgMetrics, labels);

	t.printCellMatrixToFile('./vpr_run_results.txt', cell_matrix, append);
	%t.printDataToFile('./vpr_run_results.txt', adjustedAvgMetrics, labels, true);

	toc

	exit;

end


%builds the cell matrix that will be used for printing our data out to a file
function cell_matrix = create_cell_matrix_for_print(benchmark_data, geomean_data, labels)
	%first, want to stick the labels into the matrix.
	cell_matrix = [];
	cell_matrix = cat(1, cell_matrix, [' ', labels]);
	%cell_matrix{1, :} = [' ',  cellstr(labels)];


	matrix_entry = 2;

	%next we add all the individual benchmark data. here num_metrics refers to the
	[num_data_points, num_ckts, entries_in_row] = size(benchmark_data);
	for ipoint = 1:num_data_points
		for ickt = 1:num_ckts
			%cell_matrix{matrix_entry, :} = benchmark_data{ipoint, ickt, :};
			%matrix_entry = matrix_entry + 1;
			cell_matrix = cat(1, cell_matrix, num2cell(benchmark_data{ipoint, ickt, :}));
		end
		matrix_entry = matrix_entry + 1;
		%cell_matrix{matrix_entry, entries_in_row} = [];
	end

	cell_matrix = cat(1, cell_matrix, [' ', labels]);

	%and lastly we want to add all the geomean values
	[num_data_points] = size(geomean_data);
	for ipoint = 1:num_data_points
		%cell_matrix{matrix_entry, :} = ['geomean', num2cell(geomean_data(ipoint, :))];
		%matrix_entry = matrix_entry + 1;
		cell_matrix = cat(1, cell_matrix, ['geomean', num2cell(geomean_data(ipoint, :))]);
	end
end

