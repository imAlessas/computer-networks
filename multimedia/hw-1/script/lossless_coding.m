clc, clearvars, clear, close, format compact


% Task 1 - Load the image and estime its entropy

img_file_name = 'rocky-beach-greys.jpg';
img_folder = '../imgs/';

img = imread("" + img_folder + img_file_name);

% creates figure and settings
f = figure(1);
f.Name = 'Image';
f.NumberTitle = 'off';
f.Position = [450, 100, 700, 600];

% displays figure
imagesc(img), colormap(gray), axis image, axis off
