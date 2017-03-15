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

%With short GI
thr_sh = thr(1:maxOff);
avgThr_sh = thr_sh/numSta;
conf_sh = conf(1:maxOff);
%With long GI
thr_lo = thr(maxOff+1:end);
avgThr_lo = thr_lo/numSta;
conf_lo = conf(maxOff+1:end);

%% Plot

dtPt = 2;

fitted_sh = fit(off,avgThr_sh,'smoothingspline');
% h1 = plot(fitted_sh,'k-');
% set(h1,'LineWidth',2);
hold on;
plot(off(1:dtPt:end),avgThr_sh(1:dtPt:end),'*k','LineWidth',2);
plot(off,avgThr_sh,'-k','LineWidth',2);
h1_i = plot(off,avgThr_sh,'*k-','LineWidth',2);
set(h1_i,'Visible','off');

fitted_lo = fit(off,avgThr_lo,'smoothingspline');
% h2 = plot(fitted_lo,'b--');
% set(h2,'LineWidth',2);
hold on;
plot(off(1:dtPt:end),avgThr_lo(1:dtPt:end),'ob','LineWidth',2);
plot(off,avgThr_lo,'--b','LineWidth',2);
h2_i = plot(off,avgThr_lo,'ob--','LineWidth',2);
set(h2_i,'Visible','off');

legend off;
xlabel('Dados oferecidos [Mbps]');
ylabel('Throughput [Mbps]');
grid on;
axis tight;
hold off;
legend([h1_i h2_i],{'GI Curto','GI Longo'},...
    'Location','NorthWest','Interpreter','none')
axis([0 110 0 80]);
print('-dpng','thrVSOffer_GI.png');

%% Save data

save data1

