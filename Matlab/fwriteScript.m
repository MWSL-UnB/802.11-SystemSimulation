clc
clear all
close all

%%

kmax = 50;
kmin = 2.5;

fid = fopen('nums.txt','w');
for k = kmin:kmin:kmax
    fprintf(fid,'%.1f',k);
    if k <kmax
       fprintf(fid,',');
    end
end
fclose(fid);