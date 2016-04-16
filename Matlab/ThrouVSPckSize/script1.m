clc
clear all
close all

%% General

MACoh = 30; % MAC overhead in bytes
PHYoh = 16; % PHY overheaad in bytes for 6Mbps
Total = MACoh + PHYoh;

dataSiz = 50:2304;
pckSiz = dataSiz + Total;
prop = 100*Total./pckSiz;

%% Plot

plot(dataSiz,prop,'LineWidth',2);
hold on
plot(100,prop(dataSiz == 100),'ks','markerfacecolor',[0 0 0]);
text(120,prop(dataSiz == 100) + 1,'100 bytes, 31,5% de overhead');
axis([min(dataSiz) max(dataSiz) min(prop) max(prop)]);
xlabel('Bytes de dados');
ylabel('Overhead total [%]');
grid on
hold off
print('-dbmp','overhVSpckSiz');