clc
clear all
close all

%% With BA & Aggregation

offeDataBA = [1 5 10 15 20 25 26.6 28.3 29 30 35 40 45 50];
queOFBA = [0     0       0       0       0       ;... % 1
         0     0       0       0       0       ;... % 5
         0     0       0       0       0       ;... % 10
         0     0       0       0       0       ;... % 15
         0     0       0       0       0       ;... % 20
         0     0       0       0       0       ;... % 25
         0         0         0         0.018107 0         ;... % 26.6
         0.0048238 0.0038803 0         0.068626 0         ;... % 28.3
         0.0072150 0.0231047 0.014465  0.091028 0.025436  ;... % 29
         0.0578001 0.0289609 0.0381443 0.159399 0.032384  ;... % 30
         0.219064  0.193678  0.210463  0.307434 0.232985  ;... % 35
         0.349445  0.343308  0.327511  0.416533 0.339962  ;... % 40
         0.437923  0.435213  0.43513   0.494229 0.438403  ;... % 45
         0.510328  0.500577  0.504171  0.561192 0.506693  ];   % 50
   
m_queOFBA = mean(queOFBA,2);

%% Without BA & Aggregation

offeData = [1 5 10 15 20 21 22.5 24 25 27.5 30 35 40 45 50];
queOF = [0         0         0         0         0       ;... % 1
         0         0         0         0         0       ;... % 5
         0         0         0         0         0       ;... % 10
         0         0         0         0         0       ;... % 15
         0         0         0         0         0       ;... % 20
         0         0         0         0.035207  0         ;... % 21
         0.0372639 0.002973  0.013212  0.130975  0         ;... % 22.5
         0.102464  0.103204  0.070000  0.189734  0.068542  ;... % 24
         0.152573  0.150545  0.154974  0.245781  0.163641  ;... % 25
         0.265584  0.265752  0.257916  0.350914  0.268394  ;... % 27.5
         0.348943  0.3486    0.366736  0.427527  0.481651  ;... % 30
         0.473527  0.47607   0.469213  0.529148  0.477581  ;... % 35
         0.558452  0.555116  0.557277  0.606332  0.558102  ;... % 40
         0.621349  0.614327  0.612904  0.658735  0.620625  ;... % 45
         0.669026  0.664672  0.657649  0.700869  0.666379 ];    % 50
     
m_queOF = mean(queOF,2);

%% Plot

plot(offeDataBA,100.*m_queOFBA,'b-o','LineWidth',2);
hold on;
plot(offeData,100.*m_queOF,'r--*','LineWidth',2);
grid on;
xlabel('Taxa de dados oferecida a MAC [Mbps]');
ylabel('Oveflow [%]');
legend('Com agrega��o','Sem agrega��o','Location','NorthWest');
print('-dbmp','overVSOffer');

%% Save data

save data2