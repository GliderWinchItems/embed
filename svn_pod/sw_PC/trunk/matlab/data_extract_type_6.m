clear all
close all
clc
format
fclose('all');
%file_spec = '120915.164010XRM'
file_spec = '120915.165407XRM'

%file_spec = '121117.launch1RMX'
%file_spec = '121117.launch2RMX'
%file_spec = '121117.launch3RMX'
%file_spec = '121117.launch4RMX'
%  These don't work, wrong file format
%file_spec = '120915.175936XFT'
%file_spec = '120915.181354XFT'

format compact

wnch_lat = 34.920623077    %  winch latitude
wnch_long = 81.950259487   %  winch longitude

format short g



rds =  6371e3;             %  radius of earth
g = 9.8066;                %  acceleration due to gravity

wnch_h = 233;              %  winch height
M = 550                    %  Glider mass
%
f_zero = -2.1					% Tension offset



%  Extract Data
fid = fopen(file_spec);

tmp = fscanf(fid,...
   '%*f %*d %c%c%c %c%c%c %*d %*d %*d %*d %*d %*d %*d %*f %*f %*f %*f %*f %*f %*d %*s',...
   [3, 2]);

week_day = tmp(:, 1)';
mon = tmp(:, 2)';

frewind(fid)
tmp = fscanf(fid, '%*f %*d %*s %*s %d %d %d %d %d %d %*d %*f %*f %*f %*f %*f %*f %*d %*s',...
   [1, 6])

year = num2str(tmp(5), 4);
day = num2str(tmp(1), 2);
hour = num2str(tmp(2), 2);
minite = num2str(tmp(3), 2);
sec = num2str(tmp(4) + tmp(6)/64, 4); 

banner =['Start Time: ', week_day, ' ', mon, ' ', day, ', ', year, ', ', hour, ':', minite, ':', sec]

frewind(fid)

data = fscanf(fid, '%f %d %*s %*s %*d %*d %*d %*d %*d %*d %*d %f %f %f %f %f %f %*d %*s',...
   [8, inf])';

t = data(:, 1)/64 - data(1,1)/64;                  %  time
f = data(:, 2)/1000;             %  tension (kg)
lat = data(:, 3) - wnch_lat;     %  latitude
long = data(:, 4) - wnch_long;   %  longitude
h = data(:, 5);                  %  height
est_v = data(:, 6);              %  east velocity
nrt_v = data(:, 7);              %  north velocity
vrt_v = data(:, 8);              %  vertical velocity

del_t = t(2) - t(1);             %  delta time increment

%  Determine initial zero values, height and tension
T_avg = 2;                       %  time to average initial values
n = find(t > T_avg);
n = n(1) ;
init_h = sum(h(1:n))/n
t_min = t(1)
t_max = t(end)
% t_min = 5
% t_max = 15

%  Shift velocity and positions time offsets
adv_vel = 1.0     %  time to advance velocity data (affects vector velocity)
n_dly = round(adv_vel/del_t) + 1;
est_v = [est_v(n_dly:end) ; est_v(end) * ones(n_dly - 1, 1)];
nrt_v = [nrt_v(n_dly:end) ; nrt_v(end) * ones(n_dly - 1, 1)];
vrt_v = [vrt_v(n_dly:end) ; vrt_v(end) * ones(n_dly - 1, 1)];

rtd_pos = 0.3    %  time to retard postion data (affects radial velocity)
n_dly = round(rtd_pos/del_t);
%lat = [lat(1) * ones(n_dly, 1) ; lat(1:end - n_dly)];
%long = [long(1) * ones(n_dly, 1) ; long(1:end - n_dly)];
%h = [h(1) * ones(n_dly, 1) ; h(1:end - n_dly)];
lat  = [lat(n_dly:end)   ; lat(end)  * ones(n_dly - 1, 1)];
long = [long(n_dly:end)  ; long(end) * ones(n_dly - 1, 1)];
h    = [h(n_dly:end)     ; h(end)    * ones(n_dly - 1, 1)];

%  Process Data

h2 = est_v.^2 + nrt_v.^2;     %  horizontal velocity squared
vec_v = sqrt(h2 + vrt_v.^2);  %  vector velocity (TAS if no wind)

vec_acc = [0; diff(vec_v)] * (1/del_t);


%  glider positions relative to the winch
pos_wst = pi/180 * cos(wnch_lat * pi/180) * rds * long ;
pos_nrt = pi/180 * rds * lat;
pos_int = sqrt(pos_wst(1)^2 + pos_nrt(1)^2);

r = sqrt(pos_wst.^2 + pos_nrt.^2 + (h - wnch_h).^2); %  radial distance
r_dot = -[0; diff(r)] * (1/del_t);



%  apply data smoothing
B = 16;                              %  boxcar length; 1 is no filtering
smth = conv(ones(1, B), ones(1,B));  %  CIC2 impulse response
smth = smth / sum(smth);            %  normailize to unity gain

f = cnlv_med(f, smth);
vec_acc = cnlv_med(vec_acc, smth);
r_dot = cnlv_med(r_dot, smth);
vec_v = cnlv_med(vec_v, smth);


%  Energy Analyses

ke = M * 0.5 * vec_v.^2;         %  glider kinetic energy
pe = M * g * (h - init_h);       %  glider potential energy relative to init_h
te = ke + pe;                    %  glider total energy

ke_dot = [0; diff(ke)] * (1/del_t); % Power going into kinetic energy

pow = r_dot .* (f - f_zero) * g; %  power being delivered 
wnch_engr = cumsum(pow) * del_t; %  total energy delivered

idl_h = (wnch_engr - ke) / (M * g);
idl_vrt_v = [0; diff(idl_h)] * (1/del_t);


figure(1)
clf

%subplot(2,1,1)
plot(t, f - f_zero, 'g', 'linewidth', 2)
hold on
title(banner)

xlabel('Time (sec)') 
ylabel('kgf - height (m)')
xlim([t_min t_max])
ylim([-1 800])
%ylim([-1 705])
grid on

plot(t, h - init_h, 'r', 'linewidth', 2)
plot(t, idl_h, 'r--', 'linewidth', 2)
legend('Tension', 'Height', 'Lossless Ht', 'GPS Ht', 'Ideal Ht')
zoom on

figure(5)
clf
%subplot(2,1,2)

plot(t, vec_v, 'b', 'linewidth', 2)
hold on
title(banner)
plot(t, vrt_v, 'r', 'linewidth', 2)
plot(t, r_dot, 'g', 'linewidth', 2)
plot(t, vec_acc, 'b--', 'linewidth', 2)
plot(t, idl_vrt_v, 'r--', 'linewidth', 2)
xlabel('Time (sec)')
ylim([-5 41])
%ylim([-1 10])
xlim([t_min t_max])
ylabel('Velocity (m/s), Acceleration (m/s^2)')
legend('Vector', 'Vertical', 'Radial', 'Vector Accel', 'Ideal vert v')
grid on
zoom on

figure(2)
clf
plot(-pos_wst, pos_nrt, 'linewidth', 2)
hold on

plot([-pos_wst(1), 0], [pos_nrt(1) 0], 'r--', 'linewidth', 2)
plot(0,0, 'rx')
grid on
axis square
title(banner)
xlabel('west-east position (m)')
ylabel('north-south position (m)')
legend('Ground Track', 'Centerline', 'location', 'northwest')
text(-pos_wst(1), pos_nrt(1), ['  --Run Length: ' num2str(pos_int, 4)])
zoom on



figure(3)
clf

plot(t, r, 'linewidth', 2)
grid on
xlim([t_min t_max])
xlabel('time (s)')
ylabel('radial distance (m)')
text(-pos_wst(1), pos_nrt(1), ['  --Run Length: ' num2str(pos_int, 4)])
title(banner)
zoom on

figure(4)
clf

plot(t, pow/1e3, 'r', 'linewidth', 2)
hold on
title(banner)
plot(t, ke_dot/1e3, 'b', 'linewidth', 2)
ylim([-50 200])
plot(t, (f - f_zero)/4, 'g', 'linewidth', 2)
grid on
xlabel('time (s)')
ylabel('power (kW) tension(kgf/4) kinetic(kW)')
legend ('Power', 'Kinetic power', 'Tension' )
zoom on
title(banner)


figure(1)


