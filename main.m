% DIP Final Project Team 3
% Sam Lally, Rachael Lee, Vincent Mai
clear; clc;
close all;

% Read in color image
originalImage = imread('colorImage.jpg');
[m, n, k] = size(originalImage);

% Read in depth image
filename = 'depthImage.txt';
f = fopen(filename);
data = textscan(f,'%s');
fclose(f);
variable = str2double(data{1});
depthImage = reshape(variable, [n m]);
depthImage = depthImage./max(max(depthImage));
depthImage = imrotate(depthImage, -90);
depthImage = flip(depthImage, 2);

% Create hsv and blur mask images
hsvImage = ones(m, n, 3);
blurMask = zeros(m, n);
for i=1:m
    for j=1:n
        if depthImage(i, j) == 0
            hsvImage(i,j,1) = 10;
        else
            hsvImage(i,j,1) = depthImage(i,j);
        end
        
        if depthImage(i, j) > 0.2 || depthImage(i, j) == 0
            blurMask(i,j) = 1;
        end
    end
end

% Blur the mask to soften harsh edges
blurMask = imgaussfilt(blurMask, 5);

% Create blurred image to combine with original
blurredImage = imgaussfilt(originalImage, 15);

% Combine the original and blurred image using the blur mask
finalImage = originalImage;
for i=1:m
    for j=1:n
        finalImage(i,j,:) = (1-blurMask(i,j))*finalImage(i,j,:) + blurMask(i,j)*blurredImage(i,j,:);
    end
end

% Display results
figure,imshow(originalImage),title('Color Image'); 
figure, imshow(hsv2rgb(hsvImage)), title('Depth Map');
figure,imshow(blurMask), title('Blur Mask');
figure,imshow(blurredImage), title('Blurred Image');
figure, imshow(finalImage), title('Final Image');