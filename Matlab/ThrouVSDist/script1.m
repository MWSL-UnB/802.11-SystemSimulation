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

cellNum = strfind(resStr,'cell radius    =');
cellLen = length('cell radius    =');
thrNum = strfind(resStr,'Throughput (Mbps)');
thrLen = length('Throughput (Mbps)');
meanNum = strfind(resStr,'mean           =');
meanNum = meanNum(1);
meanLen = length('mean           =');
confNum = strfind(resStr,'conf. interval =');
confNum = confNum(1);
confLen = length('conf. interval =');
tranNum = strfind(resStr,'Transfer time (ms)');

cellInt = [cellNum+cellLen thrNum-1];
thrInt = [meanNum+meanLen confNum-1];
confInt = [confNum+confLen tranNum-1];

cellStr = resStr(cellInt(1):cellInt(2));
thrStr = resStr(thrInt(1):thrInt(2));
confStr = resStr(confInt(1):confInt(2));

cell = textscan(cellStr,'%f');
cell = cell{1};
thr = textscan(thrStr,'%f');
thr = thr{1};
conf = textscan(confStr,'%f');
conf = conf{1};

[~,maxCell] = max(cell);
cell = cell(1:maxCell);

%With BA and Aggregation
thr_BA = thr(1:maxCell);
avgThr_BA = thr_BA;
conf_BA = conf(1:maxCell);
%Without BA and Aggregation
thr_noBA = thr(maxCell+1:2*maxCell);
avgThr_noBA = thr_noBA;
conf_noBA = conf(maxCell+1:2*maxCell);
%Without TXOP
thr_noTXOP = thr(3*maxCell+1:4*maxCell);
avgThr_noTXOP = thr_noTXOP;
conf_noTXOP = conf(3*maxCell+1:4*maxCell);

%% Plot

fitted_BA = fit(cell,1e3*avgThr_BA,'smoothingspline');
h1 = plot(fitted_BA,'b');
set(h1,'LineWidth',2);
hold on;
plot(cell,1e3*avgThr_BA,'bo');

fitted_noBA = fit(cell,1e3*avgThr_noBA,'smoothingspline');
h2 = plot(fitted_noBA,'r');
set(h2,'LineWidth',2);
hold on;
plot(cell,1e3*avgThr_noBA,'ro');

fitted_noTXOP = fit(cell,1e3*avgThr_noTXOP,'smoothingspline');
h3 = plot(fitted_noTXOP,'g');
set(h3,'LineWidth',2);
hold on;
plot(cell,1e3*avgThr_noTXOP,'go');

legend off;
grid on;
legend([h1 h2 h3],'Com agregação e TXOP','Sem agregação, com TXOP','Sem agregação e sem TXOP','Location','SouthWest')
axis([5 170 0 550]);
xlabel('Distância da estação ao ponto de acesso [m]');
ylabel('Throughput médio [kbps]');
hold off;

print('-dbmp','thrVSdist');

%% Plot average

avgThr_mean = (avgThr_BA + avgThr_noBA + avgThr_noTXOP)./3;

figure();
fitted_mean = fit(cell,1e3*avgThr_mean,'smoothingspline');
hm = plot(fitted_mean,'b');
set(hm,'LineWidth',2);
hold on;
plot(cell,1e3*avgThr_mean,'bo');

legend off;
grid on;
axis([5 170 0 550]);
xlabel('Distância da estação ao ponto de acesso [m]');
ylabel('Throughput médio [kbps]');
hold off;

print('-dbmp','thrVSdist_mean');

%% Save data

save data1
