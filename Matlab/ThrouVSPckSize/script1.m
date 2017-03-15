clc
clear all
close all

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

plot(offeDataBA,m_perStaBA,'k-o','LineWidth',2);
hold on;
plot(offeData,m_perSta,'b--*','LineWidth',2);
hold on;
plot(offeDataNT,m_perStaNT,'r-.+','LineWidth',2);
hold off;
grid on;
xlabel('Taxa de dados oferecida a MAC [Mbps]');
ylabel('Throughput [Mbps]');
legend('Com agregação e TXOP','Sem agregação, com TXOP','Sem agregação e sem TXOP','Location','SouthEast');
print('-dbmp','thrVSOffer');

%% Save data

save data1