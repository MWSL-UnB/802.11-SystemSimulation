clc
clear all
close all
%% General

offeData = 0.5;
simTime = 1.1;
numSta = 1;

%% Get Results

resFile = fileread('results.txt');
resNum = strfind(resFile,'%%%% Final results %%%%');
resStr = resFile(resNum:end);
clear resFile;

offNum = strfind(resStr,'data rate      =');
offLen = length('data rate      =');
thrNum = strfind(resStr,'Throughput (Mbps)');
thrLen = length('Throughput (Mbps)');
meanNum = strfind(resStr,'mean           =');
meanNum = meanNum(1);
meanLen = length('mean           =');
confNum = strfind(resStr,'conf. interval =');
confNum = confNum(1);
confLen = length('conf. interval =');
tranNum = strfind(resStr,'Transfer time (ms)');

offInt = [offNum+offLen thrNum-1];
thrInt = [meanNum+meanLen confNum-1];
confInt = [confNum+confLen tranNum-1];

offStr = resStr(offInt(1):offInt(2));
thrStr = resStr(thrInt(1):thrInt(2));
confStr = resStr(confInt(1):confInt(2));

off = textscan(offStr,'%f');
off = off{1};
thr = textscan(thrStr,'%f');
thr = thr{1};
conf = textscan(confStr,'%f');
conf = conf{1};

[~,maxOff] = max(off);
off = off(1:maxOff);

%With BA and Aggregation
thr_BA = thr(1:maxOff);
avgThr_BA = thr_BA/numSta;
conf_BA = conf(1:maxOff);
%Without BA and Aggregation
thr_noBA = thr(maxOff+1:2*maxOff);
avgThr_noBA = thr_noBA/numSta;
conf_noBA = conf(maxOff+1:2*maxOff);
%Without TXOP
thr_noTXOP = thr(3*maxOff+1:4*maxOff);
avgThr_noTXOP = thr_noTXOP/numSta;
conf_noTXOP = conf(3*maxOff+1:4*maxOff);

%% Plot

dtPt = 2;

fitted_BA = fit(off,avgThr_BA,'smoothingspline');
% h1 = plot(fitted_BA,'k-');
% set(h1,'LineWidth',2);
hold on;
% plot(off(1:dtPt:end),avgThr_BA(1:dtPt:end),'*k','LineWidth',2);
h1_i = plot(off,avgThr_BA,'*k-','LineWidth',2);
% set(h1_i,'Visible','off');

fitted_noBA = fit(off,avgThr_noBA,'smoothingspline');
% h2 = plot(fitted_noBA,'b--');
% set(h2,'LineWidth',2);
hold on;
% plot(off(1:dtPt:end),avgThr_noBA(1:dtPt:end),'ob','LineWidth',2);
h2_i = plot(off,avgThr_noBA,'ob--','LineWidth',2);
% set(h2_i,'Visible','off');

fitted_noTXOP = fit(off,avgThr_noTXOP,'smoothingspline');
% h3 = plot(fitted_noTXOP,'r-.');
% set(h3,'LineWidth',2);
hold on;
% plot(off(1:dtPt:end),avgThr_noTXOP(1:dtPt:end),'+r','LineWidth',2);
h3_i = plot(off,avgThr_noTXOP,'+r-.','LineWidth',2);
% set(h3_i,'Visible','off');

legend off;
xlabel('Dados oferecidos [Mbps]');
ylabel('Throughput [Mbps]');
grid on;
axis tight;
hold off;
legend([h1_i h2_i h3_i],{'AC_VI com agregação','AC_VI sem agregação','AC_BE'},...
    'Location','NorthWest','Interpreter','none')
axis([0 65 0 65]);
print('-dpng','thrVSOffer');

%% Save data

save data1

