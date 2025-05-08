function addPropertyToMatFiles(folderPath,key,value)
% 给指定文件夹下所有MAT文件添加SampleFrequency字段
% 输入参数：
%   folderPath - 包含MAT文件的文件夹路径（字符串）

% 检查文件夹是否存在
if ~isfolder(folderPath)
    error('指定的文件夹不存在: %s', folderPath);
end

% 获取所有.mat文件
matFiles = dir(fullfile(folderPath, '*.mat'));

% 检查是否找到MAT文件
if isempty(matFiles)
    fprintf('文件夹中没有找到.mat文件: %s\n', folderPath);
    return;
end

fprintf('正在处理文件夹: %s\n', folderPath);
fprintf('找到 %d 个.mat文件\n', length(matFiles));

% 遍历所有MAT文件
for i = 1:length(matFiles)
    currentFile = matFiles(i).name;
    fullPath = fullfile(folderPath, currentFile);
    
    try
        % 加载MAT文件数据
        matData = load(fullPath);
        
        % 检查是否已存在SampleFrequency字段
        %if isfield(matData, key)
            %fprintf('文件 %s 已包含SampleFrequency字段，跳过...\n', currentFile);
            %continue;
       % end
        
        % 添加SampleFrequency字段
        matData.SampleFrequency = value;
        
        % 保存修改后的文件（使用'-v7.3'格式以支持大文件）
        save(fullPath, '-struct', 'matData');
        
        fprintf('成功处理文件: %s\n', currentFile);
        
    catch ME
        fprintf('处理文件 %s 时出错: %s\n', currentFile, ME.message);
    end
end

fprintf('处理完成！\n');
end