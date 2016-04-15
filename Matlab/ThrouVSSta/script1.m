clc
clear all
close all

%% With BA and Aggregation

nStasBA = [1 5 10 15];
thrBA = [0.515988 0.475575 0.530536 0.546736 0.479758 ;... % 1
         2.52798  2.46796  2.53471  2.62974  2.41167  ;... % 5
         4.04253  4.74237  4.75599  4.5435   4.42159  ;... % 10
         4.19597  4.71173  4.71865  4.55767  4.67476  ];   % 15
     
mThrBA = mean(thrBA,2)./nStasBA';
     
%% Plot

plot(nStasBA,mThrBA);