% Functions from ECE4580 HW2 Appendix

function g = intScaling4e(f, mode, type)
    if nargin == 1
        type = 'floating';
        if isa(f, 'uint8')
            mode = 'default';
        elseif min(f(:)) < 0 || max(f(:)) > 1
            mode = 'full';
            type = 'floating';
        else
            mode = 'default';
        end
    elseif nargin == 2
        type = 'floating';
    end
    
    NC = size(f, 3);
    maxall = max(f(:));
    
    for I = 1:NC
        g(:,:,I) = intScaleGrayImage(f(:,:,I), maxall, mode, type);
    end
end

function g = intScaleGrayImage(f, maxall, mode, type)
    g = double(f);
    
    switch mode
        case 'default'
            if maxall > 255
                g = g/maxall;
            elseif isa(f, 'uint8')
                g = double(g)/255;
            else
                g = f;
            end
        case 'full'
            g = g - min(g(:));
            g = g/max(g(:));
    end
    
    if isequal(type, 'integer')
        if isToolboxAvailable('Image Processing Toolbox')
            g = imquint8(g);
        else
            g = uint8(floor(g*255));
        end
    end
end