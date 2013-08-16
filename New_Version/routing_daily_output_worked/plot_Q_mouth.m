clc; clear all; close all;

% Load in observed data
obs = load('/Users/tjtroy/Research/data/USGS/US_streamflow_updated/formatted/03234500.dat');
to = datenum(obs(:,2:4));

% Loop through years
for year=1997:2004
    filename = sprintf('output_%d.txt',year);
    
    data = load(filename);
    
    % Convert Q_ts into Y M D H MIN SEC
    ts = zeros(size(data,1),6);
    ts(:,1:3) = data(:,1:3);
    
    for hour=1:24
        % Find hour
        start = (hour-1)*3600+1;
        finish = start+3600-1;
        r = find(data(:,4)>= start & data(:,4)<=finish);
        ts(r,4) = hour;
        % Find minute
        temp1 = data(r,4) - start;
        temp2 = floor(temp1/60);
        ts(r,5) = temp2+1;
        ts(r,6) = temp1-temp2*60+1;
        clear r;
    end
    
    
    figure;
    t = datenum(ts);
    plot(t,data(:,30)/(0.3048^3));
        
    r = find(to>=t(1) & to<=t(length(t)));
    hold on;
    plot(to(r),obs(r,5),'k');
    clear r;
    set(gca,'FontSize',14);
    title(num2str(year));
    legend('VIC','Obs');
    
    datetick('x',6);
end