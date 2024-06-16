clc; clear; close all; clear sound;

sr = 8000;
x = cos(2 * pi * 440 * (0:1/sr:1));

EPS = 1e-4;
frameSize = 2048;
nOverlap = 4;
hopSize = frameSize / nOverlap;
window = hann(frameSize);
window = window / sqrt(2);
blockSize = 256;

nBlocks = floor(length(x) / blockSize);
nSamples = nBlocks * blockSize;

x = x(1:nSamples);
y = zeros(size(x));

xNow = zeros(frameSize, 1);
xPast = zeros(frameSize, 1);
X = ones(frameSize, 1);
lastX = zeros(frameSize, 1);
Y = zeros(frameSize, 1) + 1e-10;
yBlock = zeros(blockSize, 1);

readBuffer = zeros(frameSize, 1);
writeBuffer = zeros(frameSize, 1);
pBuffer = 1;

pIn = 1;
pOut = blockSize;

for b = 1:nBlocks

    xBlock = x(pIn:pOut);

    for n = 1:blockSize

        readBuffer(pBuffer) = xBlock(n);
        yBlock(n) = writeBuffer(pBuffer);

        pBuffer = mod(pBuffer, frameSize) + 1;

        if mod(pBuffer, hopSize) == 1

            % Read from circular buffer.
            for n = 1:frameSize
                
                xNow(n) = readBuffer(pBuffer);
                pBuffer = mod(pBuffer, frameSize) + 1;

            end

            % Do stuff.
            X = fft(window .* xNow);
            lastX = fft(window .* xPast);

            % As per Puckette phase-locked vocoder paper.
            %
            % https://msp.ucsd.edu/Publications/mohonk95.pdf
            
            Y = X .* Y ./ (lastX + EPS) ./ (abs(Y ./ X) + EPS);
            yNow = window .* ifft(Y);

            for j = 1:frameSize

                % Don't overlap add for the last `hopSize` samples.
                overlapMask = (j <= frameSize - hopSize);
                writeBuffer(pBuffer) = writeBuffer(pBuffer) * overlapMask + yNow(j);
                pBuffer = mod(pBuffer, frameSize) + 1;

            end
            
            xPast = xNow;

        end

    end

    y(pIn:pOut) = yBlock;

    pIn = pIn + blockSize;
    pOut = pOut + blockSize;

end

timeplot(x, sr); hold on;
timeplot(y, sr); hold on;
xlabel("Time (s)");