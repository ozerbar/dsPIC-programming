f_s = 1000; 

f_n = f_s / 2; 



Wp = 30 / f_n; % Passband frequency (normalized)

Ws = 20 / f_n; % Stopband frequency (normalized)

Rp = 3; % Passband ripple in dB

Rs = 40; 



[n, Wn] = buttord(Wp, Ws, Rp, Rs);

[b, a] = butter(n, Wn);



signal = readmatrix("signal.csv")



filtered_signal = filter(b, a, signal);



figure1;

plot(t, signal_combined, 'b', 'DisplayName', 'Original Signal');

hold on;

plot(t, filtered_signal, 'r', 'DisplayName', 'Filtered Signal');

hold off;

title('Comparison of Original and Filtered Signals');

xlabel('Time (seconds)');

ylabel('Amplitude');

legend;











signal = signal - mean(signal);





n = length(signal); 

f_signal = fft(signal);





P2 = abs(f_signal/n); 

P1 = P2(1:n/2+1); 

P1(2:end-1) = 2*P1(2:end-1); 







f = f_s*(0:(n/2))/n;





figure2; 

plot(f, P1);

title('Single-Sided Amplitude Spectrum of the Signal');

xlabel('Frequency (f)');

ylabel('|P1(f)|');









harmonicFrequency = findHarmonicFrequency(signal, fs);



fprintf('The frequency of the harmonic signal is: %f Hz\n', harmonicFrequency);



function harmonicFreq = findHarmonicFrequency(signal, fs)

   

    n = length(signal);



   

    Y = fft(signal);



    

    P2 = abs(Y/n);



    

    P1 = P2(1:n/2+1);

    P1(2:end-1) = 2*P1(2:end-1);



    

    f = fs*(0:(n/2))/n;



    

    [~, maxIndex] = max(P1);

    harmonicFreq = f(maxIndex);

end








