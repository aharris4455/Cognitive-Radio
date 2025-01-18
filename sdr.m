%% Frequency hopping for RTL-SDR
clear; 
fs = 2.4e6;
threshold = -30; % PSD Threshold for detection, dB
iterTime = 0.001; % Time in seconds for one iteration
%centerFreq = [939515400 941515400 943515400 945515400 947515400]; %GSM freq
%centerFreq = 433000000:2e6:1700000000; 
centerFreq = 222e6;
%centerFreq = 433000000;
%centerFreq =  949200000;
%centerFreq = 857000000;
%795 000 000; 
rx = comm.SDRRTLReceiver("CenterFrequency",centerFreq(1), ...
                         "SampleRate", fs, "OutputDataType", "double", ...
                         "SamplesPerFrame", 1024);
scope = spectrumAnalyzer('SampleRate', fs);
try
    while true
        for i = centerFreq
            rx.CenterFrequency = i;
            startTime = tic;
            while toc(startTime) < iterTime
                [rxSignal, ~] = rx();
                [psd, ~] = pwelch(rxSignal, hamming(1024),[],[],fs,'power');
                avgPSD = 10*log10(mean(psd));
                disp(['PSD of Channel: ', num2str(i/1e6), ...
                            'MHz = ', num2str(avgPSD)]);
                if avgPSD < threshold 
                    disp(['Channel: ',num2str(i/1e6), ' Free']);
                    toggleSwitch = 0;
                else
                    disp(['Channel: ', num2str(i/1e6) ,' in use']);
                    toggleSwitch = 1;
                end
                scope.Title = ['Freq: ', num2str(i/1e6), 'MHz'];
                scope(rxSignal)
            end % Toc loop
        end % Freq loop
    end % Sim loop
catch ME
    disp(ME)
    release(scope);
    release(rx);
end 

