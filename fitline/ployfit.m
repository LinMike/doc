close all
clear
%global logfile
global pha
fileFolder = fullfile('/home/robot/桌面/log_track/'); %/home/robot/桌面/rotation/log_track/
dir_file = dir(fullfile(fileFolder, '*.txt'));

global sum_ph;
sum_ph = 0;
for i=1:size(dir_file, 1) % cols
    %logfile = '/home/robot/桌面/rotation/log_track/20180725_131629_811000.txt';
    %logfile = '/home/robot/桌面/rotation/log_track/20180725_095909_373000.txt';
    fileName = dir_file(i).name;
    logfile = strcat(fileFolder, fileName);
    data = load(logfile);
    data = data(2:end, :);
    data(:, 1) = ((data(:, 1) - data(1, 1))/1000); 
    data(:, 2:4) = data(:, 2:4)/100;

    cruve_height = -1;
    lastHeight = data(1, 4);
    for j = 1:size(data, 1)
        height = data(j, 4);
        if height <= 0.8 && lastHeight < height && cruve_height - height > 0.5
            data = data(1:j-1, :);
            break;
        end
        lastHeight = height;
        if cruve_height < height 
            cruve_height = height;
        end
    end

    t = data(:, 1);
    x = data(:, 2);
    y = data(:, 3);
    z = data(:, 4);

    px = polyfit(t, x, 2);
    py = polyfit(t, y, 2);
    pz = polyfit(t, z, 3);

    syms tt
    fz = pz(1,1)*tt*tt*tt + pz(1,2)*tt*tt + pz(1,3)*tt + pz(1,4) - 0.2;
    p = sym2poly(fz);
    tt = roots(p);
    
    ts = 0;
    if tt(1,1) < 0 && tt(1,1) > -1
        ts = tt(1,1);
    elseif tt(2,1) < 0 && tt(2,1) > -1
        ts = tt(2,1);
    else ts = tt(3,1);
    end

    x0 = px(1,1)*ts*ts + px(1,2)*ts + px(1,3);
    y0 = py(1,1)*ts*ts + py(1,2)*ts + py(1,3);
    z0 = pz(1,1)*ts*ts*ts + pz(1,2)*ts*ts + pz(1,3)*ts + pz(1,4);

    %fall_point_x = px(1,1)*ts*ts + px(1,2)*ts + px(1,3);
    fall_point_x = data(end, 2);

    vx = 2*px(1,1)*ts + px(1,2);
    vy = 2*py(1,1)*ts + py(1,2);
    vz = 3*pz(1,1)*ts*ts + 2*pz(1,2)*ts + pz(1,3);
    v0 = sqrt(vx*vx + vy*vy + vz*vz);
    theta = atan2(abs(vz), abs(vx));
    s_real = x0 - fall_point_x;

    syms ph
    s = 1.0/((1+ph*sin(theta))*(1+ph*sin(theta))) * v0*v0*sin(2*theta)/9.8 - s_real;
    ph = solve(s == 0, ph);
    ph = vpa(ph, 5);
    
    if ph(2,1) > 0
        sum_ph = ph(2,1) + sum_ph;
    else
        sum_ph = ph(1,1) + sum_ph;
    end

    i
    fig1 = figure(1);
    aa = ph(2,1);
    bb = v0*v0*sin(2*theta)/9.8 - s_real;
    plot(aa, bb, '.');
    hold on

    m_ph = 0.275;
    m_sx = 1.0/((1+m_ph*sin(theta))*(1+m_ph*sin(theta))) * v0*v0*sin(2*theta)/9.8 - s_real;
    fig2 = figure(2);
    plot(i, m_sx, '.');
    hold on

    fig3 = figure(3);
    plot(aa - m_ph, m_sx, '.');
    hold on
    
%     pha(i,1) = ph(2,1);
%     pha(i,2) = vpa(sx_no_f, 5);
%     pha(i,3) = vpa(s_real, 5);

end

avg_pha = sum_ph / i

% 
% x = data(:, 1);
% y = data(:, 2);
% z = data(:, 3);
% 
% ps = [-24.81, 3.2, 12.89, -0.09692, -0.7182, -1.849];
% funz = ps(1, 1)+ps(1,2)*x1+ps(1,3)*y1+ps(1,4)*x1*x1+ps(1,5)*x1*y1+ps(1,6)*y1*y1;
% 
% pp = lsqcruvefit(funz, ps, data(:, 1), data(:, 2));