
Mods = {'A' 'B' 'C' 'D' 'E' 'F'};

fid = fopen('taps.txt','wt');

for k = 1:6
    
    model =  Mods{k};
    
    switch model
        case 'A' %Flat channel
            tau = 0*1.0e-9; %tap delay in nanoseconds
            clusters = 0;
            %             Rice_K = 1;
        case 'B'
            tau = [0 10 20 30 40 50 60 70 80]*1.0e-9; %tap delay in nanoseconds
            clusters = [[0    -5.4 -10.8 -16.2 -21.7 -Inf  -Inf  -Inf  -Inf];...
                [-Inf -Inf -3.2  -6.3  -9.4  -12.5 -15.6 -18.7 -21.8]];
            %             Rice_K = 1;
        case 'C'
            tau = [0 10 20 30 40 50 60 70 80 90 110 140 170 200]*1.0e-9;
            clusters = [[0 -2.1 -4.3 -6.5 -8.6 -10.8 -13.0 -15.2 -17.3 -19.5 -Inf -Inf -Inf -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -5.0 -7.2 -9.3 -11.5 -13.7 -15.8 -18.0 -20.2]];
            %             Rice_K = 1;
        case 'D'
            tau = [0 10 20 30 40 50 60 70 80 90 110 140 170 200 240 290 340 390]*1.0e-9;
            clusters = [[0    -0.9 -1.7 -2.6 -3.5 -4.3 -5.2 -6.1 -6.9 -7.8 -9.0 -11.1 -13.7 -16.3 -19.3 -23.2 -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -6.6 -9.5  -12.1 -14.7 -17.4 -21.9 -25.5 -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf  -Inf  -Inf  -18.8 -23.2 -25.2 -26.7]];
            %             Rice_K = 2;  %K = P_LOS/P_Rayleigh
        case 'E'
            tau = [0 10 20 30 50 80 110 140 180 230 280 330 380 430 490 560 640 730]*1.0e-9;
            clusters = [[-2.6 -3.0 -3.5 -3.9 -4.5 -5.6 -6.9 -8.2 -9.8 -11.7 -13.9 -6.1  -18.3 -20.5 -22.9 -Inf  -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -1.8 -3.2 -4.5 -5.8 -7.1 -9.9  -10.3 -14.3 -14.7 -18.7 -19.9 -22.4 -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -7.9 -9.6  -14.2 -13.8 -18.6 -18.1 -22.8 -Inf  -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf  -Inf  -Inf  -Inf  -Inf  -20.6 -20.5 -20.7 -24.6]];
            %             Rice_K = 4;
        case 'F'
            tau = [0 10 20 30 50 80 110 140 180 230 280 330 400 490 600 730 880 1050]*1.0e-9;
            clusters = [[-3.3 -3.6 -3.9 -4.2 -4.6 -5.3 -6.2 -7.1 -8.2 -9.5  -11.0 -12.5 -14.3 -16.7 -19.9 -Inf  -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -1.8 -2.8 -3.5 -4.4 -5.3 -7.4  -7.0  -10.3 -10.4 -13.8 -15.7 -19.9 -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -5.7 -6.7  -10.4 -9.6  -14.1 -12.7 -18.5 -Inf  -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf  -Inf  -Inf  -8.8  -13.3 -18.7 -Inf  -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf  -Inf  -Inf  -Inf  -Inf  -12.9 -14.2 -Inf  -Inf];...
                [-Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf -Inf  -Inf  -Inf  -Inf  -Inf  -Inf  -Inf  -16.3 -21.2]];
            %             Rice_K = 4;
        otherwise
            error ('hsr_chan_multipath_etsi: invalid channel model');
    end
    
    power_dB = 10.*log10(sum(10.^(clusters./10),1));
    
    nTaps = length(power_dB);
    
    fprintf(fid,'\nunsigned nTaps_%s = %d; \ndouble tapsPow_%s[%d] = {',model,nTaps,model,nTaps);
    for j = 1:nTaps
        fprintf(fid,' %.6f',power_dB(j));
        if(j ~= nTaps)
            fprintf(fid,',');
        end
    end
    fprintf(fid,'};\n');
    
    fprintf(fid,'double tapDelay_%s[%d] = {',model,nTaps);
    for j = 1:nTaps
        fprintf(fid,' %.6d',tau(j));
        if(j ~= nTaps)
            fprintf(fid,',');
        end
    end
    fprintf(fid,'};\n');
    
end

fclose(fid);
