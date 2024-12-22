%% Frequency hopping for RTL-SDR
clear; 
centerFreq = [939515400 941515400 943515400 945515400 947515400]; %GSM freq
rx = comm.SDRRTLReceiver("CenterFrequency",centerFreq(1), ...
                         "SampleRate", 2.4e6, "OutputDataType", "double", ...
                         "SamplesPerFrame", 1024);
disp('Receiving GSM signal...');
scope = spectrumAnalyzer('SampleRate', 2.4e6);
disp('Displaying spectrum...');
try
    while true
        for i = centerFreq
            rx.CenterFrequency = i;
            disp(i);
            startTime = tic;
            while toc(startTime) < 1
                [rxSignal, ~] = rx();
                scope.Title = ['Freq: ', num2str(i/1e6), 'MHz'];
                scope(rxSignal)
            end
        end
    end
catch ME
    disp(ME)
    release(scope);
    release(rx);
end 

