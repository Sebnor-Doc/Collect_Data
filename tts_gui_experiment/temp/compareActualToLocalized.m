function rms = compareActualToLocalized(actual, localized, maxRMS)
    % 
    correct = dlmread(actual);
    
    incorrect = dlmread(localized);
    correct = correct(:,1:3);
    incorrect = incorrect(:,1:3);
    
    % Delete first and last 1000     
    correct = correct(1000:end-1000,:);
    incorrect = incorrect(1000:end-1000,:);
    
    correct = correct(1000:3000,:);
    incorrect = incorrect(1000:3000,:);
    
    % Plot  
    openfig('setup_post.fig');
    hold on;
    
    plot3(correct(:,1),correct(:,2),correct(:,3));
    hold on;
    plot3(incorrect(:,1),incorrect(:,2),incorrect(:,3),'r');
    
    
    openfig('setup_post.fig');
    hold on;
    % Generate color matrix
    color_matrix = (sqrt(sum((correct - incorrect).^2,2)))/(maxRMS/1000);
    r = color_matrix*1;
    g = (1-color_matrix)*1;
    b = color_matrix*0;
    color_matrix = squeeze(cat(3,r,g,b));   
    
    showPointCloud(correct, color_matrix);
%     hold on;
%     showPointCloud(incorrect);
    
    % RMS
    rms = (sqrt(sum((correct - incorrect).^2,2)));
    
    figure
    plot(rms)
end