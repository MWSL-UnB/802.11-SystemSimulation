clc
clear all
close all

%%

kmax = 50;
kmin = 1;

fid = fopen('nums.txt','w');
for k = kmin:kmin:kmax
    fprintf(fid,'%d',k);
    if k <kmax
       fprintf(fid,',');
    end
end
fclose(fid);