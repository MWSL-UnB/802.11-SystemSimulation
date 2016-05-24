clc
clear all
close all

%%

kmax = 1000;
kmin = 10;

fid = fopen('test.txt','w');
for k = kmin:kmin:kmax
    fprintf(fid,'%d',k);
    if k <kmax
       fprintf(fid,',');
    end
end
fclose(fid);