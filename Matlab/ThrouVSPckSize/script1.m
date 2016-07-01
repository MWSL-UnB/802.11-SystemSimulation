clc
clear all
close all
%% General

offeData = 0.5;
simTime = 1.1;
numSta = 15;

%% Get Results

resFile = fileread('results.txt');
resNum = strfind(resFile,'%%%% Final results %%%%');
resStr = resFile(resNum:end);
clear resFile;

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

pck = (10:10:2050)';
thr = textscan(thrStr,'%f');
thr = thr{1};
conf = textscan(confStr,'%f');
conf = conf{1};

switch numSta
    case 1
        thr = thr(1:end/2);
        conf = conf(1:end/2);
    case 15
        thr = thr(end/2+1:end);
        conf = conf(end/2+1:end);
end

[~,maxPck] = max(pck);
pck = pck(1:maxPck);

%With BA and Aggregation
thr_BA = thr(1:maxPck);
avgThr_BA = thr_BA/numSta;
conf_BA = conf(1:maxPck);
%Without BA and Aggregation
thr_noBA = thr(maxPck+1:2*maxPck);
avgThr_noBA = thr_noBA/numSta;
conf_noBA = conf(maxPck+1:2*maxPck);
%Without TXOP
thr_noTXOP = thr(3*maxPck+1:4*maxPck);
avgThr_noTXOP = thr_noTXOP/numSta;
conf_noTXOP = conf(3*maxPck+1:4*maxPck);

%% Plot

dtPt = 3;

fitted_BA = fit(pck,1e3*avgThr_BA,'smoothingspline');
h1 = plot(fitted_BA,'b');
set(h1,'LineWidth',2);
hold on;
plot(pck(1:dtPt:end),1e3*avgThr_BA(1:dtPt:end),'bo');

fitted_noBA = fit(pck,1e3*avgThr_noBA,'smoothingspline');
h2 = plot(fitted_noBA,'r');
set(h2,'LineWidth',2);
hold on;
plot(pck(1:dtPt:end),1e3*avgThr_noBA(1:dtPt:end),'ro');

fitted_noTXOP = fit(pck,1e3*avgThr_noTXOP,'smoothingspline');
h3 = plot(fitted_noTXOP,'g');
set(h3,'LineWidth',2);
hold on;
plot(pck(1:dtPt:end),1e3*avgThr_noTXOP(1:dtPt:end),'go');

legend off;
xlabel('Dados por pacote [Bytes]');
ylabel('Throughput Médio [kbps]');
grid on;
axis tight;
hold off;
legend([h1 h2 h3],'Com agregação e TXOP','Sem agregação, com TXOP','Sem agregação e sem TXOP',...
    'Location','SouthEast')
axis([0 2000 0 350]);
print('-dbmp','thrVSPckSz');

%% Save data

save data1
