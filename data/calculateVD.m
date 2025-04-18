function calculateVD(inputFolder, outputFolder)
    % 获取文件夹中所有.mat文件
    fileList = dir(fullfile(inputFolder, '*.mat'));

    % 创建输出文件夹（如果不存在）
    if ~exist(outputFolder, 'dir')
        mkdir(outputFolder);
    end

    % 遍历每个文件
    for i = 1:length(fileList)
        originalPath = fullfile(fileList(i).folder, fileList(i).name);
        
        % 生成新文件名
        [~, name, ext] = fileparts(originalPath);
        newFileName = fullfile(outputFolder, [name ext]);
        
        try
            % 加载原始数据
            data = load(originalPath);
            
            % 验证必要字段
            if ~isfield(data, 'Datas') || ~isfield(data, 'SampleFrequency')
                warning('文件 %s 缺少必要字段，已跳过', originalPath);
                continue;
            end
            
            % 提取原始数据
            originalData = data.Datas;
            fs_str = data.SampleFrequency;
            
            % 转换采样频率
            fs = str2double(fs_str);
            if isnan(fs) || fs <= 0
                warning('文件 %s 包含无效采样频率，已跳过', originalPath);
                continue;
            end

            % 单位转换（假设原始单位是mm/s² → m/s²）
            % accData = originalData * 0.001;
            
            % 预处理：去趋势+高通滤波
            accFiltered = preprocessAcceleration(originalData, fs);
            
            % 双重积分计算（改进方法）
            [displacement, velocity] = improvedDoubleIntegration(accFiltered, fs);
                        % 准备保存数据
            processedData.Datas = displacement;
            processedData.SampleFrequency = fs_str;

            % 保存结果（使用v7.3格式保证兼容性）
            save(newFileName, '-struct', 'processedData');
            
            fprintf('成功处理: %s\n', originalPath);
            
        catch ME
            warning('处理文件 %s 失败: %s', originalPath, ME.message);
        end
    end
    
    fprintf('处理完成！结果保存在: %s\n', outputFolder);
end

%% 预处理函数（关键改进）
function accFiltered = preprocessAcceleration(accData, fs)
    % 参数设置
    cutoffFreq = 0.5; % 截止频率(Hz) - 根据实际振动特性调整
    
    % 1. 去趋势
    accDetrend = detrend(accData);
    
    % 2. 高通滤波（消除直流偏移）
    [b, a] = butter(4, cutoffFreq/(fs/2), 'high');
    accFiltered = filtfilt(b, a, accDetrend);
    
    % 3. 数据校验
    if max(abs(accFiltered)) > 10 * max(abs(accDetrend))
        warning('滤波可能引入异常，检查截止频率设置');
    end
end

%% 二次积分
function [displacement, velocity] = improvedDoubleIntegration(accData, fs)
    % 时间向量
    dt = 1/fs;
    t = (0:size(accData,1)-1)' * dt;
    
    % 第一次积分：加速度→速度
    velocity = cumtrapz(t, accData);
    
    % 速度去趋势
    velocity = detrend(velocity);
    
    % 第二次积分：速度→位移
    displacement = cumtrapz(t, velocity);
    
    % 位移去趋势
    displacement = detrend(displacement);
    
    % 泄漏积分补偿（可选）
    leakageFactor = 0.999;
    velocity = velocity * leakageFactor;
    displacement = displacement * leakageFactor;
    
    % 单位转换（m → mm）
    displacement = displacement * 1000; % 根据实际需求调整
end