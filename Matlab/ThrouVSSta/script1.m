clc
clear all
close all
%% General

offeData = 0.5;
simTime = 1.1;

%% Get Results

resFile = fileread('results.txt');
resNum = strfind(resFile,'%%%% Final results %%%%');
resStr = resFile(resNum:end);

staNum = strfind(resStr,'number of Stas =');
staLen = length('number of Stas =');
thrNum = strfind(resStr,'Throughput (Mbps)');
thrLen = length('Throughput (Mbps)');
meanNum = strfind(resStr,'mean           =');
meanNum = meanNum(1);
meanLen = length('mean           =');
confNum = strfind(resStr,'conf. interval =');
confNum = confNum(1);
confLen = length('conf. interval =');
tranNum = strfind(resStr,'Transfer time (ms)');

staInt = [staNum+staLen thrNum-1];
thrInt = [meanNum+meanLen confNum-1];
confInt = [confNum+confLen tranNum-1];

staStr = resStr(staInt(1):staInt(2));
thrStr = resStr(thrInt(1):thrInt(2));
confStr = resStr(confInt(1):confInt(2));

sta = textscan(staStr,'%f');
sta = sta{1};
thr = textscan(thrStr,'%f');
thr = thr{1};
conf = textscan(confStr,'%f');
conf = conf{1};

maxSta = max(sta);
sta = (1:maxSta)';

%With BA and Aggregation
thr_BA = thr(1:maxSta);
avgThr_BA = thr_BA./sta;
conf_BA = conf(1:maxSta);
%Without BA and Aggregation
thr_noBA = thr(maxSta+1:2*maxSta);
avgThr_noBA = thr_noBA./sta;
conf_noBA = conf(maxSta+1:2*maxSta);
%Without TXOP
thr_noTXOP = thr(3*maxSta+1:4*maxSta);
avgThr_noTXOP = thr_noTXOP./sta;
conf_noTXOP = conf(3*maxSta+1:4*maxSta);

%% Plot

%fitted_BA = fit(sta,1e3*avgThr_BA,'smoothingspline');
%h1 = plot(fitted_BA,'k-');
%set(h1,'LineWidth',2);
plot(sta,1e3*avgThr_BA,'k-o','LineWidth',2);
hold on;

% fitted_noBA = fit(sta,1e3*avgThr_noBA,'smoothingspline');
% h2 = plot(fitted_noBA,'b-');
% set(h2,'LineWidth',2);
plot(sta,1e3*avgThr_noBA,'b*--','LineWidth',2);
hold on;

% fitted_noTXOP = fit(sta,1e3*avgThr_noTXOP,'smoothingspline');
% h3 = plot(fitted_noTXOP,'r-');
% set(h3,'LineWidth',2);
plot(sta,1e3*avgThr_noTXOP,'r-.+','LineWidth',2);
hold on;

legend off
axis([1 20 150 550]);
xlabel('Número de estações');
ylabel('Throughput Médio [kbps]')
% legend([h1 h2 h3],'Com agregação e TXOP','Sem agregação, com TXOP','Sem agregação e sem TXOP',...
%     'Location','SouthWest');
legend('Com agregação e TXOP','Sem agregação, com TXOP','Sem agregação e sem TXOP',...
    'Location','SouthWest');
set(gca,'xtick',1:50);
grid on;
hold off;
print('-dbmp','thrVSSta');

%% Save data

save data1
