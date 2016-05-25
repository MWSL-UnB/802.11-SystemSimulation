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

pckNum = strfind(resStr,'packet length  =');
pckLen = length('packet length  =');
thrNum = strfind(resStr,'Throughput (Mbps)');
thrLen = length('Throughput (Mbps)');
meanNum = strfind(resStr,'mean           =');
meanNum = meanNum(1);
meanLen = length('mean           =');
confNum = strfind(resStr,'conf. interval =');
confNum = confNum(1);
confLen = length('conf. interval =');
tranNum = strfind(resStr,'Transfer time (ms)');

pckInt = [pckNum+pckLen thrNum-1];
thrInt = [meanNum+meanLen confNum-1];
confInt = [confNum+confLen tranNum-1];

pckStr = resStr(pckInt(1):pckInt(2));
thrStr = resStr(thrInt(1):thrInt(2));
confStr = resStr(confInt(1):confInt(2));

pck = (10:10:1000)';
thr = textscan(thrStr,'%f');
thr = thr{1};
conf = textscan(confStr,'%f');
conf = conf{1};

[~,maxPck] = max(pck);
pck = pck(1:maxPck);

%With BA and Aggregation
thr_BA = thr(1:maxPck);
avgThr_BA = thr_BA;
conf_BA = conf(1:maxPck);
%Without BA and Aggregation
thr_noBA = thr(maxPck+1:2*maxPck);
avgThr_noBA = thr_noBA;
conf_noBA = conf(maxPck+1:2*maxPck);
%Without TXOP
thr_noTXOP = thr(3*maxPck+1:4*maxPck);
avgThr_noTXOP = thr_noTXOP;
conf_noTXOP = conf(3*maxPck+1:4*maxPck);

%% Plot

fitted_BA = fit(pck,1e3*avgThr_BA,'smoothingspline');
h1 = plot(fitted_BA,'b');
set(h1,'LineWidth',2);
hold on;
plot(pck,1e3*avgThr_BA,'bo');

fitted_noBA = fit(pck,1e3*avgThr_noBA,'smoothingspline');
h2 = plot(fitted_noBA,'r');
set(h2,'LineWidth',2);
hold on;
plot(pck,1e3*avgThr_noBA,'ro');

fitted_noTXOP = fit(pck,1e3*avgThr_noTXOP,'smoothingspline');
h3 = plot(fitted_noTXOP,'g');
set(h3,'LineWidth',2);
hold on;
plot(pck,1e3*avgThr_noTXOP,'go');

legend off;
xlabel('Dados por pacote [Bytes]');
ylabel('Throughput Médio [kbps]');
grid on;
axis tight;
hold off;
legend([h1 h2 h3],'Com agregação e TXOP','Sem agregação, com TXOP','Sem agregação e sem TXOP',...
    'Location','SouthEast')

print('-dbmp','thrVSPckSz');

%% Save data

save data1
