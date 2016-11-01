clc
clear all
close all

%%

kmax = 2000;
kmin = 10;

fid = fopen('nums.txt','w');
for k = kmin:kmin:kmax
    fprintf(fid,'%d',k);
    if k <kmax
       fprintf(fid,',');
    end
end
fclose(fid);